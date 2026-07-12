#include "aarch64_regalloc.hh"

#include <algorithm>
#include <array>
#include <limits>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace backend {
namespace {

using TempSet = std::unordered_set<int>;

struct BlockInfo {
    quad::QuadBlock *block = nullptr;
    int label = -1;
    int start = 0;
    int end = 0;
    int loopDepth = 0;
    std::size_t phiStatementCount = 0;
    std::vector<std::size_t> successors;
    TempSet defs;
    TempSet uses;
    TempSet phiDefs;
    std::unordered_map<int, TempSet> phiUsesByPredecessor;
    TempSet liveIn;
    TempSet liveOut;
};

struct Interval {
    int temp = -1;
    int start = std::numeric_limits<int>::max();
    int end = std::numeric_limits<int>::min();
    long long weightedAccesses = 0;
    int accesses = 0;
    int uses = 0;
    bool parameter = false;
    bool liveAcrossCall = false;
    std::unordered_set<std::size_t> blocks;

    bool valid() const {
        return temp >= 0 && start <= end && uses != 0;
    }

    long long priority() const {
        // Loop weighting carries most of the dynamic signal.  A modest bonus
        // prevents useful values spanning blocks (especially PHIs and
        // parameters) from being displaced by one-use straight-line values.
        return weightedAccesses * 8LL + static_cast<long long>(accesses) * 2LL +
               static_cast<long long>(blocks.size()) * 16LL +
               (parameter ? 8LL : 0LL);
    }
};

bool insertAll(TempSet &destination, const TempSet &source) {
    bool changed = false;
    for (int temp : source) {
        changed = destination.insert(temp).second || changed;
    }
    return changed;
}

void addTermUse(quad::QuadTerm *term, TempSet &result) {
    if (term == nullptr || term->kind != quad::QuadTermKind::TEMP ||
        term->get_temp() == nullptr || term->get_temp()->temp == nullptr) {
        return;
    }
    result.insert(term->get_temp()->temp->num);
}

void addCallUses(quad::QuadCall *call, TempSet &result) {
    if (call == nullptr) return;
    addTermUse(call->obj_term, result);
    if (call->args != nullptr) {
        for (auto *argument : *call->args) addTermUse(argument, result);
    }
}

void addExtCallUses(quad::QuadExtCall *call, TempSet &result) {
    if (call == nullptr || call->args == nullptr) return;
    for (auto *argument : *call->args) addTermUse(argument, result);
}

TempSet statementDefs(quad::QuadStm *statement) {
    TempSet result;
    if (statement == nullptr) return result;
    quad::QuadTemp *destination = nullptr;
    switch (statement->kind) {
    case quad::QuadKind::MOVE:
        destination = static_cast<quad::QuadMove *>(statement)->dst;
        break;
    case quad::QuadKind::LOAD:
        destination = static_cast<quad::QuadLoad *>(statement)->dst;
        break;
    case quad::QuadKind::MOVE_BINOP:
        destination = static_cast<quad::QuadMoveBinop *>(statement)->dst;
        break;
    case quad::QuadKind::MOVE_CALL:
        destination = static_cast<quad::QuadMoveCall *>(statement)->dst;
        break;
    case quad::QuadKind::MOVE_EXTCALL:
        destination = static_cast<quad::QuadMoveExtCall *>(statement)->dst;
        break;
    case quad::QuadKind::PHI:
        destination = static_cast<quad::QuadPhi *>(statement)->temp_exp;
        break;
    case quad::QuadKind::PTR_CALC: {
        auto *term = static_cast<quad::QuadPtrCalc *>(statement)->dst;
        if (term != nullptr && term->kind == quad::QuadTermKind::TEMP) {
            destination = term->get_temp();
        }
        break;
    }
    default:
        break;
    }
    if (destination != nullptr && destination->temp != nullptr) {
        result.insert(destination->temp->num);
    }
    return result;
}

TempSet statementUses(quad::QuadStm *statement) {
    TempSet result;
    if (statement == nullptr) return result;
    switch (statement->kind) {
    case quad::QuadKind::MOVE:
        addTermUse(static_cast<quad::QuadMove *>(statement)->src, result);
        break;
    case quad::QuadKind::LOAD:
        addTermUse(static_cast<quad::QuadLoad *>(statement)->src, result);
        break;
    case quad::QuadKind::STORE: {
        auto *store = static_cast<quad::QuadStore *>(statement);
        addTermUse(store->src, result);
        addTermUse(store->dst, result);
        break;
    }
    case quad::QuadKind::MOVE_BINOP: {
        auto *binop = static_cast<quad::QuadMoveBinop *>(statement);
        addTermUse(binop->left, result);
        addTermUse(binop->right, result);
        break;
    }
    case quad::QuadKind::CALL:
        addCallUses(static_cast<quad::QuadCall *>(statement), result);
        break;
    case quad::QuadKind::MOVE_CALL:
        addCallUses(static_cast<quad::QuadMoveCall *>(statement)->call, result);
        break;
    case quad::QuadKind::EXTCALL:
        addExtCallUses(static_cast<quad::QuadExtCall *>(statement), result);
        break;
    case quad::QuadKind::MOVE_EXTCALL:
        addExtCallUses(
            static_cast<quad::QuadMoveExtCall *>(statement)->extcall, result);
        break;
    case quad::QuadKind::CJUMP: {
        auto *branch = static_cast<quad::QuadCJump *>(statement);
        addTermUse(branch->left, result);
        addTermUse(branch->right, result);
        break;
    }
    case quad::QuadKind::PHI: {
        auto *phi = static_cast<quad::QuadPhi *>(statement);
        if (phi->args != nullptr) {
            for (const auto &argument : *phi->args) {
                if (argument.first != nullptr) result.insert(argument.first->num);
            }
        }
        break;
    }
    case quad::QuadKind::RETURN:
        addTermUse(static_cast<quad::QuadReturn *>(statement)->exp, result);
        break;
    case quad::QuadKind::PTR_CALC: {
        auto *calculation = static_cast<quad::QuadPtrCalc *>(statement);
        addTermUse(calculation->ptr, result);
        addTermUse(calculation->offset, result);
        break;
    }
    default:
        break;
    }
    return result;
}

bool isCall(quad::QuadStm *statement) {
    if (statement == nullptr) return false;
    switch (statement->kind) {
    case quad::QuadKind::CALL:
    case quad::QuadKind::MOVE_CALL:
    case quad::QuadKind::EXTCALL:
    case quad::QuadKind::MOVE_EXTCALL:
        return true;
    default:
        return false;
    }
}

int accessWeight(int loopDepth) {
    if (loopDepth >= 3) return 256;
    if (loopDepth == 2) return 64;
    if (loopDepth == 1) return 8;
    return 1;
}

void touch(Interval &interval, int position, std::size_t block) {
    interval.start = std::min(interval.start, position);
    interval.end = std::max(interval.end, position);
    interval.blocks.insert(block);
}

void noteAccess(Interval &interval, int position, std::size_t block,
                int weight, bool isUse) {
    touch(interval, position, block);
    interval.weightedAccesses += weight;
    ++interval.accesses;
    if (isUse) ++interval.uses;
}

} // namespace

