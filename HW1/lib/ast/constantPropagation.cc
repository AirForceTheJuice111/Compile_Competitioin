#define DEBUG
#undef DEBUG

#include "constantPropagation.hh"
#include "MinusIntConverter.hh"
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace fdmj;

#define StmList vector<Stm *>

// 入口函数：先调用 minusIntRewrite 消除 -(常量)，再进行常量折叠
Program *constantPropagate(Program *root) {
    if (root == nullptr) return nullptr;

    Program *rewritten = minusIntRewrite(root);
    if (rewritten == nullptr) return nullptr;

    ConstantPropagator v;
    rewritten->accept(v);
    return dynamic_cast<Program *>(v.cur_node);
}

// 辅助函数：对节点列表逐个 visit，收集结果
template <typename T> static vector<T *> *visitList(ConstantPropagator &v, vector<T *> *tl) {
    if (tl == nullptr || tl->size() == 0) return nullptr;
    vector<T *> *vt = new vector<T *>();
    for (T *x : *tl) {
        if (x == nullptr) continue;
        x->accept(v);
        if (v.cur_node == nullptr) continue;
        vt->push_back(static_cast<T *>(v.cur_node));
    }
    if (vt->size() == 0) {
        delete vt;
        vt = nullptr;
    }
    return vt;
}

void ConstantPropagator::visit(Program *node) {
#ifdef DEBUG
    cerr << "CP: Program\n";
#endif
    if (node == nullptr) {
        cur_node = nullptr;
        return;
    }
    MainMethod *m = nullptr;
    if (node->main != nullptr) {
        node->main->accept(*this);
        m = (cur_node != nullptr) ? static_cast<MainMethod *>(cur_node) : nullptr;
    }
    cur_node = new Program(node->get_pos()->clone(), m);
}

void ConstantPropagator::visit(MainMethod *node) {
#ifdef DEBUG
    cerr << "CP: MainMethod\n";
#endif
    if (node == nullptr) {
        cur_node = nullptr;
        return;
    }
    StmList *sl = nullptr;
    if (node->sl != nullptr) sl = visitList<Stm>(*this, node->sl);
    cur_node = new MainMethod(node->get_pos()->clone(), sl);
}

void ConstantPropagator::visit(Assign *node) {
#ifdef DEBUG
    cerr << "CP: Assign\n";
#endif
    if (node == nullptr) {
        cur_node = nullptr;
        return;
    }
    Exp *l = nullptr;
    if (node->left != nullptr) {
        node->left->accept(*this);
        l = static_cast<Exp *>(cur_node); // 不是直接clone，因为accept后cur_node可能已经被修改了，指向其子节点
    }
    Exp *r = nullptr;
    if (node->right != nullptr) {
        node->right->accept(*this);
        r = static_cast<Exp *>(cur_node);
    }
    cur_node = new Assign(node->get_pos()->clone(), l, r);
}

void ConstantPropagator::visit(Return *node) {
#ifdef DEBUG
    cerr << "CP: Return\n";
#endif
    if (node == nullptr) {
        cur_node = nullptr;
        return;
    }
    Exp *e = nullptr;
    if (node->exp != nullptr) {
        node->exp->accept(*this);
        e = static_cast<Exp *>(cur_node);
    }
    cur_node = new Return(node->get_pos()->clone(), e);
}

void ConstantPropagator::visit(BinaryOp *node) {
#ifdef DEBUG
    cerr << "CP: BinaryOp\n";
#endif
    if (node == nullptr) {
        cur_node = nullptr;
        return;
    }

    // 自底向上，先递归处理左右子树
    Exp *l = nullptr;
    if (node->left != nullptr) {
        node->left->accept(*this);
        l = static_cast<Exp *>(cur_node);
    }
    Exp *r = nullptr;
    if (node->right != nullptr) {
        node->right->accept(*this);
        r = static_cast<Exp *>(cur_node);
    }

    // 核心逻辑：若左右都是 IntExp，直接计算并折叠为一个 IntExp
    if (l != nullptr && r != nullptr && l->getASTKind() == ASTKind::IntExp && r->getASTKind() == ASTKind::IntExp) {
        int lval = static_cast<IntExp *>(l)->val;
        int rval = static_cast<IntExp *>(r)->val;
        string opStr = node->op->op;

        int res = 0;
        if (opStr == "+") res = lval + rval;
        else if (opStr == "-") res = lval - rval;
        else if (opStr == "*") res = lval * rval;
        else if (opStr == "/") {
            if (rval == 0) {
                cerr << "Error: Division by zero" << endl;
                cur_node = new BinaryOp(node->get_pos()->clone(), l, node->op->clone(), r);
                return;
            }
            res = lval / rval;
        } else {
            // 未知运算符，不折叠
            cur_node = new BinaryOp(node->get_pos()->clone(), l, node->op->clone(), r);
            return;
        }
        cur_node = new IntExp(node->get_pos()->clone(), res);
        return;
    }

    // 无法折叠，保留原结构
    cur_node = new BinaryOp(node->get_pos()->clone(), l, node->op->clone(), r);
}

void ConstantPropagator::visit(UnaryOp *node) {
#ifdef DEBUG
    cerr << "CP: UnaryOp\n";
#endif
    if (node == nullptr) {
        cur_node = nullptr;
        return;
    }

    Exp *e = nullptr;
    if (node->exp != nullptr) {
        node->exp->accept(*this);
        e = static_cast<Exp *>(cur_node);
    }

    // 若经过子树折叠后 UnaryOp(-, IntExp) 出现，也将其折叠
    // if (e != nullptr && node->op != nullptr && node->op->op == "-" &&
    //     e->getASTKind() == ASTKind::IntExp) {
    //     int val = -(static_cast<IntExp *>(e)->val);
    //     cur_node = new IntExp(node->getPos()->clone(), val);
    //     return;
    // }

    OpExp *o = (node->op != nullptr) ? node->op->clone() : nullptr;
    cur_node = new UnaryOp(node->get_pos()->clone(), o, e);
}

void ConstantPropagator::visit(IdExp *node) {
    cur_node = (node == nullptr) ? nullptr : node->clone();
}

void ConstantPropagator::visit(OpExp *node) {
    cur_node = (node == nullptr) ? nullptr : node->clone();
}

void ConstantPropagator::visit(IntExp *node) {
    cur_node = (node == nullptr) ? nullptr : node->clone();
}
