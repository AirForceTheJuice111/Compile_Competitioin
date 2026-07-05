#include "instrSelection.hh"

#include <cstdint>
#include <set>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace instr {

static std::unordered_map<int, tree::Temp*> *activeConstCache = nullptr;

static tree::Temp *newTemp(int &nextTempNum) {
    return new tree::Temp(nextTempNum++);
}

static tree::Temp *termTemp(const quad::QuadTerm *term) {
    if (term == nullptr || term->kind != quad::QuadTermKind::TEMP) {
        return nullptr;
    }
    auto *mutableTerm = const_cast<quad::QuadTerm*>(term);
    auto *quadTemp = mutableTerm->get_temp();
    return quadTemp == nullptr ? nullptr : quadTemp->temp;
}

static int termConst(const quad::QuadTerm *term) {
    return const_cast<quad::QuadTerm*>(term)->get_const();
}

static std::string termName(const quad::QuadTerm *term) {
    return const_cast<quad::QuadTerm*>(term)->get_name();
}

static bool isConstTerm(const quad::QuadTerm *term, int *value = nullptr) {
    if (term == nullptr || term->kind != quad::QuadTermKind::CONST) {
        return false;
    }
    if (value != nullptr) {
        *value = termConst(term);
    }
    return true;
}

static bool isTempTerm(const quad::QuadTerm *term, tree::Temp **temp = nullptr) {
    if (term == nullptr || term->kind != quad::QuadTermKind::TEMP) {
        return false;
    }
    auto *out = termTemp(term);
    if (out == nullptr) {
        return false;
    }
    if (temp != nullptr) {
        *temp = out;
    }
    return true;
}

static bool sameTemp(const tree::Temp *left, const tree::Temp *right) {
    return left != nullptr && right != nullptr && left->num == right->num;
}

static bool isArmMemoryImmediate(int offset) {
    return offset >= 0 && offset <= 4095;
}

static void emitLoadConst(preScheduleBlock &schedBlock, tree::Temp *dst, int value) {
    uint32_t bits = static_cast<uint32_t>(value);
    uint32_t low = bits & 0xffffu;
    uint32_t high = (bits >> 16) & 0xffffu;
    schedBlock.addSelectedInstruction(AssemInstr::Oper(
        "movw `d0, #" + std::to_string(low),
        {dst},
        {},
        AssemTargets()
    ));
    if (high != 0) {
        schedBlock.addSelectedInstruction(AssemInstr::Oper(
            "movt `d0, #" + std::to_string(high),
            {dst},
            {},
            AssemTargets()
        ));
    }
}

static tree::Temp *materializeTerm(
    const quad::QuadTerm *term,
    preScheduleBlock &schedBlock,
    int &nextTempNum,
    bool useConstCache = true
) {
    if (term == nullptr) {
        return nullptr;
    }
    if (term->kind == quad::QuadTermKind::TEMP) {
        return termTemp(term);
    }

    auto *tmp = newTemp(nextTempNum);
    if (term->kind == quad::QuadTermKind::CONST) {
        int value = termConst(term);
        if (useConstCache && activeConstCache != nullptr) {
            auto found = activeConstCache->find(value);
            if (found != activeConstCache->end()) {
                return found->second;
            }
        }
        emitLoadConst(schedBlock, tmp, value);
        if (useConstCache && activeConstCache != nullptr) {
            (*activeConstCache)[value] = tmp;
        }
    } else if (term->kind == quad::QuadTermKind::NAME) {
        schedBlock.addSelectedInstruction(AssemInstr::Oper(
            "adr `d0, " + termName(term),
            {tmp},
            {},
            AssemTargets()
        ));
    }
    return tmp;
}

static void emitMove(preScheduleBlock &schedBlock, tree::Temp *dst, tree::Temp *src) {
    if (dst == nullptr || src == nullptr || dst->num == src->num) {
        return;
    }
    schedBlock.addSelectedInstruction(AssemInstr::Move("mov `d0, `s0", {dst}, {src}));
}

