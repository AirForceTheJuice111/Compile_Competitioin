#include "loopstrengthreduction.hh"
#include "loopinductionopt.hh"
#include "defusechain.hh"
#include <cstdlib>
#include <algorithm>

using namespace std;
using namespace quad;

namespace {

int blockLabel(QuadBlock* block) {
    if (block == nullptr || block->entry_label == nullptr) return -1;
    return block->entry_label->num;
}

QuadBlock* findBlock(QuadFuncDecl* func, int label) {
    for (auto block : *func->quadblocklist) {
        if (blockLabel(block) == label) return block;
    }
    return nullptr;
}

set<int> collectTempNums(QuadFuncDecl* func, const DefUseChain& du) {
    // New temps are chosen from currently used SSA temp numbers. The printed IR
    // does not depend on pointer identity, so temp-number uniqueness is what
    // matters here.
    set<int> temps;
    for (auto entry : du.getAllDefs()) temps.insert(entry.first);
    if (func != nullptr && func->params != nullptr) {
        for (auto param : *func->params) {
            if (param != nullptr) temps.insert(param->num);
        }
    }
    return temps;
}

set<int> collectTempNumsFromFunc(QuadFuncDecl* func) {
    if (func == nullptr || func->quadblocklist == nullptr) return set<int>();
    DefUseChain du(func);
    return collectTempNums(func, du);
}

int nextFreeTemp(set<int>& used, int start) {
    // Prefer numbers near the original derived IV so the generated output stays
    // close to the homework reference files, while still avoiding collisions.
    int temp = start;
    while (used.count(temp)) ++temp;
    used.insert(temp);
    return temp;
}

QuadBlock* findPreheader(QuadFuncDecl* func, LoopHeader* loop) {
    // README guarantees each loop has a preheader. We identify it structurally:
    // it is outside the loop and has an outgoing edge to the header.
    if (loop == nullptr) return nullptr;
    for (auto block : *func->quadblocklist) {
        int label = blockLabel(block);
        if (loop->bodyBlocks.count(label) || block == nullptr || block->exit_labels == nullptr) continue;
        for (auto exitLabel : *block->exit_labels) {
            if (exitLabel != nullptr && exitLabel->num == loop->headerLabel) return block;
        }
    }
    return nullptr;
}

int backedgeLabel(QuadFuncDecl* func, LoopHeader* loop) {
    // For the HW10 tests, each optimized loop has one canonical backedge block.
    // The inserted recurrence update is placed immediately before that block's
    // terminating jump back to the header.
    if (loop == nullptr) return -1;
    for (auto block : *func->quadblocklist) {
        int label = blockLabel(block);
        if (!loop->bodyBlocks.count(label) || block == nullptr || block->exit_labels == nullptr) continue;
        for (auto exitLabel : *block->exit_labels) {
            if (exitLabel != nullptr && exitLabel->num == loop->headerLabel) return label;
        }
    }
    return -1;
}

Temp* temp(int num) {
    return new Temp(num);
}

Label* label(int num) {
    return new Label(num);
}

QuadTemp* qtemp(int num) {
    return new QuadTemp(temp(num), QuadType::INT);
}

QuadTerm* tempTerm(int num) {
    return new QuadTerm(qtemp(num));
}

QuadTerm* constTerm(int value) {
    return new QuadTerm(value);
}

set<Temp*>* defs(int num) {
    auto out = new set<Temp*>();
    out->insert(temp(num));
    return out;
}

set<Temp*>* uses(initializer_list<int> nums) {
    auto out = new set<Temp*>();
    for (int num : nums) {
        if (num != -1) out->insert(temp(num));
    }
    return out;
}

QuadMoveBinop* makeBinop(int dst, QuadTerm* left, const string& op, QuadTerm* right, initializer_list<int> useTemps) {
    // Centralize QuadMoveBinop construction so def/use sets stay consistent with
    // the expression operands after we synthesize new IR.
    return new QuadMoveBinop(qtemp(dst), left, op, right, defs(dst), uses(useTemps));
}

int useNumFromTerm(QuadTerm* term) {
    if (term == nullptr || term->kind != QuadTermKind::TEMP) return -1;
    auto quadTemp = term->get_temp();
    if (quadTemp == nullptr || quadTemp->temp == nullptr) return -1;
    return quadTemp->temp->num;
}

QuadTerm* cloneTermWithTempMap(QuadTerm* term, map<int, int>& tempMap) {
    if (term == nullptr) return nullptr;
    if (term->kind != QuadTermKind::TEMP) return term->clone();

    int num = useNumFromTerm(term);
    if (tempMap.count(num)) return tempTerm(tempMap[num]);
    return term->clone();
}

bool isPureInvariantCandidate(QuadStm* stm) {
    if (stm == nullptr) return false;
    return stm->kind == QuadKind::MOVE ||
           stm->kind == QuadKind::MOVE_BINOP ||
           stm->kind == QuadKind::PTR_CALC;
}

QuadBlock* blockOfStmt(QuadFuncDecl* func, QuadStm* target) {
    if (func == nullptr || func->quadblocklist == nullptr) return nullptr;
    for (auto block : *func->quadblocklist) {
        if (block == nullptr || block->quadlist == nullptr) continue;
        for (auto stm : *block->quadlist) {
            if (stm == target) return block;
        }
    }
    return nullptr;
}

vector<int> tempUsesOfStmt(QuadStm* stm) {
    vector<int> out;
    if (stm == nullptr) return out;
    if (stm->use == nullptr) return out;
    for (auto temp : *stm->use) {
        if (temp != nullptr) out.push_back(temp->num);
    }
    return out;
}

int defTempOfStmt(QuadStm* stm) {
    if (stm == nullptr || stm->def == nullptr || stm->def->empty()) return -1;
    auto temp = *stm->def->begin();
    if (temp == nullptr) return -1;
    return temp->num;
}

QuadStm* clonePureDefWithDst(QuadStm* stm, int newDst, map<int, int>& tempMap) { // newDst is the temp number for the cloned statement's definition. tempMap is used to rewrite operand temps if they have been materialized in preheader.
    if (stm == nullptr) return nullptr;
    if (stm->kind == QuadKind::MOVE) {
        auto move = dynamic_cast<QuadMove*>(stm);
        if (move == nullptr) return nullptr;
        auto src = cloneTermWithTempMap(move->src, tempMap);
        int use = useNumFromTerm(src);
        return new QuadMove(qtemp(newDst), src, defs(newDst), uses({use}));
    }
    if (stm->kind == QuadKind::MOVE_BINOP) {
        auto binop = dynamic_cast<QuadMoveBinop*>(stm);
        if (binop == nullptr) return nullptr;
        auto left = cloneTermWithTempMap(binop->left, tempMap);
        auto right = cloneTermWithTempMap(binop->right, tempMap);
        return makeBinop(newDst, left, binop->binop, right, {useNumFromTerm(left), useNumFromTerm(right)});
    }
    if (stm->kind == QuadKind::PTR_CALC) {
        auto ptrCalc = dynamic_cast<QuadPtrCalc*>(stm);
        if (ptrCalc == nullptr) return nullptr;
        auto dst = tempTerm(newDst);
        auto ptr = cloneTermWithTempMap(ptrCalc->ptr, tempMap);
        auto offset = cloneTermWithTempMap(ptrCalc->offset, tempMap);
        return new QuadPtrCalc(dst, ptr, offset, defs(newDst), uses({useNumFromTerm(ptr), useNumFromTerm(offset)}));
    }
    return nullptr;
}

int materializeInvariantTempInPreheader(
    QuadFuncDecl* func,
    int tempNum,
    int preheaderLabel,
    vector<QuadStm*>& preheaderStmts,
    set<int>& usedTemps,
    map<int, int>& materializedTemps
) {
    if (tempNum == -1) return -1;
    if (materializedTemps.count(tempNum)) return materializedTemps[tempNum];

    DefUseChain du(func);
    auto def = du.getDef(tempNum);
    if (def == nullptr || def->defStm == nullptr) return tempNum;

    QuadBlock* defBlock = blockOfStmt(func, def->defStm);
    if (blockLabel(defBlock) == preheaderLabel) return tempNum;
    if (!isPureInvariantCandidate(def->defStm)) return tempNum;

    // Recreate the invariant computation in the preheader using fresh temps.
    // Loop-local invariant temps cannot be referenced directly from preheader,
    // because their original definitions execute only after entering the loop.
    map<int, int> operandMap; // original temp num -> materialized temp num for operands of the cloned statement. This is needed when the invariant computation has multiple levels and we need to materialize intermediate results in preheader to preserve use-def consistency.
    for (int use : tempUsesOfStmt(def->defStm)) {
        int materializedUse = materializeInvariantTempInPreheader(
            func,
            use,
            preheaderLabel,
            preheaderStmts,
            usedTemps,
            materializedTemps
        );
        if (materializedUse != use) operandMap[use] = materializedUse;
    }

    int originalDef = defTempOfStmt(def->defStm);
    int newDef = nextFreeTemp(usedTemps, originalDef + 1);
    materializedTemps[tempNum] = newDef;
    auto cloned = clonePureDefWithDst(def->defStm, newDef, operandMap);
    if (cloned != nullptr) preheaderStmts.push_back(cloned);
    return cloned == nullptr ? tempNum : newDef;
}

void replaceTempInTerm(QuadTerm*& term, int oldTemp, int newTemp) {
    if (term == nullptr || term->kind != QuadTermKind::TEMP) return;
    auto quadTemp = term->get_temp();
    if (quadTemp != nullptr && quadTemp->temp != nullptr && quadTemp->temp->num == oldTemp) term = tempTerm(newTemp);
}

void replaceTempInArgs(vector<QuadTerm*>* args, int oldTemp, int newTemp) {
    if (args == nullptr) return;
    for (auto& arg : *args) replaceTempInTerm(arg, oldTemp, newTemp);
}

void replaceTempInStmt(QuadStm* stm, int oldTemp, int newTemp) {
    // Strength reduction replaces uses of the old derived temp with the new PHI
    // temp. This has to handle all statement shapes that may contain QuadTerms,
    // plus PHI arguments which store raw Temp* values instead of QuadTerm*.
    if (stm == nullptr) return;
    switch (stm->kind) {
        case QuadKind::MOVE: {
            auto q = dynamic_cast<QuadMove*>(stm);
            replaceTempInTerm(q->src, oldTemp, newTemp);
            break;
        }
        case QuadKind::LOAD: {
            auto q = dynamic_cast<QuadLoad*>(stm);
            replaceTempInTerm(q->src, oldTemp, newTemp);
            break;
        }
        case QuadKind::STORE: {
            auto q = dynamic_cast<QuadStore*>(stm);
            replaceTempInTerm(q->src, oldTemp, newTemp);
            replaceTempInTerm(q->dst, oldTemp, newTemp);
            break;
        }
        case QuadKind::MOVE_BINOP: {
            auto q = dynamic_cast<QuadMoveBinop*>(stm);
            replaceTempInTerm(q->left, oldTemp, newTemp);
            replaceTempInTerm(q->right, oldTemp, newTemp);
            break;
        }
        case QuadKind::PTR_CALC: {
            auto q = dynamic_cast<QuadPtrCalc*>(stm);
            replaceTempInTerm(q->ptr, oldTemp, newTemp);
            replaceTempInTerm(q->offset, oldTemp, newTemp);
            break;
        }
        case QuadKind::EXTCALL: {
            auto q = dynamic_cast<QuadExtCall*>(stm);
            replaceTempInArgs(q->args, oldTemp, newTemp);
            break;
        }
        case QuadKind::MOVE_EXTCALL: {
            auto q = dynamic_cast<QuadMoveExtCall*>(stm);
            if (q->extcall != nullptr) replaceTempInArgs(q->extcall->args, oldTemp, newTemp);
            break;
        }
        case QuadKind::CALL: {
            auto q = dynamic_cast<QuadCall*>(stm);
            replaceTempInTerm(q->obj_term, oldTemp, newTemp);
            replaceTempInArgs(q->args, oldTemp, newTemp);
            break;
        }
        case QuadKind::MOVE_CALL: {
            auto q = dynamic_cast<QuadMoveCall*>(stm);
            if (q->call != nullptr) {
                replaceTempInTerm(q->call->obj_term, oldTemp, newTemp);
                replaceTempInArgs(q->call->args, oldTemp, newTemp);
            }
            break;
        }
        case QuadKind::CJUMP: {
            auto q = dynamic_cast<QuadCJump*>(stm);
            replaceTempInTerm(q->left, oldTemp, newTemp);
            replaceTempInTerm(q->right, oldTemp, newTemp);
            break;
        }
        case QuadKind::RETURN: {
            auto q = dynamic_cast<QuadReturn*>(stm);
            replaceTempInTerm(q->exp, oldTemp, newTemp);
            break;
        }
        case QuadKind::PHI: {
            auto q = dynamic_cast<QuadPhi*>(stm);
            if (q != nullptr && q->args != nullptr) {
                for (auto& arg : *q->args) {
                    if (arg.first != nullptr && arg.first->num == oldTemp) arg.first = temp(newTemp);
                }
            }
            break;
        }
        default:
            break;
    }
}

void rewriteCJumpLimit(QuadStm* stm, const StrengthReductionPlan::ReplacementIV& repl) {
    auto cjump = dynamic_cast<QuadCJump*>(stm);
    if (cjump == nullptr) return;
    // When the loop condition uses the basic IV and the derived IV is monotonic
    // with a constant affine relation, rewrite the limit into the derived IV's
    // coordinate system. Example: k > 0 and j = 4*k + 2 becomes j > 2.
    auto rewriteSide = [&] (QuadTerm*& term, QuadTerm*& other) {
        if (term == nullptr || term->kind != QuadTermKind::TEMP) return;
        auto quadTemp = term->get_temp();
        if (quadTemp == nullptr || quadTemp->temp == nullptr || quadTemp->temp->num != repl.map.basicTempNum) return;
        if (other == nullptr || other->kind != QuadTermKind::CONST) return;
        int limit = other->get_const();
        int adjusted = repl.initExpr.sourceAfterBasicUpdate && repl.initExpr.basicStepTempNum == -1
            ? limit + repl.initExpr.basicStepValue
            : limit;
        term = tempTerm(repl.map.newPhiTemp);
        other = constTerm(repl.initExpr.basicCoeff * adjusted + repl.initExpr.constant);
    };
    rewriteSide(cjump->left, cjump->right);
    rewriteSide(cjump->right, cjump->left);
}

vector<QuadStm*> buildInitStatements(const StrengthReductionPlan::ReplacementIV& repl) {
    vector<QuadStm*> out;
    int source = repl.initExpr.initTempNum;
    if (repl.initExpr.sourceAfterBasicUpdate) {
        // Some test cases compute the derived value after the basic IV update:
        //   i_next = i - 1
        //   j = 4 * i_next + 2
        // The first value of the replacement PHI must therefore be based on
        // init(i) - 1, not directly on init(i).
        int adjusted = repl.initTemps.newInitAdjustedSourceTemp;
        if (repl.initExpr.basicStepTempNum != -1) { // not const
            int stepTemp = abs(repl.initExpr.basicStepTempNum);
            string op = repl.initExpr.basicStepTempNum < 0 ? "-" : "+";
            out.push_back(makeBinop(adjusted, tempTerm(source), op, tempTerm(stepTemp), {source, stepTemp}));
        } else if (repl.initExpr.basicStepValue < 0) { // const negative step
            out.push_back(makeBinop(adjusted, tempTerm(source), "-", constTerm(-repl.initExpr.basicStepValue), {source}));
        } else { // const non-negative step
            out.push_back(makeBinop(adjusted, tempTerm(source), "+", constTerm(repl.initExpr.basicStepValue), {source}));
        }
        source = adjusted;
    }

    if (repl.initExpr.basicCoeff == 1) {
        // Avoid emitting a multiply by one. The generated code remains simpler,
        // and cleanup has less dead arithmetic to remove.
        if (repl.initExpr.constant == 0) {
            out.push_back(new QuadMove(qtemp(repl.initTemps.newInitTemp), tempTerm(source), defs(repl.initTemps.newInitTemp), uses({source})));
        } else {
            string op = repl.initExpr.constant < 0 ? "-" : "+";
            out.push_back(makeBinop(repl.initTemps.newInitTemp, tempTerm(source), op, constTerm(abs(repl.initExpr.constant)), {source}));
        }
        return out;
    }

    int product = repl.initExpr.constant == 0 ? repl.initTemps.newInitTemp : repl.initTemps.newInitIntermediateTemp;
    // General initialization:
    //   product = source * coeff
    //   init    = product +/- constant
    // If constant is zero, product itself is the final init temp.
    out.push_back(makeBinop(product, tempTerm(source), "*", constTerm(repl.initExpr.basicCoeff), {source}));
    if (repl.initExpr.constant != 0) {
        string op = repl.initExpr.constant < 0 ? "-" : "+";
        out.push_back(makeBinop(repl.initTemps.newInitTemp, tempTerm(product), op, constTerm(abs(repl.initExpr.constant)), {product}));
    }
    return out;
}

vector<QuadStm*> buildStepPreparation(const StrengthReductionPlan::ReplacementIV& repl) {
    vector<QuadStm*> out;
    if (repl.stepExpr.newStepTemp == -1) return out;
    if (repl.stepExpr.stepTempScaleFactor == 1) return out;
    // Temp-based basic step needs scaling once in the preheader. For example,
    // i = i - step and j = 8*i + 7 produces prepared_step = 8*step, then the
    // loop only performs j = j - prepared_step.
    out.push_back(makeBinop(
        repl.stepExpr.newStepTemp,
        tempTerm(repl.stepExpr.stepSourceTempNum),
        "*",
        constTerm(repl.stepExpr.stepTempScaleFactor),
        {repl.stepExpr.stepSourceTempNum}
    ));
    return out;
}

QuadStm* buildUpdateStatement(const StrengthReductionPlan::ReplacementIV& repl) {
    // Build the recurrence update statement for the new PHI temp at the backedge. The update is either based on a prepared step temp or directly on the step value, depending on whether the basic IV step is temp-based or constant.
    if (repl.stepExpr.stepIncrementTempNum != -1) {
        // Temp step: j_next = j +/- preparedStepTemp.
        int stepTemp = repl.stepExpr.newStepTemp != -1 ? repl.stepExpr.newStepTemp : repl.stepExpr.stepIncrementTempNum;
        string op = repl.stepExpr.stepIncrementNegative ? "-" : "+";
        return makeBinop(repl.map.newBackedgeTemp, tempTerm(repl.map.newPhiTemp), op, tempTerm(stepTemp), {repl.map.newPhiTemp, stepTemp});
    }
    string op = repl.stepExpr.stepIncrementValue < 0 ? "-" : "+";
    // Constant step: j_next = j +/- abs(a * basicStep).
    return makeBinop(repl.map.newBackedgeTemp, tempTerm(repl.map.newPhiTemp), op, constTerm(abs(repl.stepExpr.stepIncrementValue)), {repl.map.newPhiTemp});
}

}

