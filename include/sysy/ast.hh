#pragma once

#include "lexer.hh"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace sysy {

enum class NodeKind {
    CompUnit,
    ConstDecl,
    VarDecl,
    ConstDef,
    VarDef,
    FuncDef,
    FuncParam,
    ArrayDim,
    Block,
    AssignStmt,
    ExprStmt,
    IfStmt,
    WhileStmt,
    BreakStmt,
    ContinueStmt,
    ReturnStmt,
    InitList,
    BinaryExpr,
    UnaryExpr,
    CallExpr,
    LVal,
    Number,
    StringLiteral,
    Identifier,
    Empty
};

struct Node {
    NodeKind kind = NodeKind::Empty;
    SourceLocation loc;
    std::string text;
    std::vector<std::unique_ptr<Node>> children;

    Node() = default;
    Node(NodeKind k, SourceLocation l, std::string t = {})
        : kind(k), loc(l), text(std::move(t)) {}

    Node *add(std::unique_ptr<Node> child) {
        children.push_back(std::move(child));
        return children.back().get();
    }
};

using NodePtr = std::unique_ptr<Node>;

std::string nodeKindName(NodeKind kind);
void dumpAst(const Node &node, std::string &out, int indent = 0);

} // namespace sysy
