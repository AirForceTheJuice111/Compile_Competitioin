#include "schedule.hh"

#include "advDFG.hh"
#include "instrSelection.hh"

#include <algorithm>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace instr {

static tree::Temp *newTemp(const quad::QuadFuncDecl *func) {
    auto *mutableFunc = const_cast<quad::QuadFuncDecl*>(func);
    ++mutableFunc->last_temp_num;
    return new tree::Temp(mutableFunc->last_temp_num);
}

static tree::Temp *termTemp(const quad::QuadTerm *term) {
    if (term == nullptr || term->kind != quad::QuadTermKind::TEMP) {
        return nullptr;
    }
    auto *quadTemp = const_cast<quad::QuadTerm*>(term)->get_temp();
    return quadTemp == nullptr ? nullptr : quadTemp->temp;
}

static int termConst(const quad::QuadTerm *term) {
    return const_cast<quad::QuadTerm*>(term)->get_const();
}

static void emitLoadConst(ScheduleFunc &func, tree::Temp *dst, int value) {
    uint32_t bits = static_cast<uint32_t>(value);
    uint32_t low = bits & 0xffffu;
    uint32_t high = (bits >> 16) & 0xffffu;
    func.addLinearizedInstruction(AssemInstr::Oper(
        "movw `d0, #" + std::to_string(low),
        {dst},
        {},
        AssemTargets()
    ));
    if (high != 0) {
        func.addLinearizedInstruction(AssemInstr::Oper(
            "movt `d0, #" + std::to_string(high),
            {dst},
            {},
            AssemTargets()
        ));
    }
}

static tree::Temp *materializeTerm(
    ScheduleFunc &func,
    const quad::QuadFuncDecl *quadFunc,
    const quad::QuadTerm *term
) {
    if (term == nullptr) {
        return nullptr;
    }
    if (term->kind == quad::QuadTermKind::TEMP) {
        return termTemp(term);
    }
    auto *tmp = newTemp(quadFunc);
    if (term->kind == quad::QuadTermKind::CONST) {
        emitLoadConst(func, tmp, termConst(term));
    } else if (term->kind == quad::QuadTermKind::NAME) {
        func.addLinearizedInstruction(AssemInstr::Oper(
            "adr `d0, " + const_cast<quad::QuadTerm*>(term)->get_name(),
            {tmp},
            {},
            AssemTargets()
        ));
    }
    return tmp;
}

static bool sameLabel(const tree::Label *left, const tree::Label *right) {
    return left != nullptr && right != nullptr && left->num == right->num;
}

static bool isExitCallStmt(const quad::QuadStm *stm) {
    if (stm == nullptr) {
        return false;
    }
    if (stm->kind == quad::QuadKind::EXTCALL) {
        auto *ext = dynamic_cast<const quad::QuadExtCall*>(stm);
        return ext != nullptr && ext->extfun == "exit";
    }
    if (stm->kind == quad::QuadKind::MOVE_EXTCALL) {
        auto *moveExt = dynamic_cast<const quad::QuadMoveExtCall*>(stm);
        return moveExt != nullptr && moveExt->extcall != nullptr && moveExt->extcall->extfun == "exit";
    }
    return false;
}

static std::string branchMnemonic(const std::string &relop) {
    if (relop == ">") {
        return "bgt";
    }
    if (relop == ">=") {
        return "bge";
    }
    if (relop == "<") {
        return "blt";
    }
    if (relop == "<=") {
        return "ble";
    }
    if (relop == "==" || relop == "=") {
        return "beq";
    }
    if (relop == "!=") {
        return "bne";
    }
    return "b";
}

static void appendPhiCopies(
    ScheduleFunc &func,
    const preScheduleBlock *target,
    const tree::Label *fromLabel
) {
    if (target == nullptr || fromLabel == nullptr) {
        return;
    }
    for (auto *phi : target->phiFunctions) {
        if (phi == nullptr || phi->temp_exp == nullptr || phi->args == nullptr) {
            continue;
        }
        for (const auto &arg : *phi->args) {
            if (arg.first != nullptr && sameLabel(arg.second, fromLabel)) { // this arg has a copy from the block we're coming from, so we need to emit a move for it
                func.addLinearizedInstruction(AssemInstr::Move(
                    "mov `d0, `s0", // this means "move the value from the source temporary to the destination temporary"
                    {phi->temp_exp->temp},
                    {arg.first}
                ));
                break;
            }
        }
    }
}

