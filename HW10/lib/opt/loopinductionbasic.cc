#include "loopinductionbasic.hh"
#include "defusechain.hh"
#include "loopinductionopt.hh"
#include <algorithm>

using namespace std;
using namespace quad;

namespace {

int blockLabel(QuadBlock* block) {
    if (block == nullptr || block->entry_label == nullptr) return -1;
    return block->entry_label->num;
}

int tempNum(Temp* temp) {
    if (temp == nullptr) return -1;
    return temp->num;
}

int termTempNum(QuadTerm* term) {
    if (term == nullptr || term->kind != QuadTermKind::TEMP) return -1;
    auto quadTemp = term->get_temp();
    if (quadTemp == nullptr || quadTemp->temp == nullptr) return -1;
    return quadTemp->temp->num;
}

bool isLoopInvariantTemp(int temp, const set<int>& loopDefs) {
    return temp != -1 && !loopDefs.count(temp);
}

set<int> defsInLoop(QuadFuncDecl* func, const set<int>& bodyBlocks, const DefUseChain& du) {
    set<int> defs;
    for (auto block : *func->quadblocklist) {
        if (!bodyBlocks.count(blockLabel(block)) || block->quadlist == nullptr) continue;
        for (auto stm : *block->quadlist) {
            auto stmDefs = du.getDefsBy(stm);
            defs.insert(stmDefs.begin(), stmDefs.end());
        }
    }
    return defs;
}

int statementOrder(QuadFuncDecl* func, QuadStm* target) {
    int order = 0;
    for (auto block : *func->quadblocklist) {
        if (block == nullptr || block->quadlist == nullptr) continue;
        for (auto stm : *block->quadlist) {
            if (stm == target) return order;
            ++order;
        }
    }
    return -1;
}

bool parseUpdate(QuadStm* stm, int phiTemp, const set<int>& loopDefs, int& step, int& stepTempNum) {
    auto binop = dynamic_cast<QuadMoveBinop*>(stm);
    if (binop == nullptr) return false;

    int leftTemp = termTempNum(binop->left);
    int rightTemp = termTempNum(binop->right);
    bool leftConst = binop->left != nullptr && binop->left->kind == QuadTermKind::CONST;
    bool rightConst = binop->right != nullptr && binop->right->kind == QuadTermKind::CONST;

    if (binop->binop == "+") {
        if (leftTemp == phiTemp && rightConst) {
            step = binop->right->get_const();
            stepTempNum = -1;
            return true;
        }
        if (rightTemp == phiTemp && leftConst) {
            step = binop->left->get_const();
            stepTempNum = -1;
            return true;
        }
        if (leftTemp == phiTemp && isLoopInvariantTemp(rightTemp, loopDefs)) {
            step = 0;
            stepTempNum = rightTemp;
            return true;
        }
        if (rightTemp == phiTemp && isLoopInvariantTemp(leftTemp, loopDefs)) {
            step = 0;
            stepTempNum = leftTemp;
            return true;
        }
    }

    if (binop->binop == "-") {
        if (leftTemp == phiTemp && rightConst) {
            step = -binop->right->get_const();
            stepTempNum = -1;
            return true;
        }
        if (leftTemp == phiTemp && isLoopInvariantTemp(rightTemp, loopDefs)) {
            step = 0;
            stepTempNum = -rightTemp;
            return true;
        }
    }
    return false;
}

}

map<int, vector<BasicInductionVar>> discoverBasicInductionVars(QuadFuncDecl* func, LoopHeaderMap *loopHeaderMap, 
        const DefUseChain& du, const ControlFlowInfo& cfi) {
    map<int, vector<BasicInductionVar>> result;
    if (func == nullptr || func->quadblocklist == nullptr || loopHeaderMap == nullptr) {
        return result;
    }
    if (!loopHeaderMap->funcLoopHeaders.count(func)) {
        return result;
    }
    for (auto loop : loopHeaderMap->funcLoopHeaders[func]) {
        if (loop == nullptr || !cfi.labelToBlock.count(loop->headerLabel)) continue;
        QuadBlock* header = cfi.labelToBlock.at(loop->headerLabel);
        if (header == nullptr || header->quadlist == nullptr) continue;
        set<int> loopDefs = defsInLoop(func, loop->bodyBlocks, du);

        for (auto stm : *header->quadlist) {
            auto phi = dynamic_cast<QuadPhi*>(stm);
            if (phi == nullptr || phi->temp_exp == nullptr || phi->temp_exp->temp == nullptr || phi->args == nullptr) continue;

            int phiTemp = phi->temp_exp->temp->num;
            for (auto arg : *phi->args) {
                int incomingTemp = tempNum(arg.first);
                int incomingLabel = arg.second == nullptr ? -1 : arg.second->num;
                if (!loop->bodyBlocks.count(incomingLabel)) continue;

                auto def = du.getDef(incomingTemp);
                if (def == nullptr || def->defStm == nullptr || !loop->bodyBlocks.count(blockLabel(def->defBlock))) continue;

                int step = 0;
                int stepTempNum = -1;
                if (!parseUpdate(def->defStm, phiTemp, loopDefs, step, stepTempNum)) continue;

                int initTemp = -1;
                for (auto otherArg : *phi->args) {
                    int otherLabel = otherArg.second == nullptr ? -1 : otherArg.second->num;
                    if (!loop->bodyBlocks.count(otherLabel)) initTemp = tempNum(otherArg.first);
                }
                if (initTemp == -1) continue;

                BasicInductionVar biv(loop->headerLabel, phiTemp, initTemp, incomingTemp, step, stm, def->defStm);
                biv.stepTempNum = stepTempNum;
                biv.updateOrder = statementOrder(func, def->defStm);
                if (stepTempNum != -1) biv.addRelatedTemp(abs(stepTempNum));
                result[loop->headerLabel].push_back(biv);
            }
        }
    }

    classifyRelatedTemps(func, result, du);

    return result;
}

// Classify related temps as useless (only in backedges) or useful (in computations)
void classifyRelatedTemps(
    QuadFuncDecl* func,
    map<int, vector<BasicInductionVar>>& basicByHeader,
    const DefUseChain& du
) {
    if (func == nullptr || func->quadblocklist == nullptr) {
        return;
    }
    for (auto& entry : basicByHeader) {
        for (auto& biv : entry.second) {
            set<QuadStm*> familyStmts;
            if (biv.phiStm != nullptr) familyStmts.insert(biv.phiStm);
            if (biv.updateStm != nullptr) familyStmts.insert(biv.updateStm);

            for (int temp : biv.relatedTemps) {
                auto def = du.getDef(temp);
                bool useful = temp == biv.phiTempNum;
                if (def != nullptr) {
                    for (auto use : def->useSet) {
                        if (!familyStmts.count(use.second)) useful = true;
                    }
                }
                if (useful) biv.markUseful(temp);
                else biv.markUseless(temp);
            }
        }
    }
    return ;
}