static void emitMoveTerm(
    preScheduleBlock &schedBlock,
    tree::Temp *dst,
    const quad::QuadTerm *src,
    int &nextTempNum
) {
    if (dst == nullptr || src == nullptr) {
        return;
    }
    int value = 0;
    if (isConstTerm(src, &value) && value >= 0 && value <= 255) {
        schedBlock.addSelectedInstruction(AssemInstr::Move(
            "mov `d0, #" + std::to_string(value),
            {dst},
            {}
        ));
        return;
    }
    auto *srcTemp = materializeTerm(src, schedBlock, nextTempNum);
    emitMove(schedBlock, dst, srcTemp);
}

static void emitMoveToRegister(preScheduleBlock &schedBlock, const std::string &reg, tree::Temp *src) {
    if (src == nullptr) {
        return;
    }
    schedBlock.addSelectedInstruction(AssemInstr::Oper("mov " + reg + ", `s0", {}, {src}, AssemTargets()));
}

static void emitMoveFromRegister(preScheduleBlock &schedBlock, tree::Temp *dst, const std::string &reg) {
    if (dst == nullptr) {
        return;
    }
    schedBlock.addSelectedInstruction(AssemInstr::Oper("mov `d0, " + reg, {dst}, {}, AssemTargets()));
}

static void emitArgs(
    preScheduleBlock &schedBlock,
    const std::vector<quad::QuadTerm*> *args,
    int &nextTempNum,
    int firstReg
) {
    if (args == nullptr) {
        return;
    }
    std::vector<tree::Temp*> materializedArgs;
    for (auto *arg : *args) {
        materializedArgs.push_back(materializeTerm(arg, schedBlock, nextTempNum));
    }

    for (auto *src : materializedArgs) {
        if (src == nullptr) continue;
        schedBlock.addSelectedInstruction(AssemInstr::Oper("push {`s0}", {}, {src}, AssemTargets()));
    }
    for (int i = static_cast<int>(materializedArgs.size()) - 1; i >= 0; --i) {
        schedBlock.addSelectedInstruction(AssemInstr::Oper("pop {r" + std::to_string(firstReg + i) + "}", {}, {}, AssemTargets()));
    }
}

static void selectCall(
    preScheduleBlock &schedBlock,
    const quad::QuadCall *call,
    tree::Temp *dst,
    int &nextTempNum
) {
    if (call == nullptr) {
        return;
    }

    if (call->obj_term != nullptr) {
        auto *target = materializeTerm(call->obj_term, schedBlock, nextTempNum);
        if (target != nullptr) {
            schedBlock.addSelectedInstruction(AssemInstr::Oper("push {`s0}", {}, {target}, AssemTargets()));
        }
        emitArgs(schedBlock, call->args, nextTempNum, 0);
        if (target != nullptr) {
            schedBlock.addSelectedInstruction(AssemInstr::Oper("pop {ip}", {}, {}, AssemTargets()));
            schedBlock.addSelectedInstruction(AssemInstr::Oper("blx ip", {}, {}, AssemTargets()));
        }
    } else {
        emitArgs(schedBlock, call->args, nextTempNum, 0);
        schedBlock.addSelectedInstruction(AssemInstr::Call("bl " + call->name, {}, {}));
    }

    if (dst != nullptr) {
        emitMoveFromRegister(schedBlock, dst, "r0");
    }
}

static void selectExtCall(
    preScheduleBlock &schedBlock,
    const quad::QuadExtCall *call,
    tree::Temp *dst,
    int &nextTempNum
) {
    if (call == nullptr) {
        return;
    }
    emitArgs(schedBlock, call->args, nextTempNum, 0);
    schedBlock.addSelectedInstruction(AssemInstr::ExtCall("bl " + call->extfun, {}, {}));
    if (dst != nullptr) {
        emitMoveFromRegister(schedBlock, dst, "r0");
    }
}

