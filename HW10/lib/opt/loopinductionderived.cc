#include "loopinductionderived.hh"
#include "defusechain.hh"
#include "loopinductionopt.hh"
#include <algorithm>
#include <optional>

using namespace std;
using namespace quad;

namespace {

struct AffineValue {
    // Represents coeff * basicTemp + constant.
    // basicTemp == -1 means this is a pure constant.
    int basicTemp = -1;
    int coeff = 0;
    int constant = 0;
    // The immediate SSA temp that produced this affine value. This is used later
    // to tell whether the derived expression was computed from the basic IV's
    // backedge value or from the header PHI value.
    int sourceTemp = -1;
};

int blockLabel(QuadBlock* block) {
    if (block == nullptr || block->entry_label == nullptr) return -1;
    return block->entry_label->num;
}

int termTempNum(QuadTerm* term) {
    if (term == nullptr || term->kind != QuadTermKind::TEMP) return -1;
    auto quadTemp = term->get_temp();
    if (quadTemp == nullptr || quadTemp->temp == nullptr) return -1;
    return quadTemp->temp->num;
}

optional<AffineValue> valueOfTerm(QuadTerm* term, const map<int, AffineValue>& values) {
    // Constants are affine values with no basic variable; temps are affine only
    // after they have been seen in the local propagation table.
    if (term == nullptr) return nullopt;
    if (term->kind == QuadTermKind::CONST) return AffineValue{-1, 0, term->get_const(), -1};
    int temp = termTempNum(term);
    if (temp != -1 && values.count(temp)) return values.at(temp);
    return nullopt;
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

bool sameBase(const AffineValue& a, const AffineValue& b) {
    // A pure constant can combine with any base. Two non-constant affine values
    // may combine only if they are derived from the same basic IV.
    return a.basicTemp == -1 || b.basicTemp == -1 || a.basicTemp == b.basicTemp;
}

AffineValue combineAdd(const AffineValue& a, const AffineValue& b, int sign) {
    AffineValue out;
    out.basicTemp = a.basicTemp != -1 ? a.basicTemp : b.basicTemp;
    out.coeff = a.coeff + sign * b.coeff;
    out.constant = a.constant + sign * b.constant;
    out.sourceTemp = a.sourceTemp != -1 ? a.sourceTemp : b.sourceTemp;
    return out;
}

optional<AffineValue> evalBinop(QuadMoveBinop* binop, const map<int, AffineValue>& values) {
    auto left = valueOfTerm(binop->left, values);
    auto right = valueOfTerm(binop->right, values);
    if (!left || !right) return nullopt;

    // Keep the recognizer intentionally small: HW10 only asks for first-order
    // affine expressions a * i + b. Multiplication is therefore accepted only
    // when one operand is already known to be a pure constant.
    if (binop->binop == "+" && sameBase(*left, *right)) return combineAdd(*left, *right, 1);
    if (binop->binop == "-" && sameBase(*left, *right)) return combineAdd(*left, *right, -1);

    if (binop->binop == "*") {
        if (left->basicTemp == -1 && right->basicTemp != -1) {
            auto out = *right;
            out.coeff *= left->constant;
            out.constant *= left->constant;
            return out;
        }
        if (right->basicTemp == -1 && left->basicTemp != -1) {
            auto out = *left;
            out.coeff *= right->constant;
            out.constant *= right->constant;
            return out;
        }
    }
    return nullopt;
}

bool hasExternalUse(const DefUseChain& du, QuadStm* defStm, int temp) {
    auto def = du.getDef(temp);
    if (def == nullptr) return false;
    for (auto use : def->useSet) {
        if (use.second != defStm) return true;
    }
    return false;
}

bool hasNonAffineConsumer(const DefUseChain& du, QuadStm* defStm, int temp) {
    // Avoid optimizing temporary products such as t1 = 4 * i when they are only
    // consumed by t2 = t1 + 2. The externally meaningful derived IV is t2, not t1.
    auto def = du.getDef(temp);
    if (def == nullptr) return false;
    for (auto use : def->useSet) {
        if (use.second == defStm) continue;
        if (use.second == nullptr || use.second->kind != QuadKind::MOVE_BINOP) return true;
    }
    return false;
}

int immediateAffineSource(QuadMoveBinop* binop, const map<int, AffineValue>& values) {
    int leftTemp = termTempNum(binop->left);
    int rightTemp = termTempNum(binop->right);
    if (leftTemp != -1 && values.count(leftTemp) && values.at(leftTemp).basicTemp != -1) return leftTemp;
    if (rightTemp != -1 && values.count(rightTemp) && values.at(rightTemp).basicTemp != -1) return rightTemp;
    return -1;
}

}

map<int, vector<DerivedInductionVar>> discoverDerivedInductionVars(QuadFuncDecl* func, LoopHeaderMap *loopHeaderMap, 
        const DefUseChain& du, const ControlFlowInfo& cfi) {
    map<int, vector<DerivedInductionVar>> result;
    if (func == nullptr || func->quadblocklist == nullptr || loopHeaderMap == nullptr) {
        return result;
    }
    if (!loopHeaderMap->funcLoopHeaders.count(func)) {
        return result;
    }
    auto basicByHeader = discoverBasicInductionVars(func, loopHeaderMap, du, cfi);
    for (auto loop : loopHeaderMap->funcLoopHeaders[func]) {
        if (loop == nullptr || !basicByHeader.count(loop->headerLabel)) continue;

        // Seed the affine table with each basic IV family. Both the PHI temp and
        // the backedge temp represent the same symbolic variable for recognition;
        // sourceTemp still records which one appeared syntactically.
        map<int, AffineValue> values;
        set<int> basicTemps;
        for (auto biv : basicByHeader[loop->headerLabel]) {
            values[biv.phiTempNum] = AffineValue{biv.phiTempNum, 1, 0, biv.phiTempNum};
            values[biv.backedgeTempNum] = AffineValue{biv.phiTempNum, 1, 0, biv.backedgeTempNum};
            basicTemps.insert(biv.phiTempNum);
            basicTemps.insert(biv.backedgeTempNum);
        }

        for (auto block : *func->quadblocklist) {
            if (block == nullptr || block->quadlist == nullptr || !loop->bodyBlocks.count(blockLabel(block))) continue;
            for (auto stm : *block->quadlist) {
                // SSA order in the block list is used as a forward propagation
                // order. If a statement computes a new affine value, later
                // statements can build on it.
                auto binop = dynamic_cast<QuadMoveBinop*>(stm);
                if (binop == nullptr || binop->dst == nullptr || binop->dst->temp == nullptr) continue;
                auto value = evalBinop(binop, values);
                int dst = binop->dst->temp->num;
                if (!value || value->basicTemp == -1 || value->coeff == 0) continue;
                if (basicTemps.count(dst)) continue;

                value->sourceTemp = value->sourceTemp == -1 ? value->basicTemp : value->sourceTemp;
                values[dst] = *value;

                // Only report values that escape the affine chain. This matches
                // the reference output and prevents duplicate strength reductions
                // for intermediate arithmetic temps.
                if (!hasExternalUse(du, stm, dst) || !hasNonAffineConsumer(du, stm, dst)) continue;

                AffineIVExpr expr{value->basicTemp, value->coeff, value->constant};
                int sourceTemp = immediateAffineSource(binop, values);
                if (sourceTemp == -1) sourceTemp = value->sourceTemp;
                result[loop->headerLabel].push_back(DerivedInductionVar(
                    loop->headerLabel,
                    dst,
                    sourceTemp,
                    statementOrder(func, stm),
                    expr,
                    stm
                ));
            }
        }
    }

    return result;
}