StrengthReductionPlan generateStrengthReductionPlan(
    QuadFuncDecl* func,
    const map<int, vector<DerivedInductionVar>>& derivedIVsByHeader,
    const map<int, vector<BasicInductionVar>>& basicIVsByHeader,
    LoopHeaderMap* loopHeaderMap
) {
    StrengthReductionPlan plan;
    
    if (func == nullptr || loopHeaderMap == nullptr || derivedIVsByHeader.empty()) {
        return plan;
    }

    DefUseChain du(func);
    set<int> usedTemps = collectTempNums(func, du);

    for (auto loop : loopHeaderMap->funcLoopHeaders[func]) {
        if (loop == nullptr || !derivedIVsByHeader.count(loop->headerLabel) || !basicIVsByHeader.count(loop->headerLabel)) continue;
        QuadBlock* preheader = findPreheader(func, loop);
        int backedge = backedgeLabel(func, loop);
        if (preheader == nullptr || backedge == -1) continue;

        for (auto div : derivedIVsByHeader.at(loop->headerLabel)) {
            // Find the basic IV corresponding to this derived IV. The README guarantees each derived IV has exactly one basic IV with the same source temp, so we match by that.
            auto basicIt = find_if(basicIVsByHeader.at(loop->headerLabel).begin(), basicIVsByHeader.at(loop->headerLabel).end(), [&] (const BasicInductionVar& biv) {
                return biv.phiTempNum == div.expr.basicTempNum;
            });
            if (basicIt == basicIVsByHeader.at(loop->headerLabel).end()) continue;
            const BasicInductionVar& biv = *basicIt;

            // One replacement introduces:
            //   newInitTemp      in the preheader,
            //   newPhiTemp       in the loop header,
            //   newBackedgeTemp  before the backedge jump.
            // tempReplacement then rewrites old derived-temp uses to newPhiTemp.
            StrengthReductionPlan::ReplacementIV repl;
            repl.sourceOrder = div.sourceOrder;
            repl.map.headerLabel = loop->headerLabel;
            repl.map.oldTempNum = div.tempNum;
            repl.map.basicTempNum = biv.phiTempNum;
            repl.map.newPhiTemp = nextFreeTemp(usedTemps, div.tempNum + 1);
            repl.initTemps.newInitTemp = nextFreeTemp(usedTemps, repl.map.newPhiTemp + 1);
            repl.map.newBackedgeTemp = nextFreeTemp(usedTemps, repl.initTemps.newInitTemp + 1);
            repl.initExpr.initTempNum = biv.initTempNum;
            repl.initExpr.basicCoeff = div.expr.basicCoeff;
            repl.initExpr.constant = div.expr.constant;
            repl.initExpr.sourceAfterBasicUpdate = div.sourceTempNum == biv.backedgeTempNum || static_cast<int>(div.sourceOrder) > biv.updateOrder; // If the derived IV source is updated in the backedge or after the basic IV update, the init value must be based on the basic IV after its first update, not directly on the basic IV init value.
            repl.initExpr.basicStepTempNum = biv.stepTempNum;
            repl.initExpr.basicStepValue = biv.step;
            if (repl.initExpr.sourceAfterBasicUpdate && biv.stepTempNum != -1) {
                // Rewriting the loop guard for an after-update derived IV with
                // a variable step needs a variable bound adjustment. The current
                // strength-reduction form only supports a constant guard limit,
                // so keep the original IV for correctness.
                continue;
            }
            repl.placement.initLabel = blockLabel(preheader);
            repl.placement.backedgeLabel = backedge;

            int nextTemp = repl.map.newBackedgeTemp + 1;
            if (repl.initExpr.sourceAfterBasicUpdate) repl.initTemps.newInitAdjustedSourceTemp = nextFreeTemp(usedTemps, nextTemp++);
            if (div.expr.basicCoeff != 1 && div.expr.constant != 0) repl.initTemps.newInitIntermediateTemp = nextFreeTemp(usedTemps, nextTemp++);

            if (biv.stepTempNum == -1) {
                // Constant basic step k means derived step is a*k.
                repl.stepExpr.stepIncrementValue = div.expr.basicCoeff * biv.step;
            } else {
                // Temp basic step keeps the temp identity and sign. The scale
                // factor abs(a) is materialized in the preheader when needed.
                repl.stepExpr.stepIncrementTempNum = abs(biv.stepTempNum);
                repl.stepExpr.stepIncrementNegative = biv.stepTempNum < 0;
                repl.stepExpr.stepTempScaleFactor = abs(div.expr.basicCoeff);
                if (repl.stepExpr.stepTempScaleFactor != 1) {
                    repl.stepExpr.newStepTemp = nextFreeTemp(usedTemps, nextTemp++);
                    repl.stepExpr.stepSourceTempNum = abs(biv.stepTempNum);
                    repl.stepExpr.stepIncrementTempNum = repl.stepExpr.newStepTemp;
                }
            }

            plan.tempReplacement[div.tempNum] = repl.map.newPhiTemp;
            if (div.defStm != nullptr) plan.stmtsToRemove.insert(div.defStm);
            plan.phiStmtsToAdd[repl.map.newPhiTemp] = {repl.initTemps.newInitTemp, repl.map.newBackedgeTemp};
            plan.updateStmtsToAdd[repl.map.newBackedgeTemp] = {div.expr.basicCoeff, repl.stepExpr.stepIncrementValue};
            plan.replacements.push_back(repl);
        }
    }

    return plan;
}