static void selectStatement(
    const quad::QuadStm *stm,
    preScheduleBlock &schedBlock,
    int &nextTempNum
) {
    if (stm == nullptr) {
        return;
    }

    switch (stm->kind) {
        case quad::QuadKind::MOVE: {
            auto *move = dynamic_cast<const quad::QuadMove*>(stm);
            emitMoveTerm(schedBlock, move->dst->temp, move->src, nextTempNum);
            break;
        }
        case quad::QuadKind::LOAD: {
            auto *load = dynamic_cast<const quad::QuadLoad*>(stm);
            auto *addr = materializeTerm(load->src, schedBlock, nextTempNum);
            schedBlock.addSelectedInstruction(AssemInstr::Oper("ldr `d0, [`s0]", {load->dst->temp}, {addr}, AssemTargets()));
            break;
        }
        case quad::QuadKind::STORE: {
            auto *store = dynamic_cast<const quad::QuadStore*>(stm);
            auto *src = materializeTerm(store->src, schedBlock, nextTempNum);
            auto *addr = materializeTerm(store->dst, schedBlock, nextTempNum);
            schedBlock.addSelectedInstruction(AssemInstr::Oper("str `s0, [`s1]", {}, {src, addr}, AssemTargets()));
            break;
        }
        case quad::QuadKind::MOVE_BINOP: {
            auto *binop = dynamic_cast<const quad::QuadMoveBinop*>(stm);
            std::string op = "add";
            if (binop->binop == "-") {
                op = "sub";
            } else if (binop->binop == "*") {
                op = "mul";
            } else if (binop->binop == "/") {
                op = "sdiv";
            }
            int rightConst = 0;
            if ((op == "add" || op == "sub") && isConstTerm(binop->right, &rightConst) &&
                rightConst >= 0 && rightConst <= 4095) {
                auto *left = materializeTerm(binop->left, schedBlock, nextTempNum);
                schedBlock.addSelectedInstruction(AssemInstr::Oper(
                    op + " `d0, `s0, #" + std::to_string(rightConst),
                    {binop->dst->temp},
                    {left},
                    AssemTargets()
                ));
                break;
            }
            auto *left = materializeTerm(binop->left, schedBlock, nextTempNum, false);
            auto *right = materializeTerm(binop->right, schedBlock, nextTempNum, false);
            schedBlock.addSelectedInstruction(AssemInstr::Oper(op + " `d0, `s0, `s1", {binop->dst->temp}, {left, right}, AssemTargets()));
            break;
        }
        case quad::QuadKind::PTR_CALC: {
            auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc*>(stm);
            auto *base = materializeTerm(ptrCalc->ptr, schedBlock, nextTempNum);
            auto *dst = termTemp(ptrCalc->dst);
            int offsetConst = 0;
            if (isConstTerm(ptrCalc->offset, &offsetConst) && offsetConst == 0) {
                emitMove(schedBlock, dst, base);
                break;
            }
            if (isConstTerm(ptrCalc->offset, &offsetConst) && offsetConst > 0 && offsetConst <= 4095) {
                schedBlock.addSelectedInstruction(AssemInstr::Oper(
                    "add `d0, `s0, #" + std::to_string(offsetConst),
                    {dst},
                    {base},
                    AssemTargets()
                ));
                break;
            }
            auto *offset = materializeTerm(ptrCalc->offset, schedBlock, nextTempNum);
            schedBlock.addSelectedInstruction(AssemInstr::Oper("add `d0, `s0, `s1", {dst}, {base, offset}, AssemTargets()));
            break;
        }
        case quad::QuadKind::CALL: {
            selectCall(schedBlock, dynamic_cast<const quad::QuadCall*>(stm), nullptr, nextTempNum);
            break;
        }
        case quad::QuadKind::MOVE_CALL: {
            auto *moveCall = dynamic_cast<const quad::QuadMoveCall*>(stm);
            selectCall(schedBlock, moveCall->call, moveCall->dst->temp, nextTempNum);
            break;
        }
        case quad::QuadKind::EXTCALL: {
            selectExtCall(schedBlock, dynamic_cast<const quad::QuadExtCall*>(stm), nullptr, nextTempNum);
            break;
        }
        case quad::QuadKind::MOVE_EXTCALL: {
            auto *moveExtCall = dynamic_cast<const quad::QuadMoveExtCall*>(stm);
            selectExtCall(schedBlock, moveExtCall->extcall, moveExtCall->dst->temp, nextTempNum);
            break;
        }
        default:
            break;
    }
}

