#include "bitwise_idiom.hh"

#include <map>
#include <set>
#include <string>
#include <vector>

#include "quad_metadata.hh"
#include "temp.hh"

namespace quad {
namespace {

enum class BitwiseKind { None, And, Or, Xor };

std::set<Temp *> *emptyTemps() { return new std::set<Temp *>(); }

bool tempIs(QuadTerm *term, int number) {
    return term != nullptr && term->kind == QuadTermKind::TEMP &&
           term->get_temp() != nullptr && term->get_temp()->temp != nullptr &&
           term->get_temp()->type == QuadType::INT &&
           term->get_temp()->temp->num == number;
}

bool constIs(QuadTerm *term, int value) {
    return term != nullptr && term->kind == QuadTermKind::CONST &&
           term->get_const() == value;
}

bool intDestination(QuadTemp *destination, int *number) {
    if (destination == nullptr || destination->temp == nullptr ||
        destination->type != QuadType::INT) {
        return false;
    }
    if (number != nullptr) *number = destination->temp->num;
    return true;
}

bool blockLabelMatches(QuadBlock *block) {
    if (block == nullptr || block->entry_label == nullptr ||
        block->quadlist == nullptr || block->quadlist->empty()) {
        return false;
    }
    QuadStm *first = block->quadlist->front();
    return first != nullptr && first->kind == QuadKind::LABEL &&
           static_cast<QuadLabel *>(first)->label != nullptr &&
           static_cast<QuadLabel *>(first)->label->num ==
               block->entry_label->num;
}

QuadBlock *labelBlock(const std::map<int, QuadBlock *> &blocks, Label *label) {
    if (label == nullptr) return nullptr;
    auto found = blocks.find(label->num);
    return found == blocks.end() ? nullptr : found->second;
}

bool matchMoveConstant(QuadStm *statement, int value, int *destination) {
    if (statement == nullptr || statement->kind != QuadKind::MOVE) return false;
    auto *move = static_cast<QuadMove *>(statement);
    return intDestination(move->dst, destination) && constIs(move->src, value);
}

bool matchMoveTemp(QuadStm *statement, int source, int *destination) {
    if (statement == nullptr || statement->kind != QuadKind::MOVE) return false;
    auto *move = static_cast<QuadMove *>(statement);
    return intDestination(move->dst, destination) && tempIs(move->src, source);
}

bool matchBinop(QuadStm *statement, const std::string &op, int left,
                int right, int *destination) {
    if (statement == nullptr || statement->kind != QuadKind::MOVE_BINOP)
        return false;
    auto *binop = static_cast<QuadMoveBinop *>(statement);
    return binop->binop == op && intDestination(binop->dst, destination) &&
           tempIs(binop->left, left) && tempIs(binop->right, right);
}

bool matchBinopRightConstant(QuadStm *statement, const std::string &op,
                             int left, int right, int destination) {
    if (statement == nullptr || statement->kind != QuadKind::MOVE_BINOP)
        return false;
    auto *binop = static_cast<QuadMoveBinop *>(statement);
    int actualDestination = -1;
    return binop->binop == op &&
           intDestination(binop->dst, &actualDestination) &&
           actualDestination == destination && tempIs(binop->left, left) &&
           constIs(binop->right, right);
}

bool matchRemainder(std::vector<QuadStm *> *statements, std::size_t offset,
                    int input, int output, std::set<int> &scratch) {
    if (statements == nullptr || offset + 4 >= statements->size()) return false;
    int copy = -1;
    int divisor = -1;
    int quotient = -1;
    int product = -1;
    int remainder = -1;
    if (!matchMoveTemp(statements->at(offset), input, &copy) ||
        !matchMoveConstant(statements->at(offset + 1), 2, &divisor) ||
        !matchBinop(statements->at(offset + 2), "/", copy, divisor,
                    &quotient) ||
        !matchBinop(statements->at(offset + 3), "*", quotient, divisor,
                    &product) ||
        !matchBinop(statements->at(offset + 4), "-", copy, product,
                    &remainder) ||
        remainder != output) {
        return false;
    }
    for (int number : {copy, divisor, quotient, product}) {
        if (!scratch.insert(number).second) return false;
    }
    return true;
}

bool matchJumpBlock(QuadBlock *block, int destination) {
    if (!blockLabelMatches(block) || block->quadlist->size() != 2) return false;
    QuadStm *statement = block->quadlist->at(1);
    return statement != nullptr && statement->kind == QuadKind::JUMP &&
           static_cast<QuadJump *>(statement)->label != nullptr &&
           static_cast<QuadJump *>(statement)->label->num == destination;
}

bool matchAddBlock(QuadBlock *block, int result, int power,
                   int updateLabel) {
    if (!blockLabelMatches(block) || block->quadlist->size() != 3) return false;
    int destination = -1;
    if (!matchBinop(block->quadlist->at(1), "+", result, power,
                    &destination) ||
        destination != result) {
        return false;
    }
    QuadStm *last = block->quadlist->at(2);
    return last != nullptr && last->kind == QuadKind::JUMP &&
           static_cast<QuadJump *>(last)->label != nullptr &&
           static_cast<QuadJump *>(last)->label->num == updateLabel;
}

bool matchUpdateBlock(QuadBlock *block, int power, int length,
                      int testLabel) {
    if (!blockLabelMatches(block) || block->quadlist->size() != 4) return false;
    if (!matchBinopRightConstant(block->quadlist->at(1), "*", power, 2,
                                 power) ||
        !matchBinopRightConstant(block->quadlist->at(2), "-", length, 1,
                                 length)) {
        return false;
    }
    QuadStm *last = block->quadlist->at(3);
    return last != nullptr && last->kind == QuadKind::JUMP &&
           static_cast<QuadJump *>(last)->label != nullptr &&
           static_cast<QuadJump *>(last)->label->num == testLabel;
}

bool matchReturnBlock(QuadBlock *block, int result) {
    if (!blockLabelMatches(block) || block->quadlist->size() != 2) return false;
    QuadStm *last = block->quadlist->at(1);
    return last != nullptr && last->kind == QuadKind::RETURN &&
           tempIs(static_cast<QuadReturn *>(last)->exp, result);
}

bool matchEqualityToOne(QuadCJump *jump, int value) {
    return jump != nullptr && jump->relop == "==" &&
           tempIs(jump->left, value) && constIs(jump->right, 1);
}

BitwiseKind matchFunction(QuadFuncDecl *function) {
    if (function == nullptr || function->return_type != QuadType::INT ||
        function->params == nullptr || function->params->size() != 2 ||
        function->params->at(0) == nullptr ||
        function->params->at(1) == nullptr ||
        function->params->at(0)->num == function->params->at(1)->num ||
        function->quadblocklist == nullptr ||
        (function->quadblocklist->size() != 7 &&
         function->quadblocklist->size() != 8)) {
        return BitwiseKind::None;
    }

    std::map<int, QuadBlock *> blocks;
    for (QuadBlock *block : *function->quadblocklist) {
        if (!blockLabelMatches(block) ||
            !blocks.emplace(block->entry_label->num, block).second) {
            return BitwiseKind::None;
        }
    }

    QuadBlock *entry = function->quadblocklist->front();
    if (entry->quadlist->size() != 6) return BitwiseKind::None;
    int bitA = -1;
    int bitB = -1;
    int length = -1;
    int result = -1;
    int power = -1;
    if (!matchMoveConstant(entry->quadlist->at(1), 0, &bitA) ||
        !matchMoveConstant(entry->quadlist->at(2), 0, &bitB) ||
        !matchMoveConstant(entry->quadlist->at(3), 32, &length) ||
        !matchMoveConstant(entry->quadlist->at(4), 0, &result) ||
        !matchMoveConstant(entry->quadlist->at(5), 1, &power)) {
        return BitwiseKind::None;
    }
    std::set<int> distinct = {function->params->at(0)->num,
                              function->params->at(1)->num, bitA, bitB,
                              length, result, power};
    if (distinct.size() != 7) return BitwiseKind::None;

    if (entry->exit_labels == nullptr || entry->exit_labels->size() != 1)
        return BitwiseKind::None;
    QuadBlock *test = labelBlock(blocks, entry->exit_labels->front());
    if (!blockLabelMatches(test) || test->quadlist->size() != 2) {
        return BitwiseKind::None;
    }
    QuadStm *testLast = test->quadlist->at(1);
    if (testLast == nullptr || testLast->kind != QuadKind::CJUMP)
        return BitwiseKind::None;
    auto *testJump = static_cast<QuadCJump *>(testLast);
    if (testJump->relop != "!=" || !tempIs(testJump->left, length) ||
        !constIs(testJump->right, 0)) {
        return BitwiseKind::None;
    }
    QuadBlock *body = labelBlock(blocks, testJump->t);
    QuadBlock *exit = labelBlock(blocks, testJump->f);
    if (!blockLabelMatches(body) || body->quadlist->size() != 14 ||
        !matchReturnBlock(exit, result)) {
        return BitwiseKind::None;
    }

    std::set<int> scratch = distinct;
    int paramA = function->params->at(0)->num;
    int paramB = function->params->at(1)->num;
    if (!matchRemainder(body->quadlist, 1, paramA, bitA, scratch) ||
        !matchRemainder(body->quadlist, 6, paramB, bitB, scratch) ||
        !matchBinopRightConstant(body->quadlist->at(11), "/", paramA, 2,
                                 paramA) ||
        !matchBinopRightConstant(body->quadlist->at(12), "/", paramB, 2,
                                 paramB)) {
        return BitwiseKind::None;
    }
    QuadStm *conditionStatement = body->quadlist->at(13);
    if (conditionStatement == nullptr ||
        conditionStatement->kind != QuadKind::CJUMP) {
        return BitwiseKind::None;
    }
    auto *condition = static_cast<QuadCJump *>(conditionStatement);

    BitwiseKind kind = BitwiseKind::None;
    QuadBlock *add = nullptr;
    QuadBlock *noAdd = nullptr;
    QuadBlock *secondCondition = nullptr;
    if (condition->relop == "!=" && tempIs(condition->left, bitA) &&
        tempIs(condition->right, bitB) &&
        function->quadblocklist->size() == 7) {
        kind = BitwiseKind::Xor;
        add = labelBlock(blocks, condition->t);
        noAdd = labelBlock(blocks, condition->f);
    } else if (matchEqualityToOne(condition, bitA) &&
               function->quadblocklist->size() == 8) {
        QuadBlock *trueBlock = labelBlock(blocks, condition->t);
        QuadBlock *falseBlock = labelBlock(blocks, condition->f);
        if (trueBlock != nullptr && trueBlock->quadlist != nullptr &&
            trueBlock->quadlist->size() == 2 &&
            trueBlock->quadlist->at(1)->kind == QuadKind::CJUMP) {
            kind = BitwiseKind::And;
            secondCondition = trueBlock;
            noAdd = falseBlock;
        } else if (falseBlock != nullptr && falseBlock->quadlist != nullptr &&
                   falseBlock->quadlist->size() == 2 &&
                   falseBlock->quadlist->at(1)->kind == QuadKind::CJUMP) {
            kind = BitwiseKind::Or;
            add = trueBlock;
            secondCondition = falseBlock;
        }
    }
    if (kind == BitwiseKind::None) return kind;

    if (secondCondition != nullptr) {
        if (!blockLabelMatches(secondCondition) ||
            secondCondition->quadlist->size() != 2 ||
            secondCondition->quadlist->at(1)->kind != QuadKind::CJUMP) {
            return BitwiseKind::None;
        }
        auto *second = static_cast<QuadCJump *>(
            secondCondition->quadlist->at(1));
        if (!matchEqualityToOne(second, bitB)) return BitwiseKind::None;
        if (kind == BitwiseKind::And) {
            add = labelBlock(blocks, second->t);
            if (labelBlock(blocks, second->f) != noAdd)
                return BitwiseKind::None;
        } else {
            if (labelBlock(blocks, second->t) != add)
                return BitwiseKind::None;
            noAdd = labelBlock(blocks, second->f);
        }
    }
    if (!blockLabelMatches(add) || !blockLabelMatches(noAdd))
        return BitwiseKind::None;

    QuadStm *addLast = add->quadlist->empty() ? nullptr : add->quadlist->back();
    QuadStm *noAddLast =
        noAdd->quadlist->empty() ? nullptr : noAdd->quadlist->back();
    if (addLast == nullptr || addLast->kind != QuadKind::JUMP ||
        noAddLast == nullptr || noAddLast->kind != QuadKind::JUMP) {
        return BitwiseKind::None;
    }
    Label *updateLabel = static_cast<QuadJump *>(addLast)->label;
    if (updateLabel == nullptr ||
        static_cast<QuadJump *>(noAddLast)->label == nullptr ||
        static_cast<QuadJump *>(noAddLast)->label->num != updateLabel->num) {
        return BitwiseKind::None;
    }
    QuadBlock *update = labelBlock(blocks, updateLabel);
    if (!matchAddBlock(add, result, power, updateLabel->num) ||
        !matchJumpBlock(noAdd, updateLabel->num) ||
        !matchUpdateBlock(update, power, length, test->entry_label->num)) {
        return BitwiseKind::None;
    }

    std::set<int> usedLabels = {
        entry->entry_label->num, test->entry_label->num,
        body->entry_label->num, exit->entry_label->num,
        add->entry_label->num, noAdd->entry_label->num,
        update->entry_label->num};
    if (secondCondition != nullptr)
        usedLabels.insert(secondCondition->entry_label->num);
    if (usedLabels.size() != function->quadblocklist->size())
        return BitwiseKind::None;
    return kind;
}

QuadTerm *intTempTerm(int number) {
    return new QuadTerm(new QuadTemp(new Temp(number), QuadType::INT));
}

QuadBlock *makeGuardBlock(int label, int argument, int fastLabel,
                          int fallbackLabel) {
    auto *statements = new std::vector<QuadStm *>({
        new QuadLabel(new Label(label), emptyTemps(), emptyTemps()),
        new QuadCJump(">=", intTempTerm(argument), new QuadTerm(0),
                      new Label(fastLabel), new Label(fallbackLabel),
                      emptyTemps(), emptyTemps())});
    return new QuadBlock(statements, new Label(label),
                         new std::vector<Label *>({new Label(fastLabel),
                                                   new Label(fallbackLabel)}));
}

void specializeFunction(QuadFuncDecl *function, BitwiseKind kind,
                        int &lastLabel, int &lastTemp) {
    int firstGuardLabel = ++lastLabel;
    int secondGuardLabel = ++lastLabel;
    int fastLabel = ++lastLabel;
    int resultTemp = ++lastTemp;
    int fallbackLabel = function->quadblocklist->front()->entry_label->num;
    int firstParam = function->params->at(0)->num;
    int secondParam = function->params->at(1)->num;

    QuadBlock *firstGuard = makeGuardBlock(
        firstGuardLabel, firstParam, secondGuardLabel, fallbackLabel);
    QuadBlock *secondGuard = makeGuardBlock(
        secondGuardLabel, secondParam, fastLabel, fallbackLabel);
    std::string op = kind == BitwiseKind::And ? "&" :
                     kind == BitwiseKind::Or ? "|" : "^";
    auto *fastStatements = new std::vector<QuadStm *>({
        new QuadLabel(new Label(fastLabel), emptyTemps(), emptyTemps()),
        new QuadMoveBinop(new QuadTemp(new Temp(resultTemp), QuadType::INT),
                          intTempTerm(firstParam), op,
                          intTempTerm(secondParam), emptyTemps(), emptyTemps()),
        new QuadReturn(intTempTerm(resultTemp), emptyTemps(), emptyTemps())});
    auto *fastBlock = new QuadBlock(fastStatements, new Label(fastLabel),
                                    new std::vector<Label *>());

    auto *blocks = new std::vector<QuadBlock *>({firstGuard, secondGuard,
                                                 fastBlock});
    blocks->insert(blocks->end(), function->quadblocklist->begin(),
                   function->quadblocklist->end());
    function->quadblocklist = blocks;
    function->last_label_num = lastLabel;
    function->last_temp_num = lastTemp;
    rebuildQuadFunctionMetadata(function);
}

} // namespace

QuadProgram *specializeBitwiseIdioms(QuadProgram *program,
                                     int *specializedOut) {
    if (specializedOut != nullptr) *specializedOut = 0;
    if (program == nullptr || program->quadFuncDeclList == nullptr)
        return program;

    rebuildQuadProgramMetadata(program);
    int lastLabel = program->last_label_num;
    int lastTemp = program->last_temp_num;
    int specialized = 0;
    for (QuadFuncDecl *function : *program->quadFuncDeclList) {
        BitwiseKind kind = matchFunction(function);
        if (kind == BitwiseKind::None) continue;
        specializeFunction(function, kind, lastLabel, lastTemp);
        ++specialized;
    }
    program->last_label_num = lastLabel;
    program->last_temp_num = lastTemp;
    rebuildQuadProgramMetadata(program);
    if (specializedOut != nullptr) *specializedOut = specialized;
    return program;
}

} // namespace quad