static bool hasPhiCopies(const preScheduleBlock *target, const tree::Label *fromLabel) {
    if (target == nullptr || fromLabel == nullptr) {
        return false;
    }
    for (auto *phi : target->phiFunctions) {
        if (phi == nullptr || phi->args == nullptr) {
            continue;
        }
        for (const auto &arg : *phi->args) {
            if (arg.first != nullptr && sameLabel(arg.second, fromLabel)) {
                return true;
            }
        }
    }
    return false;
}

static void appendEpilogue(ScheduleFunc &func) {
    func.addLinearizedInstruction(AssemInstr::Oper("sub sp, fp, #36", {}, {}, AssemTargets()));
    func.addLinearizedInstruction(AssemInstr::Oper("add sp, sp, #4", {}, {}, AssemTargets()));
    func.addLinearizedInstruction(AssemInstr::Oper("pop {r4-r10, fp, lr}", {}, {}, AssemTargets()));
    func.addLinearizedInstruction(AssemInstr::Oper("bx lr", {}, {}, AssemTargets()));
}

static void appendReturn(
    ScheduleFunc &func,
    const quad::QuadFuncDecl *quadFunc,
    const quad::QuadReturn *ret
) {
    if (ret != nullptr && ret->exp != nullptr) {
        auto *value = materializeTerm(func, quadFunc, ret->exp);
        if (value != nullptr) {
            func.addLinearizedInstruction(AssemInstr::Oper("mov r0, `s0", {}, {value}, AssemTargets()));
        }
    }
    appendEpilogue(func);
}