static bool selectFoldedMemoryAccess( // fold ptr offset + load/store into single load/store if possible
    const quad::QuadPtrCalc *ptrCalc,
    const quad::QuadStm *next,
    preScheduleBlock &schedBlock,
    int &nextTempNum
) {
    if (ptrCalc == nullptr || next == nullptr) {
        return false;
    }

    auto *ptrDst = termTemp(ptrCalc->dst);
    auto *base = termTemp(ptrCalc->ptr);
    if (ptrDst == nullptr || base == nullptr) {
        return false;
    }

    int offsetConst = 0;
    bool constOffset = isConstTerm(ptrCalc->offset, &offsetConst);
    tree::Temp *offsetTemp = nullptr;
    bool tempOffset = isTempTerm(ptrCalc->offset, &offsetTemp);

    if (!constOffset && !tempOffset) {
        return false;
    }

    if (next->kind == quad::QuadKind::LOAD) {
        auto *load = dynamic_cast<const quad::QuadLoad*>(next);
        tree::Temp *addr = nullptr;
        if (load == nullptr || !isTempTerm(load->src, &addr) || !sameTemp(addr, ptrDst)) {
            return false;
        }
        if (constOffset && isArmMemoryImmediate(offsetConst)) {
            if (offsetConst == 0) {
                schedBlock.addSelectedInstruction(AssemInstr::Oper("ldr `d0, [`s0]", {load->dst->temp}, {base}, AssemTargets()));
            } else {
                schedBlock.addSelectedInstruction(AssemInstr::Oper(
                    "ldr `d0, [`s0, #" + std::to_string(offsetConst) + "]",
                    {load->dst->temp},
                    {base},
                    AssemTargets()
                ));
            }
        } else if (tempOffset) {
            schedBlock.addSelectedInstruction(AssemInstr::Oper("ldr `d0, [`s0, `s1]", {load->dst->temp}, {base, offsetTemp}, AssemTargets()));
        } else {
            return false;
        }
        return true;
    }

    if (next->kind == quad::QuadKind::STORE) {
        auto *store = dynamic_cast<const quad::QuadStore*>(next);
        tree::Temp *addr = nullptr;
        if (store == nullptr || !isTempTerm(store->dst, &addr) || !sameTemp(addr, ptrDst)) {
            return false;
        }

        if (constOffset && offsetConst == 0 && store->src != nullptr &&
            store->src->kind == quad::QuadTermKind::TEMP) {
            return false;
        }
        if (constOffset && !isArmMemoryImmediate(offsetConst)) {
            return false;
        }

        auto *src = materializeTerm(store->src, schedBlock, nextTempNum);
        if (constOffset) {
            if (offsetConst == 0) {
                schedBlock.addSelectedInstruction(AssemInstr::Oper("str `s0, [`s1]", {}, {src, base}, AssemTargets()));
            } else {
                schedBlock.addSelectedInstruction(AssemInstr::Oper(
                    "str `s0, [`s1, #" + std::to_string(offsetConst) + "]",
                    {},
                    {src, base},
                    AssemTargets()
                ));
            }
        } else {
            schedBlock.addSelectedInstruction(AssemInstr::Oper("str `s0, [`s1, `s2]", {}, {src, base, offsetTemp}, AssemTargets()));
        }
        return true;
    }

    return false;
}

