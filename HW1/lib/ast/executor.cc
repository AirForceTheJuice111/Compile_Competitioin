#define DEBUG
#undef DEBUG

#include "executor.hh"
#include <iostream>
#include <map>
#include <string>

using namespace std;
using namespace fdmj;

int execute(Program *root) {
    if (root == nullptr) return 0;
    Executor v;
    root->accept(v);
    return v.result;
}

void Executor::visit(Program *node) {
#ifdef DEBUG
    cerr << "Exec: Program\n";
#endif
    if (node == nullptr) return;
    if (node->main != nullptr) node->main->accept(*this);
}

void Executor::visit(MainMethod *node) {
#ifdef DEBUG
    cerr << "Exec: MainMethod\n";
#endif
    if (node == nullptr) return;
    if (node->sl != nullptr) {
        for (auto s : *(node->sl))
            if (s != nullptr) s->accept(*this);
    }
}

void Executor::visit(Assign *node) {
#ifdef DEBUG
    cerr << "Exec: Assign\n";
#endif
    if (node == nullptr) return;

    // 先求右侧表达式的值
    if (node->exp != nullptr) node->exp->accept(*this);
    int val = result;

    if (node->left != nullptr && node->left->getASTKind() == ASTKind::IdExp) {
        string name = static_cast<IdExp *>(node->left)->id;
        varTable[name] = val;
        varDefined[name] = true;
    }
}

void Executor::visit(Return *node) {
#ifdef DEBUG
    cerr << "Exec: Return\n";
#endif
    if (node == nullptr) return;
    if (node->exp != nullptr) node->exp->accept(*this);
    // 在 return 语句处输出返回值
    cout << result << endl;
}

void Executor::visit(BinaryOp *node) {
#ifdef DEBUG
    cerr << "Exec: BinaryOp\n";
#endif
    if (node == nullptr) return;

    // 先求左操作数
    node->left->accept(*this);
    int lval = result;

    // 再求右操作数
    node->right->accept(*this);
    int rval = result;

    // 根据运算符计算
    string opStr = node->op->op;
    if (opStr == "+") result = lval + rval;
    else if (opStr == "-") result = lval - rval;
    else if (opStr == "*") result = lval * rval;
    else if (opStr == "/") {
        if (rval == 0) {
            cerr << "Error: Division by zero at line " << node->getPos()->sline << ", column " << node->getPos()->scolumn << endl;
            result = 0;
        } else {
            result = lval / rval;
        }
    }
}

void Executor::visit(UnaryOp *node) {
#ifdef DEBUG
    cerr << "Exec: UnaryOp\n";
#endif
    if (node == nullptr) return;

    // 先求操作数的值
    node->exp->accept(*this);

    // 取反
    if (node->op != nullptr && node->op->op == "-") result = -result;
}

void Executor::visit(IdExp *node) {
#ifdef DEBUG
    cerr << "Exec: IdExp id=" << node->id << "\n";
#endif
    if (node == nullptr) return;

    // 查变量表。若未定义，假设值为 0，并在 stderr 报告位置
    if (varDefined.find(node->id) == varDefined.end()) {
        Pos *p = node->getPos();
        cerr << "Warning: variable '" << node->id << "' used before definition at line " << p->sline << ", column " << p->scolumn << endl;
        result = 0;
        // 将其标记为已定义（值为 0），避免后续重复报告
        varTable[node->id] = 0;
        varDefined[node->id] = true;
    } else {
        result = varTable[node->id];
    }
}

void Executor::visit(IntExp *node) {
#ifdef DEBUG
    cerr << "Exec: IntExp val=" << node->val << "\n";
#endif
    if (node == nullptr) return;
    result = node->val;
}

void Executor::visit(OpExp *node) {
    // OpExp 不会被直接访问，它作为 BinaryOp/UnaryOp 的成员被间接使用
    (void)node;
}