QuadFuncDecl* applyStrengthReduction(
    QuadFuncDecl* func,
    const StrengthReductionPlan& plan
) {
    if (func == nullptr || func->quadblocklist == nullptr) {
        return func;
    }

    if (plan.tempReplacement.empty()) {
        return func;
    }
    set<int> usedTemps = collectTempNumsFromFunc(func);
    // First rewrite all uses. It is done before insertion/removal so newly
    // inserted PHI/update statements are not accidentally rewritten.
    for (auto block : *func->quadblocklist) {
        if (block == nullptr || block->quadlist == nullptr) continue;
        for (auto stm : *block->quadlist) {
            for (auto repl : plan.replacements) rewriteCJumpLimit(stm, repl);
            for (auto entry : plan.tempReplacement) replaceTempInStmt(stm, entry.first, entry.second); // Replace old derived temp with new PHI temp.
        }
    }

    for (auto repl : plan.replacements) {
        QuadBlock* preheader = findBlock(func, repl.placement.initLabel);
        QuadBlock* header = findBlock(func, repl.map.headerLabel);
        QuadBlock* backedge = findBlock(func, repl.placement.backedgeLabel);
        if (preheader == nullptr || header == nullptr || backedge == nullptr) continue;

        vector<QuadStm*> materializedInvariantStmts;
        map<int, int> materializedTemps;
        int signedStepTemp = repl.initExpr.basicStepTempNum;
        if (signedStepTemp != -1) { // need to materialize the invariant if it's not a constant value, because the step preparation and update statements need to reference it.
            int materializedStep = materializeInvariantTempInPreheader(
                func,
                abs(signedStepTemp),
                repl.placement.initLabel,
                materializedInvariantStmts,
                usedTemps,
                materializedTemps
            );
            signedStepTemp = signedStepTemp < 0 ? -materializedStep : materializedStep;
            repl.initExpr.basicStepTempNum = signedStepTemp;
            repl.stepExpr.stepIncrementTempNum = abs(signedStepTemp);
            repl.stepExpr.stepSourceTempNum = abs(signedStepTemp);
        }

        // Insert preheader initialization before the terminator, otherwise the
        // new value would be placed after the jump and become unreachable.
        auto initPos = preheader->quadlist->end();
        if (initPos != preheader->quadlist->begin()) {
            auto last = initPos;
            --last;
            if ((*last)->kind == QuadKind::JUMP || (*last)->kind == QuadKind::CJUMP || (*last)->kind == QuadKind::RETURN) initPos = last;
        }
        for (auto stm : materializedInvariantStmts) {
            initPos = preheader->quadlist->insert(initPos, stm);
            ++initPos;
        }
        for (auto stm : buildInitStatements(repl)) {
            initPos = preheader->quadlist->insert(initPos, stm);
            ++initPos;
        }
        for (auto stm : buildStepPreparation(repl)) {
            initPos = preheader->quadlist->insert(initPos, stm);
            ++initPos;
        }

        // Insert the new PHI node with the new derived IV temp as destination and the backedge/basic-IV-based init temps as arguments. It is placed at the top of the header block to ensure it dominates all its uses.
        auto phiArgs = new vector<pair<Temp*, Label*>>();
        phiArgs->push_back({temp(repl.map.newBackedgeTemp), label(repl.placement.backedgeLabel)});
        phiArgs->push_back({temp(repl.initTemps.newInitTemp), label(repl.placement.initLabel)});
        auto phi = new QuadPhi(qtemp(repl.map.newPhiTemp), phiArgs, defs(repl.map.newPhiTemp), uses({repl.map.newBackedgeTemp, repl.initTemps.newInitTemp}));
        auto phiPos = header->quadlist->begin();
        // PHI nodes must stay together at the top of a SSA block, after LABEL
        // and before executable statements.
        while (phiPos != header->quadlist->end() && ((*phiPos)->kind == QuadKind::LABEL || (*phiPos)->kind == QuadKind::PHI)) ++phiPos;
        header->quadlist->insert(phiPos, phi);

        // Insert recurrence update immediately before the backedge terminator so
        // the new PHI receives the value from the correct iteration.
        auto updatePos = backedge->quadlist->end();
        if (updatePos != backedge->quadlist->begin()) {
            auto last = updatePos;
            --last;
            if ((*last)->kind == QuadKind::JUMP || (*last)->kind == QuadKind::CJUMP || (*last)->kind == QuadKind::RETURN) updatePos = last;
        }
        backedge->quadlist->insert(updatePos, buildUpdateStatement(repl));
    }

    for (auto block : *func->quadblocklist) {
        if (block == nullptr || block->quadlist == nullptr) continue;
        // Only remove the top-level derived definition here. Its now-dead
        // operands and old basic-IV cycles are removed by the cleanup pass.
        // remove_if + erase: the elements that need to be removed are moved to the end of the vector together, then erased in one go. This is more efficient than erasing while iterating, which would cause repeated shifting of elements.
        block->quadlist->erase(remove_if(block->quadlist->begin(), block->quadlist->end(), [&] (QuadStm* stm) {
            return plan.stmtsToRemove.count(stm);
        }), block->quadlist->end());
    }

    return func;
}
