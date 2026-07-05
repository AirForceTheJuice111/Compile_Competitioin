#pragma once

#include "ast.hh"

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace sysy {

enum class SymbolKind {
    Var,
    Const,
    Func
};

struct Symbol {
    SymbolKind kind = SymbolKind::Var;
    std::string type;
    SourceLocation loc;
};

class SemanticError : public std::runtime_error {
public:
    SemanticError(SourceLocation loc, const std::string &message);
    SourceLocation loc() const { return loc_; }

private:
    SourceLocation loc_;
};

class SemanticAnalyzer {
public:
    void analyze(const Node &root);

private:
    using Scope = std::unordered_map<std::string, Symbol>;

    void pushScope();
    void popScope();
    void declare(const std::string &name, Symbol symbol);
    const Symbol *lookup(const std::string &name) const;
    const Symbol *lookupCurrent(const std::string &name) const;

    void analyzeTopLevel(const Node &node);
    void analyzeDecl(const Node &node, bool global);
    void analyzeFunc(const Node &node);
    void analyzeBlock(const Node &node, bool createScope);
    void analyzeStmt(const Node &node);
    void analyzeExpr(const Node &node);

    static std::string firstWord(const std::string &text);
    static std::string secondWord(const std::string &text);

    std::vector<Scope> scopes_;
    std::string currentFuncReturn_;
    int loopDepth_ = 0;
    int mainCount_ = 0;
};

void checkSemantics(const Node &root);

} // namespace sysy
