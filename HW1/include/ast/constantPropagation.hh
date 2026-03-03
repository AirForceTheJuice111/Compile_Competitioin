#ifndef _CONSTANTPROPAGATION_H
#define _CONSTANTPROPAGATION_H

#include "ASTheader.hh"
#include "FDMJAST.hh"

using namespace std;
using namespace fdmj;

// 对程序进行常量折叠：先做 minusIntRewrite，再递归地将两个常量的二元运算折叠为常量
Program *constantPropagate(Program *root);

// 常量折叠 Visitor：遍历 AST，自底向上地将可计算的表达式替换为 IntExp 常量节点
class ConstantPropagator : public ASTVisitor {
public:
  AST *newNode = nullptr; // 访问后生成的新节点（克隆/折叠后的结果）
  ConstantPropagator() : newNode(nullptr) {}
  void visit(Program *node) override;
  void visit(MainMethod *node) override;
  void visit(Assign *node) override;
  void visit(Return *node) override;
  void visit(BinaryOp *node) override;
  void visit(UnaryOp *node) override;
  void visit(IdExp *node) override;
  void visit(OpExp *node) override;
  void visit(IntExp *node) override;
};

#endif
