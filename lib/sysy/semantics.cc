#include "semantics.hh"

#include <algorithm>
#include <sstream>

namespace sysy {

SemanticError::SemanticError(SourceLocation loc, const std::string &message)
    : std::runtime_error(message), loc_(loc) {}

void SemanticAnalyzer::pushScope() {
    scopes_.push_back({});
}

void SemanticAnalyzer::popScope() {
    scopes_.pop_back();
}

const Symbol *SemanticAnalyzer::lookupCurrent(const std::string &name) const {
    if (scopes_.empty()) {
        return nullptr;
    }
    auto it = scopes_.back().find(name);
    return it == scopes_.back().end() ? nullptr : &it->second;
}

const Symbol *SemanticAnalyzer::lookup(const std::string &name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto sym = it->find(name);
        if (sym != it->end()) {
            return &sym->second;
        }
    }
    return nullptr;
}

void SemanticAnalyzer::declare(const std::string &name, Symbol symbol) {
    if (lookupCurrent(name) != nullptr) {
        throw SemanticError(symbol.loc, "redefinition of '" + name + "'");
    }
    scopes_.back()[name] = std::move(symbol);
}

std::string SemanticAnalyzer::firstWord(const std::string &text) {
    std::size_t pos = text.find(' ');
    return pos == std::string::npos ? text : text.substr(0, pos);
}

std::string SemanticAnalyzer::secondWord(const std::string &text) {
    std::size_t pos = text.find(' ');
    if (pos == std::string::npos) {
        return {};
    }
    std::size_t begin = text.find_first_not_of(' ', pos);
    if (begin == std::string::npos) {
        return {};
    }
    std::size_t end = text.find(' ', begin);
    return text.substr(begin, end == std::string::npos ? std::string::npos : end - begin);
}

void SemanticAnalyzer::analyze(const Node &root) {
    if (root.kind != NodeKind::CompUnit) {
        throw SemanticError(root.loc, "semantic analysis expects CompUnit");
    }
    scopes_.clear();
    pushScope();
    const std::vector<std::pair<std::string, Symbol>> runtime = {
        {"getint", {SymbolKind::Func, makeType("int"), {}, root.loc}},
        {"getch", {SymbolKind::Func, makeType("int"), {}, root.loc}},
        {"getarray", {SymbolKind::Func, makeType("int"), {makeType("int", 1)}, root.loc}},
        {"getfloat", {SymbolKind::Func, makeType("float"), {}, root.loc}},
        {"getfarray", {SymbolKind::Func, makeType("int"), {makeType("float", 1)}, root.loc}},
        {"putint", {SymbolKind::Func, makeType("void"), {makeType("int")}, root.loc}},
        {"putch", {SymbolKind::Func, makeType("void"), {makeType("int")}, root.loc}},
        {"putarray", {SymbolKind::Func, makeType("void"), {makeType("int"), makeType("int", 1)}, root.loc}},
        {"putfloat", {SymbolKind::Func, makeType("void"), {makeType("float")}, root.loc}},
        {"putfarray", {SymbolKind::Func, makeType("void"), {makeType("int"), makeType("float", 1)}, root.loc}},
        {"putf", {SymbolKind::Func, makeType("void"), {}, root.loc}},
        {"_sysy_starttime", {SymbolKind::Func, makeType("void"), {makeType("int")}, root.loc}},
        {"_sysy_stoptime", {SymbolKind::Func, makeType("void"), {makeType("int")}, root.loc}},
    };
    for (const auto &fn : runtime) {
        declare(fn.first, fn.second);
    }

    mainCount_ = 0;
    for (const auto &child : root.children) {
        analyzeTopLevel(*child);
    }
    if (mainCount_ != 1) {
        throw SemanticError(root.loc, "program must define exactly one int main()");
    }
    popScope();
}