static bool useSetContainsTemp(const quad::QuadStm *stm, const tree::Temp *temp) {
    if (stm == nullptr || temp == nullptr || stm->use == nullptr) {
        return false;
    }
    for (auto *used : *stm->use) {
        if (sameTemp(used, temp)) {
            return true;
        }
    }
    return false;
}

static bool memoryAddressUsesTemp(const quad::QuadStm *stm, const tree::Temp *temp) {
    if (stm == nullptr || temp == nullptr) {
        return false;
    }
    if (stm->kind == quad::QuadKind::LOAD) {
        auto *load = dynamic_cast<const quad::QuadLoad*>(stm);
        tree::Temp *addr = nullptr;
        return load != nullptr && isTempTerm(load->src, &addr) && sameTemp(addr, temp);
    }
    if (stm->kind == quad::QuadKind::STORE) {
        auto *store = dynamic_cast<const quad::QuadStore*>(stm);
        tree::Temp *addr = nullptr;
        return store != nullptr && isTempTerm(store->dst, &addr) && sameTemp(addr, temp);
    }
    return false;
}

static bool isScheduledBySchedulePass(const quad::QuadStm *stm) {
    if (stm == nullptr) {
        return false;
    }
    return stm->kind == quad::QuadKind::JUMP ||
           stm->kind == quad::QuadKind::CJUMP ||
           stm->kind == quad::QuadKind::RETURN;
}

static bool allPredecessorsCovered(
    const advDFGNode *node,
    const std::unordered_set<const advDFGNode*> &covered
) {
    if (node == nullptr) {
        return false;
    }
    for (auto *pred : node->predecessors) {
        if (covered.find(pred) == covered.end()) {
            return false;
        }
    }
    return true;
}

static bool shouldFoldPtrCalcOnGraph(
    const quad::QuadPtrCalc *ptrCalc,
    const std::vector<advDFGNode*> &nodes
) {
    if (ptrCalc == nullptr) {
        return false;
    }

    int offsetConst = 0;
    tree::Temp *offsetTemp = nullptr;
    if (!isConstTerm(ptrCalc->offset, &offsetConst) &&
        !isTempTerm(ptrCalc->offset, &offsetTemp)) {
        return false;
    }

    auto *dst = termTemp(ptrCalc->dst);
    if (dst == nullptr) {
        return false;
    }

    int useCount = 0;
    bool onlyUseIsMemoryAddress = false;
    for (auto *node : nodes) {
        auto *stm = node == nullptr ? nullptr : node->quadStatement;
        if (!useSetContainsTemp(stm, dst)) {
            continue;
        }
        ++useCount;
        onlyUseIsMemoryAddress = memoryAddressUsesTemp(stm, dst);
        if (useCount > 1 || !onlyUseIsMemoryAddress) {
            return false;
        }
    }

    return useCount == 1 && onlyUseIsMemoryAddress;
}

static tree::Temp *memoryAddressTemp(const quad::QuadStm *stm) {
    tree::Temp *addr = nullptr;
    if (stm == nullptr) {
        return nullptr;
    }
    if (stm->kind == quad::QuadKind::LOAD) {
        auto *load = dynamic_cast<const quad::QuadLoad*>(stm);
        if (load != nullptr) {
            isTempTerm(load->src, &addr);
        }
    } else if (stm->kind == quad::QuadKind::STORE) {
        auto *store = dynamic_cast<const quad::QuadStore*>(stm);
        if (store != nullptr) {
            isTempTerm(store->dst, &addr);
        }
    }
    return addr;
}

