#include "algebrasimp.hh"

#include <string>
#include <vector>

#include "quad.hh"
#include "temp.hh"

using namespace std;

namespace {

bool isConst(quad::QuadTerm *t) {
    return t && t->kind == quad::QuadTermKind::CONST;
}
bool isTemp(quad::QuadTerm *t) {
    return t && t->kind == quad::QuadTermKind::TEMP;
}
int constVal(quad::QuadTerm *t) { return t ? t->get_const() : 0; }
quad::QuadTerm *termConst(int val) { return new quad::QuadTerm(val); }
quad::QuadTerm *termTemp(int num, quad::QuadType type) {
    return new quad::QuadTerm(new quad::QuadTemp(new Temp(num), type));
}
set<Temp*> *emptySet() { return new set<Temp*>(); }

quad::QuadTerm *simplifyBinop(const string &op, quad::QuadTerm *left,
                               quad::QuadTerm *right) {
    bool lc = isConst(left), rc = isConst(right);
    int lv = constVal(left), rv = constVal(right);

    if (lc && rc) {
        if (op == "+") return termConst(lv + rv);
        if (op == "-") return termConst(lv - rv);
        if (op == "*") return termConst(lv * rv);
        if (op == "/") { if (rv == 0) return nullptr; return termConst(lv / rv); }
        if (op == "%") { if (rv == 0) return nullptr; return termConst(lv % rv); }
        if (op == "&&") return termConst((lv && rv) ? 1 : 0);
        if (op == "||") return termConst((lv || rv) ? 1 : 0);
        if (op == "&") return termConst(lv & rv);
        if (op == "|") return termConst(lv | rv);
        if (op == "^") return termConst(lv ^ rv);
        if (op == "<<") return termConst(lv << rv);
        if (op == ">>") return termConst(lv >> rv);
        if (op == "==") return termConst((lv == rv) ? 1 : 0);
        if (op == "!=") return termConst((lv != rv) ? 1 : 0);
        if (op == "<") return termConst((lv < rv) ? 1 : 0);
        if (op == "<=") return termConst((lv <= rv) ? 1 : 0);
        if (op == ">") return termConst((lv > rv) ? 1 : 0);
        if (op == ">=") return termConst((lv >= rv) ? 1 : 0);
        return nullptr;
    }

    if (op == "+") {
        if (lc && lv == 0) return right;
        if (rc && rv == 0) return left;
    }
    if (op == "-") { if (rc && rv == 0) return left; }
    if (op == "*") {
        if ((lc && lv == 0) || (rc && rv == 0)) return termConst(0);
        if (lc && lv == 1) return right;
        if (rc && rv == 1) return left;
    }
    if (op == "/") { if (rc && rv == 1) return left; }
    if (op == "&") {
        if ((lc && lv == 0) || (rc && rv == 0)) return termConst(0);
    }
    if (op == "|") {
        if (lc && lv == 0) return right;
        if (rc && rv == 0) return left;
    }
    if (op == "^") {
        if (lc && lv == 0) return right;
        if (rc && rv == 0) return left;
    }
    if (op == "<<" || op == ">>") { if (rc && rv == 0) return left; }
    if (op == "&&") {
        if ((lc && lv == 0) || (rc && rv == 0)) return termConst(0);
        if (lc && lv != 0) return right;
        if (rc && rv != 0) return left;
    }
    if (op == "||") {
        if (lc && lv != 0) return termConst(1);
        if (rc && rv != 0) return termConst(1);
        if (lc && lv == 0) return right;
        if (rc && rv == 0) return left;
    }
    if (op == "-" && isTemp(left) && isTemp(right) &&
        left->get_temp()->temp->num == right->get_temp()->temp->num)
        return termConst(0);
    return nullptr;
}

int simplifyCondition(const string &relop, quad::QuadTerm *left,
                      quad::QuadTerm *right) {
    if (isConst(left) && isConst(right)) {
        int lv = constVal(left), rv = constVal(right);
        if (relop == "==") return (lv == rv) ? 1 : -1;
        if (relop == "!=") return (lv != rv) ? 1 : -1;
        if (relop == "<") return (lv < rv) ? 1 : -1;
        if (relop == "<=") return (lv <= rv) ? 1 : -1;
        if (relop == ">") return (lv > rv) ? 1 : -1;
        if (relop == ">=") return (lv >= rv) ? 1 : -1;
    }
    if (isTemp(left) && isTemp(right) &&
        left->get_temp()->temp->num == right->get_temp()->temp->num) {
        if (relop == "==" || relop == "<=" || relop == ">=") return 1;
        if (relop == "!=" || relop == "<" || relop == ">") return -1;
    }
    return 0;
}

void algebraSimpFunction(quad::QuadFuncDecl *func, int &eliminated) {
    if (!func || !func->quadblocklist) return;
    for (auto *block : *func->quadblocklist) {
        if (!block || !block->quadlist) continue;
        auto *newList = new vector<quad::QuadStm*>();
        for (auto *stm : *block->quadlist) {
            if (!stm) continue;
            if (stm->kind == quad::QuadKind::MOVE_BINOP) {
                auto *b = static_cast<quad::QuadMoveBinop*>(stm);
                auto *r = simplifyBinop(b->binop, b->left, b->right);
                if (r) {
                    int dn = b->dst->temp->num;
                    auto *mv = new quad::QuadMove(b->dst->clone(), r, emptySet(), emptySet());
                    mv->def->insert(new Temp(dn));
                    if (r->kind == quad::QuadTermKind::TEMP)
                        mv->use->insert(new Temp(r->get_temp()->temp->num));
                    newList->push_back(mv);
                    eliminated++;
                    continue;
                }
            }
            if (stm->kind == quad::QuadKind::CJUMP) {
                auto *c = static_cast<quad::QuadCJump*>(stm);
                int res = simplifyCondition(c->relop, c->left, c->right);
                if (res != 0) {
                    Label *tgt = (res > 0) ? c->t : c->f;
                    newList->push_back(new quad::QuadJump(new Label(tgt->num), emptySet(), emptySet()));
                    eliminated++;
                    continue;
                }
            }
            newList->push_back(stm);
        }
        block->quadlist = newList;
    }
}

} // namespace

namespace quad {
QuadProgram *algebraSimpProg(QuadProgram *prog, int *eliminatedOut) {
    if (!prog) return prog;
    int total = 0;
    auto *nf = new vector<QuadFuncDecl*>();
    for (auto *fd : *prog->quadFuncDeclList) {
        if (!fd) continue;
        int e = 0;
        algebraSimpFunction(fd, e);
        total += e;
        nf->push_back(fd);
    }
    if (eliminatedOut) *eliminatedOut = total;
    return new QuadProgram(nf, prog->last_label_num, prog->last_temp_num);
}
} // namespace quad