void SemanticAnalyzer::analyzeTopLevel(const Node &node) {
    switch (node.kind) {
    case NodeKind::ConstDecl:
    case NodeKind::VarDecl:
        analyzeDecl(node, true);
        break;
    case NodeKind::FuncDef:
        analyzeFunc(node);
        break;
    default:
        throw SemanticError(node.loc, "unexpected top-level node");
    }
}

void SemanticAnalyzer::analyzeDecl(const Node &node, bool global) {
    SymbolKind kind = node.kind == NodeKind::ConstDecl ? SymbolKind::Const : SymbolKind::Var;
    for (const auto &def : node.children) {
        ValueType declared = nodeDeclaredType(node, *def);
        declare(def->text, Symbol{kind, declared, {}, def->loc});
        for (const auto &child : def->children) {
            if (child->kind == NodeKind::ArrayDim) {
                if (child->children.empty()) {
                    throw SemanticError(child->loc, "array declaration dimension cannot be omitted");
                }
                ValueType dimType = analyzeExpr(*child->children.at(0));
                if (dimType.arrayDims != 0 || dimType.base != BaseType::Int) {
                    throw SemanticError(child->loc, "array dimension must be int scalar");
                }
            } else {
                analyzeExpr(*child);
            }
        }
    }
    (void)global;
}

void SemanticAnalyzer::analyzeFunc(const Node &node) {
    std::string ret = firstWord(node.text);
    std::string name = secondWord(node.text);
    if (name.empty()) {
        throw SemanticError(node.loc, "malformed function definition");
    }
    std::vector<std::pair<const Node *, ValueType>> params;
    for (const auto &child : node.children) {
        if (child->kind == NodeKind::FuncParam) {
            int dims = static_cast<int>(std::count_if(child->children.begin(), child->children.end(),
                                                      [](const auto &dim) {
                                                          return dim->kind == NodeKind::ArrayDim;
                                                      }));
            params.push_back({child.get(), makeType(firstWord(child->text), dims)});
        }
    }
    std::vector<ValueType> paramTypes;
    paramTypes.reserve(params.size());
    for (const auto &param : params) {
        paramTypes.push_back(param.second);
    }
    declare(name, Symbol{SymbolKind::Func, makeType(ret), paramTypes, node.loc});
    if (name == "main") {
        ++mainCount_;
        if (ret != "int") {
            throw SemanticError(node.loc, "main must return int");
        }
    }

    pushScope();
    currentFuncReturn_ = ret;
    for (const auto &child : node.children) {
        if (child->kind == NodeKind::FuncParam) {
            auto it = std::find_if(params.begin(), params.end(), [&](const auto &param) {
                return param.first == child.get();
            });
            ValueType paramType = it == params.end() ? makeType(firstWord(child->text)) : it->second;
            std::string paramName = secondWord(child->text);
            declare(paramName, Symbol{SymbolKind::Var, paramType, {}, child->loc});
            for (const auto &dim : child->children) {
                if (dim->kind != NodeKind::ArrayDim || dim->children.empty()) {
                    continue;
                }
                ValueType dimType = analyzeExpr(*dim->children.at(0));
                if (dimType.arrayDims != 0 || dimType.base != BaseType::Int) {
                    throw SemanticError(dim->loc, "array parameter dimension must be int scalar");
                }
            }
        } else if (child->kind == NodeKind::Block) {
            analyzeBlock(*child, false);
        }
    }
    currentFuncReturn_.clear();
    popScope();
}

void SemanticAnalyzer::analyzeBlock(const Node &node, bool createScope) {
    if (createScope) {
        pushScope();
    }
    for (const auto &child : node.children) {
        if (child->kind == NodeKind::ConstDecl || child->kind == NodeKind::VarDecl) {
            analyzeDecl(*child, false);
        } else {
            analyzeStmt(*child);
        }
    }
    if (createScope) {
        popScope();
    }
}

