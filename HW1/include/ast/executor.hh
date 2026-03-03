#ifndef _EXECUTOR_H
#define _EXECUTOR_H

#include "ASTheader.hh"
#include "FDMJAST.hh"
#include <map>
#include <string>

using namespace std;
using namespace fdmj;

// 执行程序并返回最终 return 语句的值
int execute(Program *root);

// 解释执行 Visitor：逐语句执行，维护变量表，返回 return 表达式的值
class Executor : public ASTVisitor {
public:
  int result = 0;                  // 当前表达式的求值结果
  map<string, int> varTable;       // 变量名 → 当前值
  map<string, bool> varDefined;    // 变量名 → 是否已定义

  Executor() : result(0) {}
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