ScheduleProg *scheduleProg(preScheduleProg *preScheduleProgram) {
    if (preScheduleProgram == nullptr) {
        return nullptr;
    }

    auto *out = new ScheduleProg(preScheduleProgram->quadProgram);

    for (auto *preFunc : preScheduleProgram->funcSchedules) {
        if (preFunc == nullptr || preFunc->quadFunc == nullptr) {
            continue;
        }

        auto *func = new ScheduleFunc(preFunc->quadFunc);
        out->addFunc(func);

        std::unordered_map<int, preScheduleBlock*> labelToBlock;
        for (auto *block : preFunc->blockSchedules) {
            if (block != nullptr && block->entryLabel != nullptr) {
                labelToBlock[block->entryLabel->num] = block;
            }
        }

        std::unordered_set<int> visited;
        auto *entryBlock = preFunc->blockSchedules.empty() ? nullptr : preFunc->blockSchedules.front();
        std::function<void(preScheduleBlock*)> appendBlock = [&](preScheduleBlock *block) {
            if (block == nullptr || block->entryLabel == nullptr ||
                visited.find(block->entryLabel->num) != visited.end()) {
                return;
            }

            visited.insert(block->entryLabel->num);
            if (block == entryBlock) {
                func->addLinearizedInstruction(AssemInstr::Oper("push {r4-r10, fp, lr}", {}, {}, AssemTargets()));
                func->addLinearizedInstruction(AssemInstr::Oper("sub sp, sp, #4", {}, {}, AssemTargets()));
                func->addLinearizedInstruction(AssemInstr::Oper("add fp, sp, #36", {}, {}, AssemTargets()));

                if (preFunc->quadFunc->params != nullptr) {
                    std::vector<tree::Temp*> registerParams;
                    for (auto *param : *preFunc->quadFunc->params) {
                        if (registerParams.size() >= 4) {
                            break;
                        }
                        registerParams.push_back(param);
                    }
                    for (size_t i = 0; i < registerParams.size(); ++i) {
                        if (registerParams[i] != nullptr) {
                            func->addLinearizedInstruction(AssemInstr::Oper(
                                "push {r" + std::to_string(i) + "}",
                                {},
                                {},
                                AssemTargets()
                            ));
                        }
                    }
                    for (int i = static_cast<int>(registerParams.size()) - 1; i >= 0; --i) {
                        if (registerParams[i] != nullptr) {
                            func->addLinearizedInstruction(AssemInstr::Oper(
                                "pop {`d0}",
                                {registerParams[i]},
                                {},
                                AssemTargets()
                            ));
                        }
                    }
                }
            }
            func->addLinearizedInstruction(AssemInstr::Label(block->entryLabel->str() + ":", block->entryLabel));
            func->linearizedInstructions.extend(block->selectedInstructions); // because selectedInstructions is already in the correct order for this block, we can just extend it directly

            auto *last = block->lastInstruction;
            if (last == nullptr || isExitCallStmt(last)) {
                return;
            }

            if (last->kind == quad::QuadKind::RETURN) {
                appendReturn(*func, preFunc->quadFunc, dynamic_cast<const quad::QuadReturn*>(last));
                return;
            }

            if (last->kind == quad::QuadKind::JUMP) {
                auto *jump = dynamic_cast<const quad::QuadJump*>(last);
                auto *target = jump == nullptr ? nullptr : labelToBlock[jump->label->num];
                appendPhiCopies(*func, target, block->entryLabel);
                if (target != nullptr && visited.find(target->entryLabel->num) == visited.end()) {
                    appendBlock(target);
                } else if (jump != nullptr) {
                    func->addLinearizedInstruction(AssemInstr::Oper("b `j0", {}, {}, AssemTargets({jump->label})));
                }
                return;
            }

            if (last->kind == quad::QuadKind::CJUMP) {
                auto *cjump = dynamic_cast<const quad::QuadCJump*>(last);
                if (cjump == nullptr) {
                    return;
                }
                auto *left = materializeTerm(*func, preFunc->quadFunc, cjump->left);
                auto *right = materializeTerm(*func, preFunc->quadFunc, cjump->right);
                func->addLinearizedInstruction(AssemInstr::Oper("cmp `s0, `s1", {}, {left, right}, AssemTargets()));
                auto *falseBlock = cjump->f == nullptr ? nullptr : labelToBlock[cjump->f->num];
                auto *trueBlock = cjump->t == nullptr ? nullptr : labelToBlock[cjump->t->num];
                bool trueNeedsPhi = hasPhiCopies(trueBlock, block->entryLabel);

                if (trueNeedsPhi) { // if the true block needs phi copies, we need to emit the branch to the false block first, so that the phi copies for the false block can jump to the edge label we emit later
                    auto *edgeLabel = new tree::Label(++const_cast<quad::QuadFuncDecl*>(preFunc->quadFunc)->last_label_num);
                    func->addLinearizedInstruction(AssemInstr::Oper(
                        branchMnemonic(cjump->relop) + " `j0",
                        {},
                        {},
                        AssemTargets({edgeLabel})
                    ));
                    appendPhiCopies(*func, falseBlock, block->entryLabel);
                    if (falseBlock != nullptr && visited.find(falseBlock->entryLabel->num) == visited.end()) {
                        appendBlock(falseBlock);
                    } else if (cjump->f != nullptr) {
                        func->addLinearizedInstruction(AssemInstr::Oper("b `j0", {}, {}, AssemTargets({cjump->f})));
                    }
                    func->addLinearizedInstruction(AssemInstr::Label(edgeLabel->str() + ":", edgeLabel)); // emit the edge label right before the true block, so that the phi copies for the true block can jump to it
                    appendPhiCopies(*func, trueBlock, block->entryLabel);
                    if (trueBlock != nullptr && visited.find(trueBlock->entryLabel->num) == visited.end()) {
                        appendBlock(trueBlock);
                    } else if (cjump->t != nullptr) {
                        func->addLinearizedInstruction(AssemInstr::Oper("b `j0", {}, {}, AssemTargets({cjump->t})));
                    }
                    return;
                }

                func->addLinearizedInstruction(AssemInstr::Oper(
                    branchMnemonic(cjump->relop) + " `j0",
                    {},
                    {},
                    AssemTargets({cjump->t})
                ));
                appendPhiCopies(*func, falseBlock, block->entryLabel);

                if (falseBlock != nullptr && visited.find(falseBlock->entryLabel->num) == visited.end()) {
                    appendBlock(falseBlock);
                } else if (cjump->f != nullptr) {
                    func->addLinearizedInstruction(AssemInstr::Oper("b `j0", {}, {}, AssemTargets({cjump->f})));
                }

                if (trueBlock != nullptr && visited.find(trueBlock->entryLabel->num) == visited.end()) {
                    appendBlock(trueBlock);
                }
            }
        };

        for (auto *block : preFunc->blockSchedules) {
            appendBlock(block);
        }
    }

    return out;
}

} // namespace instr