void SemanticAnalyzer::analyzeStmt(const Node &node) {
    switch (node.kind) {
    case NodeKind::Block:
        analyzeBlock(node, true);
        break;
    case NodeKind::AssignStmt: {
        const Node &lhs = *node.children.at(0);
        const Symbol *sym = lookup(lhs.text);
        if (sym == nullptr) {
            throw SemanticError(lhs.loc, "use of undeclared identifier '" + lhs.text + "'");
        }
        if (sym->kind == SymbolKind::Const) {
            throw SemanticError(lhs.loc, "cannot assign to const '" + lhs.text + "'");
        }
        ValueType lhsType = analyzeExpr(lhs);
        ValueType rhsType = analyzeExpr(*node.children.at(1));
        if (!assignmentCompatible(lhsType, rhsType)) {
            throw SemanticError(node.children.at(1)->loc, "cannot assign " + typeName(rhsType) +
                                                      " to " + typeName(lhsType));
        }
        break;
    }
    case NodeKind::ExprStmt:
        for (const auto &child : node.children) {
            analyzeExpr(*child);
        }
        break;
    case NodeKind::IfStmt:
        analyzeExpr(*node.children.at(0));
        analyzeStmt(*node.children.at(1));
        if (node.children.size() > 2) {
            analyzeStmt(*node.children.at(2));
        }
        break;
    case NodeKind::WhileStmt:
        analyzeExpr(*node.children.at(0));
        ++loopDepth_;
        analyzeStmt(*node.children.at(1));
        --loopDepth_;
        break;
    case NodeKind::BreakStmt:
        if (loopDepth_ == 0) {
            throw SemanticError(node.loc, "break must be inside while");
        }
        break;
    case NodeKind::ContinueStmt:
        if (loopDepth_ == 0) {
            throw SemanticError(node.loc, "continue must be inside while");
        }
        break;
    case NodeKind::ReturnStmt:
        if (currentFuncReturn_ == "void" && !node.children.empty()) {
            throw SemanticError(node.loc, "void function cannot return a value");
        }
        if (currentFuncReturn_ != "void" && node.children.empty()) {
            throw SemanticError(node.loc, "non-void function must return a value");
        }
        for (const auto &child : node.children) {
            analyzeExpr(*child);
        }
        break;
    default:
        throw SemanticError(node.loc, "unexpected statement node");
    }
}

