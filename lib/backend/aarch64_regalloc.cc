#include "aarch64_regalloc.hh"

#include <algorithm>
#include <limits>
#include <set>
#include <string>
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

// A half-open portion of the linearized instruction stream in which an SSA
// value is live.  Keeping the portions separate is important: the old
// allocator represented every value by one [first,last] envelope, so values
// with a dead region in the middle unnecessarily interfered.
struct LiveSegment {
    int start = 0;
    int end = 0;
};

struct Interval {
    int temp = -1;
    quad::QuadType type = quad::QuadType::INT;
    int start = std::numeric_limits<int>::max();
    int end = std::numeric_limits<int>::min();
    long long weightedAccesses = 0;
    int accesses = 0;
    int uses = 0;
    bool parameter = false;
    bool liveAcrossCall = false;
    // A value consumed by an ABI call argument cannot use x0-x7/s0-s7 as its
    // home.  Argument setup writes those registers left-to-right, so keeping
    // an argument itself in one of them would make a later argument move
    // observe a clobbered source.  x8 (and s16-s29) remain legal for an
    // argument because the marshaller does not overwrite them before the
    // call.
    bool callArgument = false;
    std::unordered_set<std::size_t> blocks;
    std::vector<LiveSegment> segments;

    bool valid() const {
        return temp >= 0 && start <= end && uses != 0 && !segments.empty();
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

bool segmentsOverlap(const LiveSegment &left, const LiveSegment &right) {
    return left.start < right.end && right.start < left.end;
}

bool intervalsOverlap(const Interval &left, const Interval &right) {
    std::size_t l = 0;
    std::size_t r = 0;
    while (l < left.segments.size() && r < right.segments.size()) {
        if (segmentsOverlap(left.segments[l], right.segments[r])) return true;
        if (left.segments[l].end <= right.segments[r].start) {
            ++l;
        } else {
            ++r;
        }
    }
    return false;
}

void normalizeSegments(Interval &interval) {
    auto &segments = interval.segments;
    std::sort(segments.begin(), segments.end(),
              [](const LiveSegment &left, const LiveSegment &right) {
                  if (left.start != right.start) return left.start < right.start;
                  return left.end < right.end;
              });
    std::vector<LiveSegment> normalized;
    for (const LiveSegment &segment : segments) {
        if (segment.start >= segment.end) continue;
        if (!normalized.empty() && segment.start <= normalized.back().end) {
            normalized.back().end = std::max(normalized.back().end, segment.end);
        } else {
            normalized.push_back(segment);
        }
    }
    segments = std::move(normalized);
    if (segments.empty() && interval.start <= interval.end) {
        // Malformed/hand-built Quad input can omit precise statement points.
        // Retain the old conservative envelope rather than allocating an
        // untracked value in a physical register.
        segments.push_back({interval.start, interval.end + 1});
    }
}

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

bool isCall(quad::QuadStm *statement);

// Record the values which are consumed by the target ABI argument marshaller.
// This is intentionally separate from addCallUses: the object term of an
// indirect QuadCall is loaded through x15 and therefore is not subject to the
// x0-x7 argument-register clobber rule.  Likewise, inlined floating helpers
// are not ABI calls and are handled directly by the emitter.
void addCallArgumentUses(quad::QuadStm *statement, TempSet &result) {
    if (statement == nullptr || !isCall(statement)) return;
    switch (statement->kind) {
    case quad::QuadKind::CALL:
        if (auto *call = static_cast<quad::QuadCall *>(statement);
            call != nullptr && call->args != nullptr) {
            for (auto *argument : *call->args) addTermUse(argument, result);
        }
        break;
    case quad::QuadKind::MOVE_CALL: {
        auto *move = static_cast<quad::QuadMoveCall *>(statement);
        auto *call = move == nullptr ? nullptr : move->call;
        if (call != nullptr && call->args != nullptr) {
            for (auto *argument : *call->args) addTermUse(argument, result);
        }
        break;
    }
    case quad::QuadKind::EXTCALL:
        addExtCallUses(static_cast<quad::QuadExtCall *>(statement), result);
        break;
    case quad::QuadKind::MOVE_EXTCALL: {
        auto *move = static_cast<quad::QuadMoveExtCall *>(statement);
        addExtCallUses(move == nullptr ? nullptr : move->extcall, result);
        break;
    }
    default:
        break;
    }
}

struct AddressFusionOverrides {
    std::unordered_map<quad::QuadStm *, quad::QuadPtrCalc *> byMemoryStatement;
    std::unordered_set<quad::QuadStm *> elidedCalculations;
};

TempSet statementDefs(quad::QuadStm *statement,
                      const AddressFusionOverrides &addressFusions) {
    TempSet result;
    if (statement == nullptr) return result;
    if (addressFusions.elidedCalculations.count(statement) != 0) {
        return result;
    }
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

TempSet statementUses(quad::QuadStm *statement,
                      const AddressFusionOverrides &addressFusions) {
    TempSet result;
    if (statement == nullptr) return result;
    if (addressFusions.elidedCalculations.count(statement) != 0) {
        return result;
    }
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
    auto fused = addressFusions.byMemoryStatement.find(statement);
    if (fused != addressFusions.byMemoryStatement.end() &&
        fused->second != nullptr) {
        auto *calculation = fused->second;
        auto *destination = calculation->dst == nullptr
                                ? nullptr
                                : calculation->dst->kind ==
                                          quad::QuadTermKind::TEMP
                                      ? calculation->dst->get_temp()
                                      : nullptr;
        if (destination != nullptr && destination->temp != nullptr) {
            result.erase(destination->temp->num);
        }
        addTermUse(calculation->ptr, result);
        addTermUse(calculation->offset, result);
    }
    return result;
}

bool isCall(quad::QuadStm *statement) {
    if (statement == nullptr) return false;
    auto isInlinedFloatHelper = [](const std::string &name) {
        return name == "__sysy_i2f_bits" || name == "__sysy_f2i_bits" ||
               name == "__sysy_fadd_bits" || name == "__sysy_fsub_bits" ||
               name == "__sysy_fmul_bits" || name == "__sysy_fdiv_bits" ||
               name == "__sysy_fneg_bits" || name == "__sysy_fcmpeq_bits" ||
               name == "__sysy_fcmpne_bits" || name == "__sysy_fcmplt_bits" ||
               name == "__sysy_fcmple_bits" || name == "__sysy_fcmpgt_bits" ||
               name == "__sysy_fcmpge_bits";
    };
    switch (statement->kind) {
    case quad::QuadKind::CALL:
    case quad::QuadKind::MOVE_CALL:
        return true;
    case quad::QuadKind::EXTCALL: {
        auto *call = static_cast<quad::QuadExtCall *>(statement);
        return call == nullptr || !isInlinedFloatHelper(call->extfun);
    }
    case quad::QuadKind::MOVE_EXTCALL: {
        auto *move = static_cast<quad::QuadMoveExtCall *>(statement);
        return move == nullptr || move->extcall == nullptr ||
               !isInlinedFloatHelper(move->extcall->extfun);
    }
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
    const std::unordered_set<int> &rematerializedTemps,
    const std::vector<Aarch64FusedAddress> &fusedAddresses) {
    Aarch64RegisterAllocation result;
    if (function == nullptr || function->quadblocklist == nullptr ||
        function->quadblocklist->empty()) {
        return result;
    }

    AddressFusionOverrides addressFusions;
    for (const auto &fusion : fusedAddresses) {
        if (fusion.memoryStatement == nullptr ||
            fusion.pointerCalculation == nullptr) {
            continue;
        }
        addressFusions.byMemoryStatement.emplace(fusion.memoryStatement,
                                                 fusion.pointerCalculation);
        addressFusions.elidedCalculations.insert(fusion.pointerCalculation);
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
            TempSet uses = statementUses(statement, addressFusions);
            TempSet defs = statementDefs(statement, addressFusions);
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
    TempSet callArgumentTemps;
    // Coalescing preferences never override interference or ABI legality.
    // They merely bias a destination toward a MOVE/PHI input's now-free home,
    // allowing the emitter to omit the corresponding copy when adjacent live
    // ranges do not overlap.
    std::unordered_map<int, std::vector<int>> affinities;
    for (const auto &entry : tempTypes) {
        if (rematerializedTemps.count(entry.first) != 0) continue;
        byTemp[entry.first].temp = entry.first;
        byTemp[entry.first].type = entry.second;
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
            addCallArgumentUses(statement, callArgumentTemps);
            auto position = positions.find(statement);
            if (position == positions.end()) continue;
            if (statement->kind == quad::QuadKind::PHI) {
                auto *phi = static_cast<quad::QuadPhi *>(statement);
                int destination = phi->temp_exp->temp->num;
                if (Interval *interval = intervalFor(phi->temp_exp->temp->num)) {
                    noteAccess(*interval, blockInfo.start, index, weight, false);
                }
                for (const auto &argument : *phi->args) {
                    if (argument.first != nullptr &&
                        argument.first->num != destination) {
                        affinities[destination].push_back(argument.first->num);
                        // PHI backedge inputs are commonly defined after the
                        // destination interval has ended.  Record the reverse
                        // preference as well so the later-defined input can
                        // reuse the PHI home's now-free register.  The scan's
                        // occupancy and call-crossing checks below still have
                        // final authority, so this cannot override real
                        // interference or ABI legality.
                        affinities[argument.first->num].push_back(destination);
                    }
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
            if (statement->kind == quad::QuadKind::MOVE) {
                auto *move = static_cast<quad::QuadMove *>(statement);
                auto *source = move == nullptr ? nullptr : move->src;
                auto *sourceTemp = source != nullptr &&
                                           source->kind == quad::QuadTermKind::TEMP
                                       ? source->get_temp()
                                       : nullptr;
                if (move != nullptr && move->dst != nullptr &&
                    move->dst->temp != nullptr && source != nullptr &&
                    source->kind == quad::QuadTermKind::TEMP &&
                    sourceTemp != nullptr && sourceTemp->temp != nullptr &&
                    sourceTemp->temp->num != move->dst->temp->num) {
                    affinities[move->dst->temp->num].push_back(
                        sourceTemp->temp->num);
                    affinities[sourceTemp->temp->num].push_back(
                        move->dst->temp->num);
                }
            }
            for (int temp : statementUses(statement, addressFusions)) {
                if (Interval *interval = intervalFor(temp)) {
                    noteAccess(*interval, position->second.first, index, weight,
                               true);
                }
            }
            for (int temp : statementDefs(statement, addressFusions)) {
                if (Interval *interval = intervalFor(temp)) {
                    noteAccess(*interval, position->second.second, index, weight,
                               false);
                }
            }
        }
    }
    for (auto &entry : byTemp) {
        entry.second.callArgument = callArgumentTemps.count(entry.first) != 0;
    }

    // Build statement-granular live segments.  The block data-flow sets above
    // tell us which values cross a CFG edge; walking each block backwards
    // tells us where a value is actually live inside that block.  We record
    // discrete instruction points and then turn consecutive points into
    // half-open ranges.  A value used early in a block and redefined before a
    // later use therefore has a real hole instead of one conservative envelope.
    for (std::size_t index = 0; index < info.size(); ++index) {
        BlockInfo &blockInfo = info[index];
        auto *block = blockInfo.block;
        if (block == nullptr || block->quadlist == nullptr) continue;

        std::unordered_map<int, std::vector<int>> points;
        auto mark = [&](int temp, int point) {
            points[temp].push_back(point);
        };
        TempSet live = blockInfo.liveOut;
        for (auto iterator = block->quadlist->rbegin();
             iterator != block->quadlist->rend(); ++iterator) {
            auto *statement = *iterator;
            if (statement == nullptr) continue;
            auto position = positions.find(statement);
            if (position == positions.end()) continue;

            TempSet defs = statementDefs(statement, addressFusions);
            TempSet uses;
            // PHI operands are live on their predecessor edges and are not
            // local uses in the destination block.
            if (statement->kind != quad::QuadKind::PHI) {
                uses = statementUses(statement, addressFusions);
            }
            const int first = position->second.first;
            const int last = position->second.second;
            for (int temp : live) {
                mark(temp, first);
                mark(temp, last);
            }
            for (int temp : uses) {
                mark(temp, first);
                mark(temp, last);
            }
            for (int temp : defs) live.erase(temp);
            insertAll(live, uses);
        }

        // Preserve the live-in/out boundary even for an empty block or a
        // value whose first/last use is on a neighboring edge.
        for (int temp : blockInfo.liveIn) {
            mark(temp, blockInfo.start);
            mark(temp, blockInfo.start + 1);
        }
        for (int temp : blockInfo.liveOut) {
            mark(temp, blockInfo.end);
            mark(temp, blockInfo.end + 1);
        }

        for (auto &entry : points) {
            auto *interval = intervalFor(entry.first);
            if (interval == nullptr) continue;
            auto &values = entry.second;
            std::sort(values.begin(), values.end());
            values.erase(std::unique(values.begin(), values.end()), values.end());
            if (values.empty()) continue;
            int runStart = values.front();
            int previous = values.front();
            for (std::size_t point = 1; point < values.size(); ++point) {
                if (values[point] <= previous + 1) {
                    previous = values[point];
                    continue;
                }
                interval->segments.push_back({runStart, previous + 1});
                runStart = previous = values[point];
            }
            interval->segments.push_back({runStart, previous + 1});
        }
    }
    for (auto &entry : byTemp) normalizeSegments(entry.second);

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
            TempSet defs = statementDefs(statement, addressFusions);
            if (isCall(statement)) {
                for (int temp : live) {
                    if (defs.count(temp) != 0) continue;
                    if (Interval *interval = intervalFor(temp)) {
                        interval->liveAcrossCall = true;
                    }
                }
            }
            for (int temp : defs) live.erase(temp);
            insertAll(live, statementUses(statement, addressFusions));
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

    struct Active {
        std::size_t interval = 0;
        int reg = -1;
    };
    auto registerUnavailable = [](const Interval &interval, int reg,
                                  const std::unordered_set<int> &callerSaved) {
        if (interval.liveAcrossCall && callerSaved.count(reg) != 0) {
            return true;
        }
        // x0-x7 and s0-s7 are overwritten while ABI arguments are prepared.
        // A call argument may still use x8/s16-s29, which are not touched by
        // the register-argument moves themselves.
        return interval.callArgument && reg >= 0 && reg <= 7;
    };
    auto allocateBank = [&](const std::vector<std::size_t> &bank,
                            const std::vector<int> &callerSaved,
                            const std::vector<int> &calleeSaved,
                            std::unordered_map<int, int> &homes) {
        std::vector<Active> active;
        std::unordered_set<int> callerSavedSet(callerSaved.begin(),
                                               callerSaved.end());
        for (std::size_t currentIndex : bank) {
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
            auto preferred = affinities.find(current.temp);
            if (preferred != affinities.end()) {
                for (int source : preferred->second) {
                    auto sourceHome = homes.find(source);
                    if (sourceHome == homes.end() ||
                        occupied.count(sourceHome->second) != 0 ||
                        registerUnavailable(current, sourceHome->second,
                                            callerSavedSet)) {
                        continue;
                    }
                    selected = sourceHome->second;
                    break;
                }
            }
            if (!current.liveAcrossCall && selected < 0) {
                for (int reg : callerSaved) {
                    if (current.callArgument && reg >= 0 && reg <= 7) {
                        continue;
                    }
                    if (occupied.count(reg) == 0) {
                        selected = reg;
                        break;
                    }
                }
            }
            if (selected < 0) {
                for (int reg : calleeSaved) {
                    if (occupied.count(reg) == 0) {
                        selected = reg;
                        break;
                    }
                }
            }

            if (selected < 0) {
                // Whole-interval spilling keeps the emitter integration
                // simple. Prefer evicting the least profitable active range
                // that owns a register legal for this call-crossing class.
                auto victim = active.end();
                for (auto iterator = active.begin(); iterator != active.end();
                     ++iterator) {
                    if (registerUnavailable(current, iterator->reg,
                                            callerSavedSet)) {
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
                    homes.erase(intervals[victim->interval].temp);
                    active.erase(victim);
                }
            }

            if (selected < 0) continue;
            homes[current.temp] = selected;
            active.push_back(Active{currentIndex, selected});
        }
    };

    std::vector<std::size_t> gprIntervals;
    std::vector<std::size_t> floatIntervals;
    for (std::size_t index = 0; index < intervals.size(); ++index) {
        if (intervals[index].type == quad::QuadType::FLOAT) {
            floatIntervals.push_back(index);
        } else {
            gprIntervals.push_back(index);
        }
    }
    allocateBank(gprIntervals, {0, 1, 2, 3, 4, 5, 6, 7, 8},
                 {19, 20, 21, 22, 23, 24, 25, 26, 27, 28},
                 result.tempToRegister);
    allocateBank(floatIntervals,
                 {0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23,
                  24, 25, 26, 27, 28, 29},
                 {8, 9, 10, 11, 12, 13, 14, 15},
                 result.tempToFloatRegister);

    // The legacy scan intentionally keeps a whole [start,end] envelope, so a
    // range that could only fit in a dead hole is often left on the stack.
    // Recover those values opportunistically without changing any already
    // selected home: this preserves direct-home MOVE/PHI behavior while
    // making the new segment information useful.  A candidate is assigned
    // only when no assigned interval with the same physical register has an
    // overlapping segment, and call-crossing values never use x8/s16-s29.
    auto fillLifetimeHoles = [&](const std::vector<std::size_t> &bank,
                                 const std::vector<int> &callerSaved,
                                 const std::vector<int> &calleeSaved,
                                 std::unordered_map<int, int> &homes) {
        std::unordered_set<int> callerSavedSet(callerSaved.begin(),
                                               callerSaved.end());
        std::vector<std::size_t> candidates;
        for (std::size_t index : bank) {
            if (homes.find(intervals[index].temp) == homes.end()) {
                candidates.push_back(index);
            }
        }
        std::sort(candidates.begin(), candidates.end(),
                  [&](std::size_t left, std::size_t right) {
                      long long lp = intervals[left].priority();
                      long long rp = intervals[right].priority();
                      if (lp != rp) return lp > rp;
                      if (intervals[left].segments.size() !=
                          intervals[right].segments.size()) {
                          return intervals[left].segments.size() >
                                 intervals[right].segments.size();
                      }
                      return intervals[left].start < intervals[right].start;
                  });

        auto occupiedBy = [&](std::size_t current, int reg) {
            const Interval &candidate = intervals[current];
            if (registerUnavailable(candidate, reg, callerSavedSet)) {
                return true;
            }
            for (std::size_t other : bank) {
                auto home = homes.find(intervals[other].temp);
                if (home == homes.end() || home->second != reg ||
                    other == current) {
                    continue;
                }
                if (intervalsOverlap(candidate, intervals[other])) return true;
            }
            return false;
        };

        for (std::size_t current : candidates) {
            if (homes.find(intervals[current].temp) != homes.end()) continue;
            std::vector<int> choices;
            std::unordered_set<int> seen;
            auto addChoice = [&](int reg) {
                if (seen.insert(reg).second) choices.push_back(reg);
            };
            auto preferred = affinities.find(intervals[current].temp);
            if (preferred != affinities.end()) {
                for (int source : preferred->second) {
                    auto home = homes.find(source);
                    if (home != homes.end()) addChoice(home->second);
                }
            }
            if (!intervals[current].liveAcrossCall) {
                for (int reg : callerSaved) {
                    if (intervals[current].callArgument && reg >= 0 && reg <= 7) {
                        continue;
                    }
                    addChoice(reg);
                }
            }
            for (int reg : calleeSaved) addChoice(reg);
            for (int reg : choices) {
                if (occupiedBy(current, reg)) continue;
                homes[intervals[current].temp] = reg;
                break;
            }
        }
    };
    fillLifetimeHoles(gprIntervals, {0, 1, 2, 3, 4, 5, 6, 7, 8},
                      {19, 20, 21, 22, 23, 24, 25, 26, 27, 28},
                      result.tempToRegister);
    fillLifetimeHoles(floatIntervals,
                      {0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22,
                       23, 24, 25, 26, 27, 28, 29},
                      {8, 9, 10, 11, 12, 13, 14, 15},
                      result.tempToFloatRegister);

    // Affinity is stronger than a first-fit preference when two SSA ranges
    // do not interfere.  Coalesce such MOVE/PHI pairs after the conservative
    // allocation above: move only the lower-priority range to the established
    // home of the hotter range, and never displace a third overlapping value.
    // This keeps the emitter's one-home invariant while removing physical
    // copies that the ordinary linear scan could not coalesce due to scan
    // order.
    auto coalesceBank = [&](const std::vector<std::size_t> &bank,
                            const std::vector<int> &callerSaved,
                            std::unordered_map<int, int> &homes) {
        std::unordered_set<int> callerSavedSet(callerSaved.begin(),
                                               callerSaved.end());
        std::unordered_map<int, std::size_t> indexByTemp;
        for (std::size_t index : bank) indexByTemp[intervals[index].temp] = index;

        auto conflictsAt = [&](std::size_t moving, int target) {
            const Interval &candidate = intervals[moving];
            if (registerUnavailable(candidate, target, callerSavedSet)) {
                return true;
            }
            for (std::size_t other : bank) {
                if (other == moving) continue;
                auto home = homes.find(intervals[other].temp);
                if (home == homes.end() || home->second != target) continue;
                if (intervalsOverlap(candidate, intervals[other])) return true;
            }
            return false;
        };

        std::vector<std::pair<int, int>> edges;
        for (const auto &entry : affinities) {
            auto left = indexByTemp.find(entry.first);
            if (left == indexByTemp.end()) continue;
            for (int source : entry.second) {
                auto right = indexByTemp.find(source);
                if (right == indexByTemp.end() || entry.first == source) continue;
                if (entry.first < source) edges.emplace_back(entry.first, source);
            }
        }
        std::sort(edges.begin(), edges.end(), [&](const auto &left, const auto &right) {
            auto score = [&](const auto &edge) {
                const Interval &a = intervals[indexByTemp.at(edge.first)];
                const Interval &b = intervals[indexByTemp.at(edge.second)];
                return std::max(a.priority(), b.priority());
            };
            return score(left) > score(right);
        });

        for (const auto &[leftTemp, rightTemp] : edges) {
            auto leftHome = homes.find(leftTemp);
            auto rightHome = homes.find(rightTemp);
            if (leftHome == homes.end() || rightHome == homes.end() ||
                leftHome->second == rightHome->second) {
                continue;
            }
            std::size_t leftIndex = indexByTemp.at(leftTemp);
            std::size_t rightIndex = indexByTemp.at(rightTemp);
            if (intervalsOverlap(intervals[leftIndex], intervals[rightIndex])) {
                continue;
            }
            std::size_t moving = leftIndex;
            int target = rightHome->second;
            if (intervals[leftIndex].priority() > intervals[rightIndex].priority()) {
                moving = rightIndex;
                target = leftHome->second;
            }
            if (!conflictsAt(moving, target)) {
                homes[intervals[moving].temp] = target;
            }
        }
    };
    coalesceBank(gprIntervals, {0, 1, 2, 3, 4, 5, 6, 7, 8},
                 result.tempToRegister);
    coalesceBank(floatIntervals,
                 {0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23,
                  24, 25, 26, 27, 28, 29},
                 result.tempToFloatRegister);

    // Build a graph-colored alternative from the same segment interference
    // relation.  The old linear scan remains the reference assignment; this
    // candidate is adopted only when it colors at least as many values and
    // does not worsen MOVE/PHI affinity.  That makes the stronger allocator
    // self-guarding while still allowing values separated by lifetime holes
    // to share a register even when their envelopes overlap.
    auto graphColorBank = [&](const std::vector<std::size_t> &bank,
                              const std::vector<int> &callerSaved,
                              const std::vector<int> &calleeSaved,
                              std::unordered_map<int, int> &homes) {
        if (bank.empty() || bank.size() > 2500) return;

        std::unordered_map<int, std::size_t> indexByTemp;
        for (std::size_t i = 0; i < bank.size(); ++i) {
            indexByTemp[intervals[bank[i]].temp] = i;
        }
        std::vector<std::vector<std::size_t>> neighbors(bank.size());
        for (std::size_t i = 0; i < bank.size(); ++i) {
            for (std::size_t j = i + 1; j < bank.size(); ++j) {
                if (!intervalsOverlap(intervals[bank[i]], intervals[bank[j]])) {
                    continue;
                }
                neighbors[i].push_back(j);
                neighbors[j].push_back(i);
            }
        }

        std::vector<std::size_t> order;
        std::vector<bool> done(bank.size(), false);
        std::vector<int> colors(bank.size(), -1);
        std::unordered_set<int> callerSavedSet(callerSaved.begin(),
                                               callerSaved.end());
        std::unordered_map<int, int> candidateHomes;

        auto canUse = [&](std::size_t node, int reg) {
            const Interval &interval = intervals[bank[node]];
            if (registerUnavailable(interval, reg, callerSavedSet)) {
                return false;
            }
            for (std::size_t neighbor : neighbors[node]) {
                if (colors[neighbor] == reg) return false;
            }
            return true;
        };

        auto addChoice = [](std::vector<int> &choices,
                           std::unordered_set<int> &seen, int reg) {
            if (seen.insert(reg).second) choices.push_back(reg);
        };

        while (order.size() < bank.size()) {
            std::size_t selected = bank.size();
            long long bestPriority = std::numeric_limits<long long>::min();
            std::size_t bestDegree = 0;
            for (std::size_t node = 0; node < bank.size(); ++node) {
                if (done[node]) continue;
                const Interval &interval = intervals[bank[node]];
                std::unordered_set<int> saturation;
                for (std::size_t neighbor : neighbors[node]) {
                    if (colors[neighbor] >= 0) saturation.insert(colors[neighbor]);
                }
                // Priority dominates, while saturation/degree break ties so
                // dense loop-carried values get a color before short copies.
                long long score = interval.priority() * 1024LL +
                                  static_cast<long long>(saturation.size()) * 32LL;
                if (selected == bank.size() || score > bestPriority ||
                    (score == bestPriority && neighbors[node].size() > bestDegree)) {
                    selected = node;
                    bestPriority = score;
                    bestDegree = neighbors[node].size();
                }
            }
            if (selected == bank.size()) break;

            const Interval &current = intervals[bank[selected]];
            std::vector<int> choices;
            std::unordered_set<int> seen;
            auto oldHome = homes.find(current.temp);
            if (oldHome != homes.end()) addChoice(choices, seen, oldHome->second);
            auto preferred = affinities.find(current.temp);
            if (preferred != affinities.end()) {
                for (int source : preferred->second) {
                    auto sourceHome = homes.find(source);
                    if (sourceHome != homes.end()) {
                        addChoice(choices, seen, sourceHome->second);
                    }
                    auto candidateSourceHome = candidateHomes.find(source);
                    if (candidateSourceHome != candidateHomes.end()) {
                        addChoice(choices, seen, candidateSourceHome->second);
                    }
                }
            }
            if (!current.liveAcrossCall) {
                for (int reg : callerSaved) {
                    if (current.callArgument && reg >= 0 && reg <= 7) {
                        continue;
                    }
                    addChoice(choices, seen, reg);
                }
            }
            for (int reg : calleeSaved) addChoice(choices, seen, reg);

            for (int reg : choices) {
                if (!canUse(selected, reg)) continue;
                colors[selected] = reg;
                candidateHomes[current.temp] = reg;
                break;
            }
            done[selected] = true;
            order.push_back(selected);
        }

        auto affinityCost = [&](const std::unordered_map<int, int> &mapping) {
            long long cost = 0;
            for (const auto &entry : affinities) {
                auto left = mapping.find(entry.first);
                if (left == mapping.end()) continue;
                for (int source : entry.second) {
                    if (entry.first >= source) continue;
                    auto right = mapping.find(source);
                    if (right == mapping.end()) {
                        cost += intervals[indexByTemp.at(entry.first)].priority();
                    } else if (left->second != right->second) {
                        cost += std::min(
                            intervals[indexByTemp.at(entry.first)].priority(),
                            intervals[indexByTemp.at(source)].priority());
                    }
                }
            }
            return cost;
        };

        auto calleeRegisterCount = [&](const std::unordered_map<int, int> &mapping) {
            std::unordered_set<int> used;
            for (const auto &entry : mapping) {
                if (std::find(calleeSaved.begin(), calleeSaved.end(),
                              entry.second) != calleeSaved.end()) {
                    used.insert(entry.second);
                }
            }
            return used.size();
        };

        std::size_t baselineSpills = bank.size() - homes.size();
        std::size_t candidateSpills = bank.size() - candidateHomes.size();
        long long baselineMoves = affinityCost(homes);
        long long candidateMoves = affinityCost(candidateHomes);
        std::size_t baselineCallee = calleeRegisterCount(homes);
        std::size_t candidateCallee = calleeRegisterCount(candidateHomes);
        if (candidateSpills < baselineSpills ||
            (candidateSpills == baselineSpills && candidateMoves <= baselineMoves &&
             candidateCallee <= baselineCallee)) {
            homes = std::move(candidateHomes);
        }
    };
    graphColorBank(gprIntervals, {0, 1, 2, 3, 4, 5, 6, 7, 8},
                   {19, 20, 21, 22, 23, 24, 25, 26, 27, 28},
                   result.tempToRegister);
    graphColorBank(floatIntervals,
                   {0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23,
                    24, 25, 26, 27, 28, 29},
                   {8, 9, 10, 11, 12, 13, 14, 15},
                   result.tempToFloatRegister);

    // Color stack-resident ranges as well.  The emitter keeps one fixed home
    // per SSA temp, so sharing a slot is safe precisely when no pair of their
    // live segments overlaps.  A simple first-fit coloring is sufficient here
    // because the number of physical spill slots is normally much larger
    // than the register bank; sorting by first segment keeps hot short ranges
    // from needlessly extending a color's occupancy.
    std::vector<std::size_t> spilled;
    for (std::size_t index = 0; index < intervals.size(); ++index) {
        const Interval &interval = intervals[index];
        if (interval.type == quad::QuadType::FLOAT) {
            if (result.tempToFloatRegister.count(interval.temp) == 0) {
                spilled.push_back(index);
            }
        } else if (result.tempToRegister.count(interval.temp) == 0) {
            spilled.push_back(index);
        }
    }
    std::sort(spilled.begin(), spilled.end(), [&](std::size_t left,
                                                   std::size_t right) {
        int ls = intervals[left].segments.empty()
                     ? intervals[left].start
                     : intervals[left].segments.front().start;
        int rs = intervals[right].segments.empty()
                     ? intervals[right].start
                     : intervals[right].segments.front().start;
        if (ls != rs) return ls < rs;
        return intervals[left].priority() > intervals[right].priority();
    });
    std::vector<std::vector<std::size_t>> slotUsers;
    for (std::size_t index : spilled) {
        int selected = -1;
        for (std::size_t color = 0; color < slotUsers.size(); ++color) {
            bool conflict = false;
            for (std::size_t other : slotUsers[color]) {
                if (intervalsOverlap(intervals[index], intervals[other])) {
                    conflict = true;
                    break;
                }
            }
            if (!conflict) {
                selected = static_cast<int>(color);
                break;
            }
        }
        if (selected < 0) {
            selected = static_cast<int>(slotUsers.size());
            slotUsers.emplace_back();
        }
        slotUsers[static_cast<std::size_t>(selected)].push_back(index);
        result.spillSlotColors[intervals[index].temp] = selected;
    }
    result.spillSlotCount = slotUsers.size();

    std::set<int> usedCalleeSaved;
    for (const auto &entry : result.tempToRegister) {
        if (entry.second >= 19 && entry.second <= 28) {
            usedCalleeSaved.insert(entry.second);
        }
    }
    result.usedCalleeSavedRegisters.assign(usedCalleeSaved.begin(),
                                           usedCalleeSaved.end());
    std::set<int> usedCalleeSavedFloat;
    for (const auto &entry : result.tempToFloatRegister) {
        if (entry.second >= 8 && entry.second <= 15) {
            usedCalleeSavedFloat.insert(entry.second);
        }
    }
    result.usedCalleeSavedFloatRegisters.assign(usedCalleeSavedFloat.begin(),
                                                usedCalleeSavedFloat.end());
    // Evicted intervals are also spills even though they were tentatively
    // assigned earlier in the scan.
    result.spilledIntervalCount = result.intervalCount -
                                  result.tempToRegister.size() -
                                  result.tempToFloatRegister.size();
    return result;
}

} // namespace backend
