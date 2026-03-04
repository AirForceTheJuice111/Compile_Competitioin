#ifndef _EXECUTOR_H
#define _EXECUTOR_H

#include "ASTheader.hh"
#include "FDMJAST.hh"
#include <map>
#include <string>

using namespace std;
using namespace fdmj;

int execute(Program *root);

// interpreter visitor：逐语句执行，维护变量表，返回 return 表达式的值
class Executor : public ASTVisitor {
  public:
    int result = 0; // 当前表达式的求值结果
    map<string, int> varTable;
    map<string, bool> varDefined;

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