ValueType SemanticAnalyzer::analyzeExpr(const Node &node) {
    switch (node.kind) {
    case NodeKind::Number:
        if (node.text.find('.') != std::string::npos || node.text.find('e') != std::string::npos ||
            node.text.find('E') != std::string::npos || node.text.find('p') != std::string::npos ||
            node.text.find('P') != std::string::npos) {
            return makeType("float");
        }
        return makeType("int");
    case NodeKind::StringLiteral:
        return ValueType{BaseType::Unknown, 1};
    case NodeKind::LVal: {
        const Symbol *sym = lookup(node.text);
        if (sym == nullptr) {
            throw SemanticError(node.loc, "use of undeclared identifier '" + node.text + "'");
        }
        if (sym->kind == SymbolKind::Func) {
            throw SemanticError(node.loc, "function name used as value '" + node.text + "'");
        }
        ValueType type = sym->type;
        for (const auto &child : node.children) {
            ValueType indexType = analyzeExpr(*child);
            if (indexType.arrayDims != 0 || indexType.base != BaseType::Int) {
                throw SemanticError(child->loc, "array index must be int scalar");
            }
        }
        if (static_cast<int>(node.children.size()) > type.arrayDims) {
            throw SemanticError(node.loc, "too many indices for '" + node.text + "'");
        }
        type.arrayDims -= static_cast<int>(node.children.size());
        return type;
    }
    case NodeKind::CallExpr: {
        const Symbol *sym = lookup(node.text);
        if (sym == nullptr || sym->kind != SymbolKind::Func) {
            throw SemanticError(node.loc, "call to undeclared function '" + node.text + "'");
        }
        if (node.text != "putf" && sym->params.size() != node.children.size()) {
            throw SemanticError(node.loc, "wrong number of arguments to '" + node.text + "'");
        }
        for (std::size_t i = 0; i < node.children.size(); ++i) {
            ValueType arg = analyzeExpr(*node.children[i]);
            if (node.text != "putf" && i < sym->params.size() &&
                !assignmentCompatible(sym->params[i], arg)) {
                throw SemanticError(node.children[i]->loc, "argument type mismatch: expected " +
                                                              typeName(sym->params[i]) + ", got " +
                                                              typeName(arg));
            }
        }
        return sym->type;
    }
    case NodeKind::UnaryExpr: {
        ValueType operand = analyzeExpr(*node.children.at(0));
        if (operand.arrayDims != 0) {
            throw SemanticError(node.loc, "unary operator requires scalar operand");
        }
        if (node.text == "!") {
            return makeType("int");
        }
        return operand;
    }
    case NodeKind::BinaryExpr: {
        ValueType lhs = analyzeExpr(*node.children.at(0));
        ValueType rhs = analyzeExpr(*node.children.at(1));
        if (lhs.arrayDims != 0 || rhs.arrayDims != 0) {
            throw SemanticError(node.loc, "binary operator requires scalar operands");
        }
        if (node.text == "%") {
            if (lhs.base != BaseType::Int || rhs.base != BaseType::Int) {
                throw SemanticError(node.loc, "modulo requires int operands");
            }
            return makeType("int");
        }
        if (node.text == "==" || node.text == "!=" || node.text == "<" || node.text == ">" ||
            node.text == "<=" || node.text == ">=" || node.text == "&&" || node.text == "||") {
            return makeType("int");
        }
        if (lhs.base == BaseType::Float || rhs.base == BaseType::Float) {
            return makeType("float");
        }
        return makeType("int");
    }
    case NodeKind::InitList:
    case NodeKind::ExprStmt:
    case NodeKind::ArrayDim:
        for (const auto &child : node.children) {
            analyzeExpr(*child);
        }
        return ValueType{BaseType::Unknown, 0};
    default:
        for (const auto &child : node.children) {
            analyzeExpr(*child);
        }
        return ValueType{BaseType::Unknown, 0};
    }
}

ValueType SemanticAnalyzer::makeType(const std::string &base, int arrayDims) const {
    if (base == "int") {
        return ValueType{BaseType::Int, arrayDims};
    }
    if (base == "float") {
        return ValueType{BaseType::Float, arrayDims};
    }
    if (base == "void") {
        return ValueType{BaseType::Void, arrayDims};
    }
    return ValueType{BaseType::Unknown, arrayDims};
}

ValueType SemanticAnalyzer::nodeDeclaredType(const Node &decl, const Node &def) const {
    int dims = 0;
    for (const auto &child : def.children) {
        if (child->kind == NodeKind::ArrayDim) {
            ++dims;
        }
    }
    return makeType(decl.text, dims);
}

bool SemanticAnalyzer::assignmentCompatible(ValueType lhs, ValueType rhs) {
    if (lhs.base == BaseType::Unknown || rhs.base == BaseType::Unknown) {
        return true;
    }
    if (lhs.arrayDims != rhs.arrayDims) {
        return false;
    }
    if (lhs.arrayDims > 0) {
        return lhs.base == rhs.base;
    }
    return lhs.base != BaseType::Void && rhs.base != BaseType::Void;
}

std::string SemanticAnalyzer::typeName(ValueType type) {
    std::string base = "unknown";
    if (type.base == BaseType::Int) {
        base = "int";
    } else if (type.base == BaseType::Float) {
        base = "float";
    } else if (type.base == BaseType::Void) {
        base = "void";
    }
    for (int i = 0; i < type.arrayDims; ++i) {
        base += "[]";
    }
    return base;
}

void checkSemantics(const Node &root) {
    SemanticAnalyzer analyzer;
    analyzer.analyze(root);
}

} // namespace sysy
