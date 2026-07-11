#include "algebrasimp.hh"

#include <cstdint>
#include <cstring>
#include <limits>
#include <set>
#include <string>
#include <vector>

#include "quad.hh"
#include "temp.hh"

using namespace std;

namespace {

bool isConst(quad::QuadTerm *term) {
    return term != nullptr && term->kind == quad::QuadTermKind::CONST;
}

bool isTemp(quad::QuadTerm *term) {
    return term != nullptr && term->kind == quad::QuadTermKind::TEMP;
}

bool isIntTemp(quad::QuadTerm *term) {
    return isTemp(term) && term->get_temp() != nullptr &&
           term->get_temp()->type == quad::QuadType::INT;
}

bool sameIntTemp(quad::QuadTerm *left, quad::QuadTerm *right) {
    return isIntTemp(left) && isIntTemp(right) &&
           left->get_temp()->temp != nullptr && right->get_temp()->temp != nullptr &&
           left->get_temp()->temp->num == right->get_temp()->temp->num;
}

int constVal(quad::QuadTerm *term) {
    return term == nullptr ? 0 : term->get_const();
}

quad::QuadTerm *termConst(std::int32_t value) {
    return new quad::QuadTerm(static_cast<int>(value));
}

set<Temp *> *emptySet() {
    return new set<Temp *>();
}

std::uint32_t intBits(std::int32_t value) {
    return static_cast<std::uint32_t>(value);
}

std::int32_t bitsAsInt(std::uint32_t bits) {
    std::int32_t value = 0;
    static_assert(sizeof(value) == sizeof(bits), "SysY int must be 32 bits");
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

std::int32_t wrapAdd(std::int32_t left, std::int32_t right) {
    return bitsAsInt(intBits(left) + intBits(right));
}

std::int32_t wrapSub(std::int32_t left, std::int32_t right) {
    return bitsAsInt(intBits(left) - intBits(right));
}

std::int32_t wrapMul(std::int32_t left, std::int32_t right) {
    return bitsAsInt(intBits(left) * intBits(right));
}

std::int32_t arithmeticShiftRight(std::int32_t value, unsigned shift) {
    if (shift == 0) {
        return value;
    }
    std::uint32_t bits = intBits(value);
    std::uint32_t shifted = bits >> shift;
    if ((bits & 0x80000000u) != 0) {
        shifted |= (~std::uint32_t{0}) << (32u - shift);
    }
    return bitsAsInt(shifted);
}

bool termIsCompatibleWithInt(quad::QuadTerm *term) {
    return isConst(term) || isIntTemp(term);
}

bool isComparison(const string &op) {
    return op == "==" || op == "!=" || op == "<" || op == "<=" ||
           op == ">" || op == ">=";
}

bool intBinopIsProvable(const quad::QuadMoveBinop *binop) {
    if (binop == nullptr || binop->dst == nullptr ||
        binop->dst->type != quad::QuadType::INT ||
        !termIsCompatibleWithInt(binop->left) ||
        !termIsCompatibleWithInt(binop->right)) {
        return false;
    }
    // Comparison results are INT even when their operands are FLOAT. Constants
    // carry no QuadType, so a constant-only comparison is not provably integer.
    if (isComparison(binop->binop) && isConst(binop->left) &&
        isConst(binop->right)) {
        return false;
    }
    return true;
}

quad::QuadTerm *foldIntConstants(const string &op, std::int32_t left,
                                 std::int32_t right) {
    if (op == "+") return termConst(wrapAdd(left, right));
    if (op == "-") return termConst(wrapSub(left, right));
    if (op == "*") return termConst(wrapMul(left, right));
    if (op == "/") {
        if (right == 0) return nullptr;
        if (left == std::numeric_limits<std::int32_t>::min() && right == -1)
            return termConst(std::numeric_limits<std::int32_t>::min());
        return termConst(static_cast<std::int32_t>(left / right));
    }
    if (op == "%") {
        if (right == 0) return nullptr;
        if (left == std::numeric_limits<std::int32_t>::min() && right == -1)
            return termConst(0);
        return termConst(static_cast<std::int32_t>(left % right));
    }
    if (op == "&") return termConst(bitsAsInt(intBits(left) & intBits(right)));
    if (op == "|") return termConst(bitsAsInt(intBits(left) | intBits(right)));
    if (op == "^") return termConst(bitsAsInt(intBits(left) ^ intBits(right)));
    if (op == "<<" || op == ">>") {
        if (right < 0 || right >= 32) return nullptr;
        unsigned shift = static_cast<unsigned>(right);
        if (op == "<<") return termConst(bitsAsInt(intBits(left) << shift));
        return termConst(arithmeticShiftRight(left, shift));
    }
    // Logical operators are intentionally not folded here. Quad constants are
    // untyped, and identities such as x && 1 -> x are not valid for arbitrary
    // non-normalized integer operands.
    return nullptr;
}

quad::QuadTerm *simplifyIntBinop(const string &op, quad::QuadTerm *left,
                                 quad::QuadTerm *right) {
    bool leftConst = isConst(left);
    bool rightConst = isConst(right);
    std::int32_t leftValue = static_cast<std::int32_t>(constVal(left));
    std::int32_t rightValue = static_cast<std::int32_t>(constVal(right));

    if (leftConst && rightConst) {
        if (quad::QuadTerm *folded = foldIntConstants(op, leftValue, rightValue))
            return folded;
    }

    if (op == "+") {
        if (leftConst && leftValue == 0) return right->clone();
        if (rightConst && rightValue == 0) return left->clone();
    } else if (op == "-") {
        if (rightConst && rightValue == 0) return left->clone();
        if (sameIntTemp(left, right)) return termConst(0);
    } else if (op == "*") {
        if ((leftConst && leftValue == 0) || (rightConst && rightValue == 0))
            return termConst(0);
        if (leftConst && leftValue == 1) return right->clone();
        if (rightConst && rightValue == 1) return left->clone();
    } else if (op == "/") {
        if (rightConst && rightValue == 1) return left->clone();
    } else if (op == "&") {
        if ((leftConst && leftValue == 0) || (rightConst && rightValue == 0))
            return termConst(0);
        if (sameIntTemp(left, right)) return left->clone();
    } else if (op == "|") {
        if (leftConst && leftValue == 0) return right->clone();
        if (rightConst && rightValue == 0) return left->clone();
        if (sameIntTemp(left, right)) return left->clone();
    } else if (op == "^") {
        if (leftConst && leftValue == 0) return right->clone();
        if (rightConst && rightValue == 0) return left->clone();
        if (sameIntTemp(left, right)) return termConst(0);
    } else if (op == "<<" || op == ">>") {
        if (rightConst && rightValue == 0) return left->clone();
    } else if (sameIntTemp(left, right)) {
        if (op == "==" || op == "<=" || op == ">=") return termConst(1);
        if (op == "!=" || op == "<" || op == ">") return termConst(0);
    }
    return nullptr;
}

bool tryNegateInt(quad::QuadMoveBinop *binop) {
    if (!intBinopIsProvable(binop) || binop->binop != "*") return false;
    bool leftMinusOne = isConst(binop->left) && constVal(binop->left) == -1;
    bool rightMinusOne = isConst(binop->right) && constVal(binop->right) == -1;
    if (!leftMinusOne && !rightMinusOne) return false;

    quad::QuadTerm *value = leftMinusOne ? binop->right->clone()
                                         : binop->left->clone();
    binop->binop = "-";
    binop->left = termConst(0);
    binop->right = value;
    binop->def = emptySet();
    binop->use = emptySet();
    binop->def->insert(new Temp(binop->dst->temp->num));
    if (isTemp(value)) binop->use->insert(new Temp(value->get_temp()->temp->num));
    return true;
}

int simplifyIntCondition(const string &relop, quad::QuadTerm *left,
                         quad::QuadTerm *right) {
    // A CJUMP has no result type. At least one typed INT temp is required so
    // constant-only and FLOAT comparisons cannot be mistaken for integer ones.
    if (!termIsCompatibleWithInt(left) || !termIsCompatibleWithInt(right) ||
        (!isIntTemp(left) && !isIntTemp(right))) {
        return 0;
    }
    if (isConst(left) && isConst(right)) {
        std::int32_t lv = static_cast<std::int32_t>(constVal(left));
        std::int32_t rv = static_cast<std::int32_t>(constVal(right));
        if (relop == "==") return lv == rv ? 1 : -1;
        if (relop == "!=") return lv != rv ? 1 : -1;
        if (relop == "<") return lv < rv ? 1 : -1;
        if (relop == "<=") return lv <= rv ? 1 : -1;
        if (relop == ">") return lv > rv ? 1 : -1;
        if (relop == ">=") return lv >= rv ? 1 : -1;
    }
    if (sameIntTemp(left, right)) {
        if (relop == "==" || relop == "<=" || relop == ">=") return 1;
        if (relop == "!=" || relop == "<" || relop == ">") return -1;
    }
    return 0;
}

void rebuildExitLabels(quad::QuadBlock *block) {
    auto *exits = new vector<Label *>();
    if (block != nullptr && block->quadlist != nullptr) {
        for (auto it = block->quadlist->rbegin(); it != block->quadlist->rend(); ++it) {
            quad::QuadStm *statement = *it;
            if (statement == nullptr) continue;
            if (statement->kind == quad::QuadKind::JUMP) {
                auto *jump = static_cast<quad::QuadJump *>(statement);
                if (jump->label != nullptr) exits->push_back(new Label(jump->label->num));
            } else if (statement->kind == quad::QuadKind::CJUMP) {
                auto *jump = static_cast<quad::QuadCJump *>(statement);
                if (jump->t != nullptr) exits->push_back(new Label(jump->t->num));
                if (jump->f != nullptr) exits->push_back(new Label(jump->f->num));
            }
            break;
        }
    }
    block->exit_labels = exits;
}

void algebraSimpFunction(quad::QuadFuncDecl *func, int &eliminated) {
    if (func == nullptr || func->quadblocklist == nullptr) return;
    for (auto *block : *func->quadblocklist) {
        if (block == nullptr || block->quadlist == nullptr) continue;
        auto *newList = new vector<quad::QuadStm *>();
        bool cfgChanged = false;
        for (auto *statement : *block->quadlist) {
            if (statement == nullptr) continue;
            if (statement->kind == quad::QuadKind::MOVE_BINOP) {
                auto *binop = static_cast<quad::QuadMoveBinop *>(statement);
                if (intBinopIsProvable(binop)) {
                    if (quad::QuadTerm *result =
                            simplifyIntBinop(binop->binop, binop->left, binop->right)) {
                        auto *move = new quad::QuadMove(binop->dst->clone(), result,
                                                       emptySet(), emptySet());
                        move->def->insert(new Temp(binop->dst->temp->num));
                        if (isTemp(result))
                            move->use->insert(new Temp(result->get_temp()->temp->num));
                        newList->push_back(move);
                        ++eliminated;
                        continue;
                    }
                    if (tryNegateInt(binop)) {
                        newList->push_back(binop);
                        ++eliminated;
                        continue;
                    }
                }
            }
            if (statement->kind == quad::QuadKind::CJUMP) {
                auto *jump = static_cast<quad::QuadCJump *>(statement);
                int result = simplifyIntCondition(jump->relop, jump->left,
                                                  jump->right);
                if (result != 0) {
                    Label *target = result > 0 ? jump->t : jump->f;
                    if (target != nullptr) {
                        newList->push_back(new quad::QuadJump(
                            new Label(target->num), emptySet(), emptySet()));
                        ++eliminated;
                        cfgChanged = true;
                        continue;
                    }
                }
            }
            newList->push_back(statement);
        }
        block->quadlist = newList;
        if (cfgChanged) rebuildExitLabels(block);
    }
}

} // namespace

namespace quad {

QuadProgram *algebraSimpProg(QuadProgram *prog, int *eliminatedOut) {
    if (prog == nullptr) return prog;
    int total = 0;
    auto *functions = new vector<QuadFuncDecl *>();
    for (auto *function : *prog->quadFuncDeclList) {
        if (function == nullptr) continue;
        int eliminated = 0;
        algebraSimpFunction(function, eliminated);
        total += eliminated;
        functions->push_back(function);
    }
    if (eliminatedOut != nullptr) *eliminatedOut = total;
    return new QuadProgram(functions, prog->last_label_num, prog->last_temp_num);
}

} // namespace quad