// Main instruction selection for a block
void selectInstructionsForBlock(
    const advDFGblock &blockGraph,
    preScheduleBlock &schedBlock,
    int &nextTempNum
) {
    const auto& graph = blockGraph.graph;
    const auto& nodes = graph.getNodes();
    if (nodes.empty()) return;
    
    std::unordered_map<int, tree::Temp*> constCache;
    activeConstCache = &constCache;

    std::unordered_set<const advDFGNode*> covered;
    std::unordered_map<int, const quad::QuadPtrCalc*> deferredPtrCalc;
    covered.insert(nodes.front());

    bool changed = true;
    while (changed && covered.size() < nodes.size()) {
        changed = false;
        for (auto *node : nodes) {
            if (node == nullptr || covered.find(node) != covered.end()) continue;
            if (!allPredecessorsCovered(node, covered)) continue;
            

            auto *stm = node->quadStatement;
            if (stm == nullptr || node->type == NodeType::EntryLabel ||
                node->type == NodeType::ExitStatement ||
                (stm == schedBlock.lastInstruction && isScheduledBySchedulePass(stm))) {
                covered.insert(node);
                changed = true;
                continue;
            }

            if (stm->kind == quad::QuadKind::PTR_CALC) {
                auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc*>(stm);
                auto *dst = ptrCalc == nullptr ? nullptr : termTemp(ptrCalc->dst);
                if (dst != nullptr && shouldFoldPtrCalcOnGraph(ptrCalc, nodes)) {
                    deferredPtrCalc[dst->num] = ptrCalc;
                    covered.insert(node);
                    changed = true;
                    continue;
                }
            }

            auto *addr = memoryAddressTemp(stm);
            if (addr != nullptr) {
                auto found = deferredPtrCalc.find(addr->num);
                if (found != deferredPtrCalc.end()) {
                    if (selectFoldedMemoryAccess(found->second, stm, schedBlock, nextTempNum)) {
                        deferredPtrCalc.erase(found);
                        covered.insert(node);
                        changed = true;
                        continue;
                    }
                    selectStatement(found->second, schedBlock, nextTempNum);
                    deferredPtrCalc.erase(found);
                }
            }

            selectStatement(stm, schedBlock, nextTempNum);
            covered.insert(node);
            changed = true;
        }
    }

    // Fallback. Shouldnt be necessary if the graph is well formed, but just in case, select any remaining uncovered statements
    for (auto *node : nodes) {
        if (node == nullptr || covered.find(node) != covered.end()) continue;
        
        auto *stm = node->quadStatement;
        if (stm == nullptr || node->type == NodeType::ExitStatement ||
            (stm == schedBlock.lastInstruction && isScheduledBySchedulePass(stm))) {
            continue;
        }
        selectStatement(stm, schedBlock, nextTempNum);
    }

    activeConstCache = nullptr;
    return;
}

void runInstructionSelectionPass(
    const advDFGprog &graphProgram,
    preScheduleProg &preScheduleProgram
) {
    size_t funcCount = std::min(graphProgram.fungraph.size(), preScheduleProgram.funcSchedules.size());
    for (size_t funcIndex = 0; funcIndex < funcCount; ++funcIndex) {
        auto *funcGraph = graphProgram.fungraph[funcIndex];
        auto *funcSchedule = preScheduleProgram.funcSchedules[funcIndex];
        if (funcGraph == nullptr || funcSchedule == nullptr || funcSchedule->quadFunc == nullptr) {
            continue;
        }

        int nextTempNum = funcSchedule->quadFunc->last_temp_num + 1;
        size_t blockCount = std::min(funcGraph->blockgraph.size(), funcSchedule->blockSchedules.size());
        for (size_t blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
            auto *blockGraph = funcGraph->blockgraph[blockIndex];
            auto *blockSchedule = funcSchedule->blockSchedules[blockIndex];
            if (blockGraph == nullptr || blockSchedule == nullptr) {
                continue;
            }

            selectInstructionsForBlock(*blockGraph, *blockSchedule, nextTempNum);
        }

        auto *mutableFunc = const_cast<quad::QuadFuncDecl*>(funcSchedule->quadFunc);
        mutableFunc->last_temp_num = nextTempNum - 1;
    }
}

} // namespace instr
