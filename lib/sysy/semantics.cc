#include "semantics.hh"

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
    const std::vector<std::pair<std::string, std::string>> runtime = {
        {"getint", "int"},       {"getch", "int"},       {"getarray", "int"},
        {"getfloat", "float"},   {"getfarray", "int"},   {"putint", "void"},
        {"putch", "void"},       {"putarray", "void"},   {"putfloat", "void"},
        {"putfarray", "void"},   {"putf", "void"},       {"_sysy_starttime", "void"},
        {"_sysy_stoptime", "void"},
    };
    for (const auto &fn : runtime) {
        declare(fn.first, Symbol{SymbolKind::Func, fn.second, root.loc});
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
        declare(def->text, Symbol{kind, node.text, def->loc});
        for (const auto &child : def->children) {
            analyzeExpr(*child);
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
    declare(name, Symbol{SymbolKind::Func, ret, node.loc});
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
            std::string paramType = firstWord(child->text);
            std::string paramName = secondWord(child->text);
            declare(paramName, Symbol{SymbolKind::Var, paramType, child->loc});
            for (const auto &dim : child->children) {
                analyzeExpr(*dim);
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
        for (const auto &child : node.children) {
            analyzeExpr(*child);
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

void SemanticAnalyzer::analyzeExpr(const Node &node) {
    switch (node.kind) {
    case NodeKind::Number:
    case NodeKind::StringLiteral:
        return;
    case NodeKind::LVal: {
        if (lookup(node.text) == nullptr) {
            throw SemanticError(node.loc, "use of undeclared identifier '" + node.text + "'");
        }
        for (const auto &child : node.children) {
            analyzeExpr(*child);
        }
        return;
    }
    case NodeKind::CallExpr: {
        const Symbol *sym = lookup(node.text);
        if (sym == nullptr || sym->kind != SymbolKind::Func) {
            throw SemanticError(node.loc, "call to undeclared function '" + node.text + "'");
        }
        for (const auto &child : node.children) {
            analyzeExpr(*child);
        }
        return;
    }
    case NodeKind::UnaryExpr:
    case NodeKind::BinaryExpr:
    case NodeKind::InitList:
    case NodeKind::ExprStmt:
        for (const auto &child : node.children) {
            analyzeExpr(*child);
        }
        return;
    default:
        for (const auto &child : node.children) {
            analyzeExpr(*child);
        }
        return;
    }
}

void checkSemantics(const Node &root) {
    SemanticAnalyzer analyzer;
    analyzer.analyze(root);
}

} // namespace sysy
