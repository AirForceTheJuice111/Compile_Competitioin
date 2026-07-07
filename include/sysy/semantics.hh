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

enum class BaseType {
    Int,
    Float,
    Void,
    Unknown
};

struct ValueType {
    BaseType base = BaseType::Unknown;
    int arrayDims = 0;

    bool isNumericScalar() const { return arrayDims == 0 && (base == BaseType::Int || base == BaseType::Float); }
};

struct Symbol {
    SymbolKind kind = SymbolKind::Var;
    ValueType type;
    std::vector<ValueType> params;
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
    ValueType analyzeExpr(const Node &node);
    ValueType makeType(const std::string &base, int arrayDims = 0) const;
    ValueType nodeDeclaredType(const Node &decl, const Node &def) const;
    static bool assignmentCompatible(ValueType lhs, ValueType rhs);
    static bool argumentCompatible(const std::string &callee, std::size_t index,
                                   ValueType expected, ValueType actual);
    static void validatePutfCall(const Node &node, const std::vector<ValueType> &args);
    static std::string typeName(ValueType type);

    static std::string firstWord(const std::string &text);
    static std::string secondWord(const std::string &text);

    std::vector<Scope> scopes_;
    std::string currentFuncReturn_;
    int loopDepth_ = 0;
    int mainCount_ = 0;
};

void checkSemantics(const Node &root);

} // namespace sysy