Aarch64RegisterAllocation allocateAarch64Gprs(
    quad::QuadFuncDecl *function,
    const std::unordered_map<int, quad::QuadType> &tempTypes,
    const std::unordered_set<int> &rematerializedTemps) {
    Aarch64RegisterAllocation result;
    if (function == nullptr || function->quadblocklist == nullptr ||
        function->quadblocklist->empty()) {
        return result;
    }

    const auto &blocks = *function->quadblocklist;
    std::vector<BlockInfo> info(blocks.size());
    std::unordered_map<int, std::size_t> labelToBlock;
    std::unordered_map<quad::QuadStm *, std::pair<int, int>> positions;
    int cursor = 0;

    for (std::size_t index = 0; index < blocks.size(); ++index) {
        auto *block = blocks[index];
        if (block == nullptr || block->entry_label == nullptr ||
            block->quadlist == nullptr) {
            return result;
        }
        info[index].block = block;
        info[index].start = cursor;
        info[index].label = block->entry_label->num;
        // Duplicate labels make the CFG ambiguous.  Returning an empty
        // allocation is a correctness-preserving stack-code fallback.
        if (!labelToBlock.emplace(info[index].label, index).second) {
            return result;
        }
        for (auto *statement : *block->quadlist) {
            positions[statement] = {cursor, cursor + 1};
            cursor += 2;
        }
        info[index].end = cursor;
        cursor += 2;
    }

    auto addSuccessor = [&](std::size_t from, tree::Label *label) -> bool {
        if (label == nullptr) return true;
        auto found = labelToBlock.find(label->num);
        if (found == labelToBlock.end()) return false;
        auto &successors = info[from].successors;
        if (std::find(successors.begin(), successors.end(), found->second) ==
            successors.end()) {
            successors.push_back(found->second);
        }
        return true;
    };

    for (std::size_t index = 0; index < blocks.size(); ++index) {
        auto *block = blocks[index];
        if (block == nullptr) continue;
        bool hasExplicitTerminator = false;
        if (block->quadlist != nullptr) {
            for (auto it = block->quadlist->rbegin();
                 it != block->quadlist->rend(); ++it) {
                auto *statement = *it;
                if (statement == nullptr) continue;
                if (statement->kind == quad::QuadKind::JUMP) {
                    hasExplicitTerminator = true;
                    if (!addSuccessor(index,
                                      static_cast<quad::QuadJump *>(statement)->label)) {
                        return result;
                    }
                } else if (statement->kind == quad::QuadKind::CJUMP) {
                    hasExplicitTerminator = true;
                    auto *branch = static_cast<quad::QuadCJump *>(statement);
                    if (!addSuccessor(index, branch->t) ||
                        !addSuccessor(index, branch->f)) {
                        return result;
                    }
                } else if (statement->kind == quad::QuadKind::RETURN) {
                    hasExplicitTerminator = true;
                }
                break;
            }
        }
        if (!hasExplicitTerminator && block->exit_labels != nullptr) {
            for (auto *label : *block->exit_labels) {
                if (!addSuccessor(index, label)) return result;
            }
        }
    }

    // Backward edges produced by structured lowering provide a robust and
    // deliberately conservative loop-depth estimate for spill priorities.
    for (std::size_t index = 0; index < info.size(); ++index) {
        for (std::size_t successor : info[index].successors) {
            if (successor > index) continue;
            for (std::size_t member = successor; member <= index; ++member) {
                ++info[member].loopDepth;
            }
        }
    }

    // Build block use/def sets.  PHI inputs are not ordinary successor-block
    // uses: each belongs to exactly one incoming CFG edge.
    for (std::size_t index = 0; index < info.size(); ++index) {
        auto *block = info[index].block;
        if (block == nullptr || block->quadlist == nullptr) continue;
        bool pastPhiPrefix = false;
        TempSet seenPhiDestinations;
        for (auto *statement : *block->quadlist) {
            if (statement == nullptr) continue;
            if (statement->kind == quad::QuadKind::LABEL) continue;
            if (statement->kind == quad::QuadKind::PHI) {
                if (pastPhiPrefix) return Aarch64RegisterAllocation{};
                auto *phi = static_cast<quad::QuadPhi *>(statement);
                if (phi->temp_exp == nullptr || phi->temp_exp->temp == nullptr ||
                    phi->args == nullptr) {
                    return Aarch64RegisterAllocation{};
                }
                int destination = phi->temp_exp->temp->num;
                if (!seenPhiDestinations.insert(destination).second) {
                    return Aarch64RegisterAllocation{};
                }
                info[index].defs.insert(destination);
                info[index].phiDefs.insert(destination);
                ++info[index].phiStatementCount;
                for (const auto &argument : *phi->args) {
                    if (argument.first == nullptr || argument.second == nullptr ||
                        labelToBlock.find(argument.second->num) == labelToBlock.end()) {
                        return Aarch64RegisterAllocation{};
                    }
                    info[index]
                        .phiUsesByPredecessor[argument.second->num]
                        .insert(argument.first->num);
                }
                continue;
            }
            pastPhiPrefix = true;
            TempSet uses = statementUses(statement);
            TempSet defs = statementDefs(statement);
            for (int temp : uses) {
                if (info[index].defs.count(temp) == 0) {
                    info[index].uses.insert(temp);
                }
            }
            insertAll(info[index].defs, defs);
        }
    }

    // maxPhiCopies is per edge/block, not the total number of PHIs.
    result.maxPhiCopies = 0;
    for (const BlockInfo &block : info) {
        result.maxPhiCopies = std::max(result.maxPhiCopies,
                                       block.phiStatementCount);
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t reverse = info.size(); reverse > 0; --reverse) {
            std::size_t index = reverse - 1;
            TempSet newOut;
            for (std::size_t successor : info[index].successors) {
                for (int temp : info[successor].liveIn) {
                    if (info[successor].phiDefs.count(temp) == 0) {
                        newOut.insert(temp);
                    }
                }
                auto phiUses = info[successor].phiUsesByPredecessor.find(
                    info[index].label);
                if (phiUses != info[successor].phiUsesByPredecessor.end()) {
                    insertAll(newOut, phiUses->second);
                }
            }
            TempSet newIn = info[index].uses;
            for (int temp : newOut) {
                if (info[index].defs.count(temp) == 0) newIn.insert(temp);
            }
            if (newOut != info[index].liveOut || newIn != info[index].liveIn) {
                info[index].liveOut = std::move(newOut);
                info[index].liveIn = std::move(newIn);
                changed = true;
            }
        }
    }

    std::unordered_map<int, Interval> byTemp;
    for (const auto &entry : tempTypes) {
        if (rematerializedTemps.count(entry.first) != 0) continue;
        byTemp[entry.first].temp = entry.first;
    }
    auto intervalFor = [&](int temp) -> Interval * {
        if (rematerializedTemps.count(temp) != 0) return nullptr;
        auto found = byTemp.find(temp);
        if (found == byTemp.end()) {
            // Unknown type means the emitter cannot select the correct w/x
            // width.  Leave such a temp in its stack slot.
            return nullptr;
        }
        return &found->second;
    };

    if (function->params != nullptr) {
        for (auto *parameter : *function->params) {
            if (parameter == nullptr) continue;
            Interval *interval = intervalFor(parameter->num);
            if (interval == nullptr) continue;
            interval->parameter = true;
            noteAccess(*interval, info.front().start, 0, 1, false);
        }
    }

    for (std::size_t index = 0; index < info.size(); ++index) {
        BlockInfo &blockInfo = info[index];
        int weight = accessWeight(blockInfo.loopDepth);
        for (int temp : blockInfo.liveIn) {
            if (Interval *interval = intervalFor(temp)) {
                touch(*interval, blockInfo.start, index);
            }
        }
        for (int temp : blockInfo.liveOut) {
            if (Interval *interval = intervalFor(temp)) {
                touch(*interval, blockInfo.end, index);
            }
        }

        auto *block = blockInfo.block;
        if (block == nullptr || block->quadlist == nullptr) continue;
        for (auto *statement : *block->quadlist) {
            if (statement == nullptr) continue;
            auto position = positions.find(statement);
            if (position == positions.end()) continue;
            if (statement->kind == quad::QuadKind::PHI) {
                auto *phi = static_cast<quad::QuadPhi *>(statement);
                if (Interval *interval = intervalFor(phi->temp_exp->temp->num)) {
                    noteAccess(*interval, blockInfo.start, index, weight, false);
                }
                for (const auto &argument : *phi->args) {
                    auto predecessor = labelToBlock.find(argument.second->num);
                    if (predecessor == labelToBlock.end()) continue;
                    std::size_t predecessorIndex = predecessor->second;
                    if (Interval *interval = intervalFor(argument.first->num)) {
                        noteAccess(*interval, info[predecessorIndex].end,
                                   predecessorIndex,
                                   accessWeight(info[predecessorIndex].loopDepth),
                                   true);
                    }
                }
                continue;
            }
            for (int temp : statementUses(statement)) {
                if (Interval *interval = intervalFor(temp)) {
                    noteAccess(*interval, position->second.first, index, weight,
                               true);
                }
            }
            for (int temp : statementDefs(statement)) {
                if (Interval *interval = intervalFor(temp)) {
                    noteAccess(*interval, position->second.second, index, weight,
                               false);
                }
            }
        }
    }

    // Walk exact statement liveness backwards to identify values that must
    // survive a call.  Call operands die before the ABI clobber and call
    // results are born afterwards, so both remain eligible for x8 unless used
    // across some other call.
    for (std::size_t index = 0; index < info.size(); ++index) {
        TempSet live = info[index].liveOut;
        auto *block = info[index].block;
        if (block == nullptr || block->quadlist == nullptr) continue;
        for (auto iterator = block->quadlist->rbegin();
             iterator != block->quadlist->rend(); ++iterator) {
            auto *statement = *iterator;
            if (statement == nullptr || statement->kind == quad::QuadKind::PHI) {
                continue;
            }
            TempSet defs = statementDefs(statement);
            if (isCall(statement)) {
                for (int temp : live) {
                    if (defs.count(temp) != 0) continue;
                    if (Interval *interval = intervalFor(temp)) {
                        interval->liveAcrossCall = true;
                    }
                }
            }
            for (int temp : defs) live.erase(temp);
            insertAll(live, statementUses(statement));
        }
    }

    std::vector<Interval> intervals;
    intervals.reserve(byTemp.size());
    for (auto &entry : byTemp) {
        if (entry.second.valid()) intervals.push_back(std::move(entry.second));
    }
    std::sort(intervals.begin(), intervals.end(), [](const Interval &left,
                                                     const Interval &right) {
        if (left.start != right.start) return left.start < right.start;
        if (left.end != right.end) return left.end < right.end;
        return left.temp < right.temp;
    });
    result.intervalCount = intervals.size();

    static constexpr int kCallerSavedRegister = 8;
    static constexpr std::array<int, 10> kCalleeSavedRegisters = {
        19, 20, 21, 22, 23, 24, 25, 26, 27, 28};
    struct Active {
        std::size_t interval = 0;
        int reg = -1;
    };
    std::vector<Active> active;

    for (std::size_t currentIndex = 0; currentIndex < intervals.size();
         ++currentIndex) {
        const Interval &current = intervals[currentIndex];
        active.erase(std::remove_if(active.begin(), active.end(),
                                    [&](const Active &entry) {
                                        return intervals[entry.interval].end <
                                               current.start;
                                    }),
                     active.end());

        std::unordered_set<int> occupied;
        for (const Active &entry : active) occupied.insert(entry.reg);
        int selected = -1;
        if (!current.liveAcrossCall &&
            occupied.count(kCallerSavedRegister) == 0) {
            selected = kCallerSavedRegister;
        }
        if (selected < 0) {
            for (int reg : kCalleeSavedRegisters) {
                if (occupied.count(reg) == 0) {
                    selected = reg;
                    break;
                }
            }
        }

        if (selected < 0) {
            // Whole-interval spilling keeps the emitter integration simple:
            // an evicted temp falls back to its frame slot for all accesses.
            // Prefer evicting the least profitable active interval that owns a
            // register legal for the current call-crossing class.
            auto victim = active.end();
            for (auto iterator = active.begin(); iterator != active.end();
                 ++iterator) {
                if (current.liveAcrossCall &&
                    iterator->reg == kCallerSavedRegister) {
                    continue;
                }
                if (victim == active.end() ||
                    intervals[iterator->interval].priority() <
                        intervals[victim->interval].priority() ||
                    (intervals[iterator->interval].priority() ==
                         intervals[victim->interval].priority() &&
                     intervals[iterator->interval].end >
                         intervals[victim->interval].end)) {
                    victim = iterator;
                }
            }
            if (victim != active.end() &&
                intervals[victim->interval].priority() < current.priority()) {
                selected = victim->reg;
                result.tempToRegister.erase(
                    intervals[victim->interval].temp);
                active.erase(victim);
            }
        }

        if (selected < 0) {
            ++result.spilledIntervalCount;
            continue;
        }
        result.tempToRegister[current.temp] = selected;
        active.push_back(Active{currentIndex, selected});
    }

    std::set<int> usedCalleeSaved;
    for (const auto &entry : result.tempToRegister) {
        if (entry.second >= 19 && entry.second <= 28) {
            usedCalleeSaved.insert(entry.second);
        }
    }
    result.usedCalleeSavedRegisters.assign(usedCalleeSaved.begin(),
                                           usedCalleeSaved.end());
    // Evicted intervals are also spills even though they were tentatively
    // assigned earlier in the scan.
    result.spilledIntervalCount = result.intervalCount -
                                  result.tempToRegister.size();
    return result;
}

} // namespace backend
