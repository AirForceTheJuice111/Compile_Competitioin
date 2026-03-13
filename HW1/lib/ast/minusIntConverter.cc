#define DEBUG
#undef DEBUG

#include "MinusIntConverter.hh"
#include "ASTheader.hh"
#include "FDMJAST.hh"
#include <iostream>
#include <variant>
#include <vector>

using namespace std;
using namespace fdmj;

#define StmList vector<Stm *>
#define ExpList vector<Exp *>

Program *minusIntRewrite(Program *root) {
    if (root == nullptr) return nullptr;
    MinusIntConverter v(nullptr);
    root->accept(v);
    return dynamic_cast<Program *>(v.cur_node);
}

template <typename T> static vector<T *> *visitList(MinusIntConverter &v, vector<T *> *tl) {
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

void MinusIntConverter::visit(Program *node) {
#ifdef DEBUG
    cerr << "Rewriting Program...\n";
#endif
    if (node == nullptr) {
        cur_node = nullptr;
        return;
    }
    // Visit the main method node if there is one
    MainMethod *m;
    if (node->main == nullptr) m = nullptr;
    else {
        node->main->accept(*this);
        if (cur_node == nullptr) m = nullptr;
        else m = static_cast<MainMethod *>(cur_node); // cur_node must point to a clone of the MainMethod
    }
    // Visit the class declaration list
    cur_node = new Program(node->get_pos()->clone(), m); // clone a new Program node
}

void MinusIntConverter::visit(MainMethod *node) {
#ifdef DEBUG
    cerr << "Rewriting MainMethod...\n";
#endif
    // Visit the statement list
    if (node == nullptr) {
        cur_node = nullptr;
        return;
    }
    StmList *sl = nullptr;
    if (node->sl != nullptr) sl = visitList<Stm>(*this, node->sl);
    cur_node = new MainMethod(node->get_pos()->clone(), sl);
}

void MinusIntConverter::visit(Assign *node) {
#ifdef DEBUG
    cerr << "Rewriting Assign..., node = " << stringASTKind(node->getASTKind()) << endl;
#endif
    if (node == nullptr) {
        cur_node = nullptr;
        return;
    }
    Exp *l = nullptr;
    if (node->left != nullptr) {
        node->left->accept(*this);
        l = static_cast<Exp *>(cur_node);
    } else {
        cerr << "Error: No left expression found in the Assign statement" << endl;
        cur_node = nullptr;
        return;
    }
    Exp *r = nullptr;
    if (node->right != nullptr) {
        node->right->accept(*this);
        r = static_cast<Exp *>(cur_node);
    } else {
        cerr << "Error: No right expression found in the Assign statement" << endl;
        cur_node = nullptr;
        return;
    }
    cur_node = new Assign(node->get_pos()->clone(), l, r);
}

void MinusIntConverter::visit(Return *node) {
#ifdef DEBUG
    cerr << "Rewriting Return..., node = " << stringASTKind(node->getASTKind()) << endl;
#endif
    if (node == nullptr) {
        cur_node = nullptr;
        return;
    }
    Exp *e = nullptr;
    if (node->exp != nullptr) {
        node->exp->accept(*this);
        e = static_cast<Exp *>(cur_node);
    } else {
        cerr << "Error: No expression found in the Return statement" << endl;
        cur_node = nullptr;
        return;
    }
    cur_node = new Return(node->get_pos()->clone(), e);
}

void MinusIntConverter::visit(BinaryOp *node) {
#ifdef DEBUG
    cerr << "Rewriting BinaryOp..., node = " << stringASTKind(node->getASTKind()) << endl;
#endif
    if (node == nullptr) {
        cur_node = nullptr;
        return;
    }
    Exp *l = nullptr;
    if (node->left != nullptr) {
        node->left->accept(*this);
        l = static_cast<Exp *>(cur_node);
    } else {
        cerr << "Error: No left expression found in the BinaryOp statement" << endl;
        cur_node = nullptr;
        return;
    }
    Exp *r = nullptr;
    if (node->right != nullptr) {
        node->right->accept(*this);
        r = static_cast<Exp *>(cur_node);
    } else {
        cerr << "Error: No right expression found in the BinaryOp statement" << endl;
        cur_node = nullptr;
        return;
    }
    cur_node = new BinaryOp(node->get_pos()->clone(), l, node->op->clone(), r);
}

void MinusIntConverter::visit(UnaryOp *node) {
    Exp *e = nullptr;
    OpExp *op = nullptr;
#ifdef DEBUG
    cerr << "Rewriting UnaryOp..., node = " << stringASTKind(node->getASTKind()) << endl;
#endif
    if (node == nullptr) {
        cur_node = nullptr;
        return;
    }
    if (node->exp != nullptr) {
        node->exp->accept(*this);
        e = static_cast<Exp *>(cur_node);
    } else {
        cerr << "Error: No expression found in the UnaryOp statement" << endl;
        cur_node = nullptr;
        return;
    }
    if (node->op == nullptr) {
        cerr << "Error: No operator found in the UnaryOp statement" << endl;
        cur_node = nullptr;
        return;
    }
    // Here's the converter logic (minus int)
    if (node->op->op == "-" && e->getASTKind() == ASTKind::IntExp) {
        int val = -(static_cast<IntExp *>(e)->val);
        cur_node = new IntExp(node->get_pos()->clone(), val);
        return;
    }
    cur_node = new UnaryOp(node->get_pos()->clone(), node->op->clone(), e);
}

void MinusIntConverter::visit(IdExp *node) {
#ifdef DEBUG
    cerr << "Rewriting IdExp..., node = " << stringASTKind(node->getASTKind()) << endl;
#endif
    cur_node = (node == nullptr) ? nullptr : static_cast<IdExp *>(node->clone());
}

void MinusIntConverter::visit(OpExp *node) {
#ifdef DEBUG
    cerr << "Rewriting OpExp..., node = " << stringASTKind(node->getASTKind()) << endl;
#endif
    cur_node = (node == nullptr) ? nullptr : static_cast<OpExp *>(node->clone());
}

void MinusIntConverter::visit(IntExp *node) {
#ifdef DEBUG
    cerr << "Rewriting IntExp..., node = " << stringASTKind(node->getASTKind()) << endl;
#endif
    cur_node = (node == nullptr) ? nullptr : static_cast<IntExp *>(node->clone());
}
