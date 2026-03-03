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
  if (root == nullptr)
    return nullptr;

  // 第一步：用 minusIntRewrite 将所有 UnaryOp(-, IntExp) 转换为 IntExp
  Program *rewritten = minusIntRewrite(root);
  if (rewritten == nullptr)
    return nullptr;

  // 第二步：常量折叠 —— 将两个常量的二元运算折叠为单个 IntExp
  ConstantPropagator v;
  rewritten->accept(v);
  return dynamic_cast<Program *>(v.newNode);
}

// 辅助函数：对节点列表逐个 visit，收集结果
template <typename T>
static vector<T *> *visitList(ConstantPropagator &v, vector<T *> *tl) {
  if (tl == nullptr || tl->size() == 0)
    return nullptr;
  vector<T *> *vt = new vector<T *>();
  for (T *x : *tl) {
    if (x == nullptr)
      continue;
    x->accept(v);
    if (v.newNode == nullptr)
      continue;
    vt->push_back(static_cast<T *>(v.newNode));
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
    newNode = nullptr;
    return;
  }
  MainMethod *m = nullptr;
  if (node->main != nullptr) {
    node->main->accept(*this);
    m = (newNode != nullptr) ? static_cast<MainMethod *>(newNode) : nullptr;
  }
  newNode = new Program(node->getPos()->clone(), m);
}

void ConstantPropagator::visit(MainMethod *node) {
#ifdef DEBUG
  cerr << "CP: MainMethod\n";
#endif
  if (node == nullptr) {
    newNode = nullptr;
    return;
  }
  StmList *sl = nullptr;
  if (node->sl != nullptr)
    sl = visitList<Stm>(*this, node->sl);
  newNode = new MainMethod(node->getPos()->clone(), sl);
}

void ConstantPropagator::visit(Assign *node) {
#ifdef DEBUG
  cerr << "CP: Assign\n";
#endif
  if (node == nullptr) {
    newNode = nullptr;
    return;
  }
  Exp *l = nullptr;
  if (node->left != nullptr) {
    node->left->accept(*this);
    l = static_cast<Exp *>(newNode);
  }
  Exp *r = nullptr;
  if (node->exp != nullptr) {
    node->exp->accept(*this);
    r = static_cast<Exp *>(newNode);
  }
  newNode = new Assign(node->getPos()->clone(), l, r);
}

void ConstantPropagator::visit(Return *node) {
#ifdef DEBUG
  cerr << "CP: Return\n";
#endif
  if (node == nullptr) {
    newNode = nullptr;
    return;
  }
  Exp *e = nullptr;
  if (node->exp != nullptr) {
    node->exp->accept(*this);
    e = static_cast<Exp *>(newNode);
  }
  newNode = new Return(node->getPos()->clone(), e);
}

void ConstantPropagator::visit(BinaryOp *node) {
#ifdef DEBUG
  cerr << "CP: BinaryOp\n";
#endif
  if (node == nullptr) {
    newNode = nullptr;
    return;
  }

  // 自底向上：先递归处理左右子树
  Exp *l = nullptr;
  if (node->left != nullptr) {
    node->left->accept(*this);
    l = static_cast<Exp *>(newNode);
  }
  Exp *r = nullptr;
  if (node->right != nullptr) {
    node->right->accept(*this);
    r = static_cast<Exp *>(newNode);
  }

  // 核心逻辑：若左右都是 IntExp，直接计算并折叠为一个 IntExp
  if (l != nullptr && r != nullptr &&
      l->getASTKind() == ASTKind::IntExp &&
      r->getASTKind() == ASTKind::IntExp) {
    int lval = static_cast<IntExp *>(l)->val;
    int rval = static_cast<IntExp *>(r)->val;
    string opStr = node->op->op;
    int res = 0;
    if (opStr == "+")
      res = lval + rval;
    else if (opStr == "-")
      res = lval - rval;
    else if (opStr == "*")
      res = lval * rval;
    else if (opStr == "/") {
      if (rval == 0) {
        cerr << "Error: Division by zero" << endl;
        newNode = new BinaryOp(node->getPos()->clone(), l, node->op->clone(), r);
        return;
      }
      res = lval / rval;
    } else {
      // 未知运算符，不折叠
      newNode = new BinaryOp(node->getPos()->clone(), l, node->op->clone(), r);
      return;
    }
    newNode = new IntExp(node->getPos()->clone(), res);
    return;
  }

  // 无法折叠，保留原结构
  newNode = new BinaryOp(node->getPos()->clone(), l, node->op->clone(), r);
}

void ConstantPropagator::visit(UnaryOp *node) {
#ifdef DEBUG
  cerr << "CP: UnaryOp\n";
#endif
  if (node == nullptr) {
    newNode = nullptr;
    return;
  }

  Exp *e = nullptr;
  if (node->exp != nullptr) {
    node->exp->accept(*this);
    e = static_cast<Exp *>(newNode);
  }

  // 若经过子树折叠后 UnaryOp(-, IntExp) 出现，也将其折叠
  if (e != nullptr && node->op != nullptr &&
      node->op->op == "-" && e->getASTKind() == ASTKind::IntExp) {
    int val = -(static_cast<IntExp *>(e)->val);
    newNode = new IntExp(node->getPos()->clone(), val);
    return;
  }

  OpExp *o = (node->op != nullptr) ? node->op->clone() : nullptr;
  newNode = new UnaryOp(node->getPos()->clone(), o, e);
}

void ConstantPropagator::visit(IdExp *node) {
  newNode = (node == nullptr) ? nullptr : node->clone();
}

void ConstantPropagator::visit(OpExp *node) {
  newNode = (node == nullptr) ? nullptr : node->clone();
}

void ConstantPropagator::visit(IntExp *node) {
  newNode = (node == nullptr) ? nullptr : node->clone();
}
