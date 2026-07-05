#include "lower_tree.hh"

#include "temp.hh"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace sysy {

namespace {

std::string firstWord(const std::string &text) {
    std::size_t pos = text.find(' ');
    return pos == std::string::npos ? text : text.substr(0, pos);
}

std::string secondWord(const std::string &text) {
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

bool isFloatText(const std::string &text) {
    return text.find('.') != std::string::npos || text.find('e') != std::string::npos ||
           text.find('E') != std::string::npos || text.find('p') != std::string::npos ||
           text.find('P') != std::string::npos;
}

int parseIntLiteral(const std::string &text) {
    char *end = nullptr;
    long value = std::strtol(text.c_str(), &end, 0);
    return static_cast<int>(value);
}

int constIntValue(const Node &node) {
    switch (node.kind) {
    case NodeKind::Number:
        if (isFloatText(node.text)) {
            throw LoweringError(node.loc, "native backend does not support float constants yet");
        }
        return parseIntLiteral(node.text);
    case NodeKind::UnaryExpr: {
        int value = constIntValue(*node.children.at(0));
        if (node.text == "-") {
            return -value;
        }
        if (node.text == "+") {
            return value;
        }
        if (node.text == "!") {
            return value == 0 ? 1 : 0;
        }
        break;
    }
    case NodeKind::BinaryExpr: {
        int lhs = constIntValue(*node.children.at(0));
        int rhs = constIntValue(*node.children.at(1));
        if (node.text == "+") return lhs + rhs;
        if (node.text == "-") return lhs - rhs;
        if (node.text == "*") return lhs * rhs;
        if (node.text == "/") return rhs == 0 ? 0 : lhs / rhs;
        if (node.text == "%") return rhs == 0 ? 0 : lhs % rhs;
        if (node.text == "==") return lhs == rhs;
        if (node.text == "!=") return lhs != rhs;
        if (node.text == "<") return lhs < rhs;
        if (node.text == ">") return lhs > rhs;
        if (node.text == "<=") return lhs <= rhs;
        if (node.text == ">=") return lhs >= rhs;
        if (node.text == "&&") return (lhs != 0) && (rhs != 0);
        if (node.text == "||") return (lhs != 0) || (rhs != 0);
        break;
    }
    default:
        break;
    }
    throw LoweringError(node.loc, "native backend only supports constant scalar initializers for globals");
}

bool isRuntimeFunction(const std::string &name) {
    static const std::set<std::string> runtime = {
        "getint", "getch", "getarray", "putint", "putch", "putarray",
        "starttime", "stoptime", "_sysy_starttime", "_sysy_stoptime"
    };
    return runtime.count(name) != 0;
}

struct Symbol {
    tree::Temp *temp = nullptr;
    bool global = false;
    std::string label;
    std::vector<int> dims;
};

std::string globalLabel(const std::string &name) {
    return "__sysy_global_" + name;
}

int dimProduct(const std::vector<int> &dims) {
    if (dims.empty()) {
        return 1;
    }
    return std::accumulate(dims.begin(), dims.end(), 1, [](int acc, int dim) {
        return acc * dim;
    });
}

bool isArraySymbol(const Symbol &sym) {
    return !sym.dims.empty();
}

std::size_t dimSpan(const std::vector<int> &dims, std::size_t level) {
    std::size_t span = 1;
    for (std::size_t i = level; i < dims.size(); ++i) {
        span *= static_cast<std::size_t>(dims[i]);
    }
    return span;
}

std::size_t initListChildLevel(const std::vector<int> &dims,
                               std::size_t currentLevel,
                               std::size_t flatPos) {
    std::size_t level = std::min(currentLevel + 1, dims.size());
    while (level < dims.size()) {
        std::size_t span = dimSpan(dims, level);
        if (span == 0 || flatPos % span == 0) {
            break;
        }
        ++level;
    }
    return level;
}

template <typename Value, typename MakeScalar>
void fillArrayInitializerList(const Node &node, const std::vector<int> &dims,
                              std::size_t level, std::size_t begin, std::size_t end,
                              std::vector<Value> &values, MakeScalar makeScalar) {
    if (node.kind != NodeKind::InitList) {
        if (begin >= end) {
            throw LoweringError(node.loc, "too many array initializer elements for native backend");
        }
        values[begin] = makeScalar(node);
        return;
    }

    std::size_t pos = begin;
    for (const auto &child : node.children) {
        if (pos >= end) {
            throw LoweringError(child->loc, "too many array initializer elements for native backend");
        }
        if (child->kind == NodeKind::InitList) {
            std::size_t childLevel = initListChildLevel(dims, level, pos);
            std::size_t childSpan = dimSpan(dims, childLevel);
            if (pos + childSpan > end) {
                throw LoweringError(child->loc, "too many array initializer elements for native backend");
            }
            fillArrayInitializerList(*child, dims, childLevel, pos, pos + childSpan, values, makeScalar);
            pos += childSpan;
        } else {
            values[pos] = makeScalar(*child);
            ++pos;
        }
    }
}

template <typename Value, typename MakeScalar>
void fillArrayInitializer(const Node &node, const std::vector<int> &dims,
                          std::vector<Value> &values, MakeScalar makeScalar) {
    if (values.empty()) {
        return;
    }
    if (node.kind == NodeKind::InitList) {
        fillArrayInitializerList(node, dims, 0, 0, values.size(), values, makeScalar);
    } else {
        values[0] = makeScalar(node);
    }
}

class Lowerer {
public:
    tree::Program *lower(const Node &root) {
        if (root.kind != NodeKind::CompUnit) {
            throw LoweringError(root.loc, "expected compilation unit");
        }

        collectGlobals(root);

        auto *funcs = new std::vector<tree::FuncDecl *>();
        for (const auto &child : root.children) {
            if (child->kind == NodeKind::FuncDef) {
                funcs->push_back(lowerFunction(*child));
            }
        }
        return new tree::Program(funcs);
    }

private:
    std::map<std::string, Symbol> globalSymbols_;
    std::map<std::string, int> globalInitializers_;
    std::vector<std::unordered_map<std::string, Symbol>> scopes_;
    tree::Temp_map temps_;
    std::vector<tree::Label *> breakLabels_;
    std::vector<tree::Label *> continueLabels_;

    tree::Temp *newTemp() { return temps_.newtemp(); }
    tree::Label *newLabel() { return temps_.newlabel(); }

    tree::Exp *tempExp(tree::Temp *temp) {
        return new tree::TempExp(tree::Type::INT, new tree::Temp(temp->num));
    }

    tree::Exp *zero() { return new tree::Const(0); }

    void pushScope() { scopes_.push_back({}); }

    void popScope() { scopes_.pop_back(); }

    void declareLocal(const std::string &name, tree::Temp *temp, SourceLocation loc) {
        if (scopes_.empty()) {
            throw LoweringError(loc, "internal lowering scope error");
        }
        scopes_.back()[name] = Symbol{temp, false, {}, {}};
    }

    void declareLocalArray(const std::string &name, tree::Temp *temp,
                           std::vector<int> dims, SourceLocation loc) {
        if (scopes_.empty()) {
            throw LoweringError(loc, "internal lowering scope error");
        }
        scopes_.back()[name] = Symbol{temp, false, {}, std::move(dims)};
    }

    Symbol lookup(const std::string &name, SourceLocation loc) const {
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                return found->second;
            }
        }
        auto global = globalSymbols_.find(name);
        if (global != globalSymbols_.end()) {
            return global->second;
        }
        throw LoweringError(loc, "unknown lowered symbol '" + name + "'");
    }

    void collectGlobals(const Node &root) {
        for (const auto &child : root.children) {
            if (child->kind != NodeKind::ConstDecl && child->kind != NodeKind::VarDecl) {
                continue;
            }
            if (child->text != "int") {
                throw LoweringError(child->loc, "native backend does not support float globals yet");
            }
            for (const auto &def : child->children) {
                std::vector<int> dims = arrayDims(*def);
                globalSymbols_[def->text] = Symbol{nullptr, true, globalLabel(def->text), dims};
                if (dims.empty()) {
                    int init = 0;
                    if (!def->children.empty()) {
                        init = constIntValue(*def->children.back());
                    }
                    globalInitializers_[def->text] = init;
                }
            }
        }
    }

    std::vector<int> arrayDims(const Node &def) {
        std::vector<int> dims;
        for (const auto &child : def.children) {
            if (child->kind != NodeKind::ArrayDim) {
                continue;
            }
            if (child->children.empty()) {
                throw LoweringError(child->loc, "native backend does not support omitted array dimensions yet");
            }
            int dim = constIntValue(*child->children.at(0));
            if (dim <= 0) {
                throw LoweringError(child->loc, "array dimension must be positive for native backend");
            }
            dims.push_back(dim);
        }
        return dims;
    }

    const Node *initializerNode(const Node &def) const {
        for (const auto &child : def.children) {
            if (child->kind != NodeKind::ArrayDim) {
                return child.get();
            }
        }
        return nullptr;
    }

    std::vector<tree::Exp *> arrayInitializerExprs(const Node &def, const std::vector<int> &dims) {
        std::vector<tree::Exp *> values(static_cast<std::size_t>(dimProduct(dims)), nullptr);
        const Node *init = initializerNode(def);
        if (init == nullptr) {
            for (auto *&value : values) {
                value = zero();
            }
            return values;
        }
        fillArrayInitializer(*init, dims, values, [this](const Node &scalar) {
            return lowerExpr(scalar);
        });
        for (auto *&value : values) {
            if (value == nullptr) {
                value = zero();
            }
        }
        return values;
    }

    tree::FuncDecl *lowerFunction(const Node &node) {
        std::string ret = firstWord(node.text);
        std::string name = secondWord(node.text);
        if (ret == "float") {
            throw LoweringError(node.loc, "native backend does not support float functions yet");
        }

        pushScope();
        auto *params = new std::vector<tree::Temp *>();
        auto *stms = new std::vector<tree::Stm *>();
        stms->push_back(new tree::LabelStm(newLabel()));

        for (const auto &child : node.children) {
            if (child->kind != NodeKind::FuncParam) {
                continue;
            }
            if (firstWord(child->text) != "int") {
                throw LoweringError(child->loc, "native backend does not support float parameters yet");
            }
            auto *param = newTemp();
            params->push_back(param);
            std::vector<int> dims;
            if (!child->children.empty()) {
                for (const auto &dim : child->children) {
                    if (dim->kind != NodeKind::ArrayDim) {
                        continue;
                    }
                    dims.push_back(dim->children.empty() ? -1 : constIntValue(*dim->children.at(0)));
                }
            }
            if (dims.empty()) {
                declareLocal(secondWord(child->text), param, child->loc);
            } else {
                declareLocalArray(secondWord(child->text), param, std::move(dims), child->loc);
            }
        }

        for (const auto &child : node.children) {
            if (child->kind == NodeKind::Block) {
                lowerBlock(*child, stms, false);
            }
        }

        if (blockFallsThrough(stms)) {
            stms->push_back(new tree::Return(zero()));
        }
        popScope();

        return new tree::FuncDecl(name, params, new tree::Seq(stms),
                                  ret == "void" ? tree::Type::INT : tree::Type::INT,
                                  temps_.next_temp - 1, temps_.next_label - 1);
    }

    bool blockFallsThrough(const std::vector<tree::Stm *> *stms) const {
        if (stms == nullptr || stms->empty()) {
            return true;
        }
        tree::Stm *last = stms->back();
        if (last == nullptr) {
            return true;
        }
        return last->getTreeKind() != tree::Kind::RETURN && last->getTreeKind() != tree::Kind::JUMP;
    }

    bool lowerBlock(const Node &node, std::vector<tree::Stm *> *stms, bool scoped) {
        if (scoped) {
            pushScope();
        }
        bool fallsThrough = true;
        for (const auto &child : node.children) {
            if (!fallsThrough) {
                break;
            }
            if (child->kind == NodeKind::ConstDecl || child->kind == NodeKind::VarDecl) {
                lowerDecl(*child, stms);
                fallsThrough = true;
            } else {
                fallsThrough = lowerStmt(*child, stms);
            }
        }
        if (scoped) {
            popScope();
        }
        return fallsThrough;
    }

    void lowerDecl(const Node &node, std::vector<tree::Stm *> *stms) {
        if (node.text != "int") {
            throw LoweringError(node.loc, "native backend does not support float locals yet");
        }
        for (const auto &def : node.children) {
            std::vector<int> dims = arrayDims(*def);
            auto *temp = newTemp();
            if (!dims.empty()) {
                declareLocalArray(def->text, temp, dims, def->loc);
                int totalBytes = dimProduct(dims) * 4;
                auto *mallocArgs = new std::vector<tree::Exp *>({new tree::Const(totalBytes)});
                stms->push_back(new tree::Move(new tree::TempExp(tree::Type::PTR, new tree::Temp(temp->num)),
                                               new tree::ExtCall(tree::Type::PTR, "malloc", mallocArgs)));
                std::vector<tree::Exp *> values = arrayInitializerExprs(*def, dims);
                for (std::size_t i = 0; i < values.size(); ++i) {
                    stms->push_back(new tree::Move(
                        new tree::Mem(tree::Type::INT, arrayElementAddress(Symbol{temp, false, {}, dims}, i)),
                        values[i]));
                }
                continue;
            }
            declareLocal(def->text, temp, def->loc);
            tree::Exp *init = zero();
            if (!def->children.empty()) {
                init = lowerExpr(*def->children.back());
            }
            stms->push_back(new tree::Move(tempExp(temp), init));
        }
    }

    bool lowerStmt(const Node &node, std::vector<tree::Stm *> *stms) {
        switch (node.kind) {
        case NodeKind::Block:
            return lowerBlock(node, stms, true);
        case NodeKind::AssignStmt: {
            auto *dst = lowerLValue(*node.children.at(0));
            stms->push_back(new tree::Move(dst, lowerExpr(*node.children.at(1))));
            return true;
        }
        case NodeKind::ExprStmt:
            if (!node.children.empty()) {
                stms->push_back(new tree::ExpStm(lowerExpr(*node.children.at(0))));
            }
            return true;
        case NodeKind::IfStmt:
            return lowerIf(node, stms);
        case NodeKind::WhileStmt:
            lowerWhile(node, stms);
            return true;
        case NodeKind::BreakStmt:
            if (breakLabels_.empty()) {
                throw LoweringError(node.loc, "break outside loop");
            }
            stms->push_back(new tree::Jump(breakLabels_.back()));
            return false;
        case NodeKind::ContinueStmt:
            if (continueLabels_.empty()) {
                throw LoweringError(node.loc, "continue outside loop");
            }
            stms->push_back(new tree::Jump(continueLabels_.back()));
            return false;
        case NodeKind::ReturnStmt:
            if (node.children.empty()) {
                stms->push_back(new tree::Return(zero()));
            } else {
                stms->push_back(new tree::Return(lowerExpr(*node.children.at(0))));
            }
            return false;
        default:
            throw LoweringError(node.loc, "unsupported statement in native backend");
        }
    }

    bool lowerIf(const Node &node, std::vector<tree::Stm *> *stms) {
        auto *trueLabel = newLabel();
        auto *falseLabel = newLabel();
        auto *doneLabel = newLabel();
        emitCond(*node.children.at(0), trueLabel, falseLabel, stms);
        stms->push_back(new tree::LabelStm(trueLabel));
        bool thenFallsThrough = lowerStmt(*node.children.at(1), stms);
        if (thenFallsThrough) {
            stms->push_back(new tree::Jump(doneLabel));
        }
        stms->push_back(new tree::LabelStm(falseLabel));
        bool elseFallsThrough = true;
        if (node.children.size() > 2) {
            elseFallsThrough = lowerStmt(*node.children.at(2), stms);
        }
        if (elseFallsThrough) {
            stms->push_back(new tree::Jump(doneLabel));
        }
        stms->push_back(new tree::LabelStm(doneLabel));
        return thenFallsThrough || elseFallsThrough;
    }

    void lowerWhile(const Node &node, std::vector<tree::Stm *> *stms) {
        auto *testLabel = newLabel();
        auto *bodyLabel = newLabel();
        auto *doneLabel = newLabel();
        stms->push_back(new tree::LabelStm(testLabel));
        emitCond(*node.children.at(0), bodyLabel, doneLabel, stms);
        stms->push_back(new tree::LabelStm(bodyLabel));
        continueLabels_.push_back(testLabel);
        breakLabels_.push_back(doneLabel);
        bool bodyFallsThrough = lowerStmt(*node.children.at(1), stms);
        breakLabels_.pop_back();
        continueLabels_.pop_back();
        if (bodyFallsThrough) {
            stms->push_back(new tree::Jump(testLabel));
        }
        stms->push_back(new tree::LabelStm(doneLabel));
    }

    tree::Exp *lowerLValue(const Node &node) {
        Symbol sym = lookup(node.text, node.loc);
        if (isArraySymbol(sym)) {
            if (node.children.empty()) {
                return arrayBase(sym);
            }
            if (node.children.size() > sym.dims.size()) {
                throw LoweringError(node.loc, "too many array indices for native backend");
            }
            tree::Exp *offset = linearizedIndex(sym, node);
            tree::Exp *addr = new tree::Binop(tree::Type::PTR, "+", arrayBase(sym),
                                              new tree::Binop(tree::Type::INT, "*", offset, new tree::Const(4)));
            if (node.children.size() == sym.dims.size()) {
                return new tree::Mem(tree::Type::INT, addr);
            }
            return addr;
        }
        if (!node.children.empty()) {
            throw LoweringError(node.loc, "cannot index scalar in native backend");
        }
        if (sym.global) {
            return new tree::Mem(tree::Type::INT, new tree::Name(new tree::String_Label(sym.label)));
        }
        return tempExp(sym.temp);
    }

    tree::Exp *arrayBase(const Symbol &sym) {
        if (sym.global) {
            return new tree::Name(new tree::String_Label(sym.label));
        }
        return new tree::TempExp(tree::Type::PTR, new tree::Temp(sym.temp->num));
    }

    tree::Exp *arrayElementAddress(const Symbol &sym, std::size_t flatIndex) {
        return new tree::Binop(tree::Type::PTR, "+", arrayBase(sym),
                               new tree::Const(static_cast<int>(flatIndex) * 4));
    }

    tree::Exp *linearizedIndex(const Symbol &sym, const Node &lval) {
        tree::Exp *result = zero();
        for (std::size_t i = 0; i < lval.children.size(); ++i) {
            int stride = 1;
            for (std::size_t j = i + 1; j < sym.dims.size(); ++j) {
                if (sym.dims[j] <= 0) {
                    throw LoweringError(lval.loc, "native backend needs known non-first array dimensions");
                }
                stride *= sym.dims[j];
            }
            tree::Exp *term = lowerExpr(*lval.children.at(i));
            if (stride != 1) {
                term = new tree::Binop(tree::Type::INT, "*", term, new tree::Const(stride));
            }
            result = new tree::Binop(tree::Type::INT, "+", result, term);
        }
        return result;
    }

    tree::Exp *lowerExpr(const Node &node) {
        switch (node.kind) {
        case NodeKind::Number:
            if (isFloatText(node.text)) {
                throw LoweringError(node.loc, "native backend does not support float literals yet");
            }
            return new tree::Const(parseIntLiteral(node.text));
        case NodeKind::LVal:
            return lowerLValue(node);
        case NodeKind::UnaryExpr:
            return lowerUnary(node);
        case NodeKind::BinaryExpr:
            return lowerBinary(node);
        case NodeKind::CallExpr:
            return lowerCall(node);
        case NodeKind::StringLiteral:
            throw LoweringError(node.loc, "native backend does not support string literals yet");
        default:
            throw LoweringError(node.loc, "unsupported expression in native backend");
        }
    }

    tree::Exp *lowerUnary(const Node &node) {
        tree::Exp *operand = lowerExpr(*node.children.at(0));
        if (node.text == "+") {
            return operand;
        }
        if (node.text == "-") {
            return new tree::Binop(tree::Type::INT, "-", zero(), operand);
        }
        if (node.text == "!") {
            auto *result = newTemp();
            auto *trueLabel = newLabel();
            auto *falseLabel = newLabel();
            auto *doneLabel = newLabel();
            auto *stms = new std::vector<tree::Stm *>();
            stms->push_back(new tree::Cjump("==", operand, zero(), trueLabel, falseLabel));
            stms->push_back(new tree::LabelStm(trueLabel));
            stms->push_back(new tree::Move(tempExp(result), new tree::Const(1)));
            stms->push_back(new tree::Jump(doneLabel));
            stms->push_back(new tree::LabelStm(falseLabel));
            stms->push_back(new tree::Move(tempExp(result), zero()));
            stms->push_back(new tree::Jump(doneLabel));
            stms->push_back(new tree::LabelStm(doneLabel));
            return new tree::Eseq(tree::Type::INT, new tree::Seq(stms), tempExp(result));
        }
        throw LoweringError(node.loc, "unsupported unary operator");
    }

    tree::Exp *lowerBinary(const Node &node) {
        const std::string &op = node.text;
        if (op == "&&" || op == "||") {
            return lowerLogical(node);
        }
        if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") {
            return lowerComparison(node);
        }
        if (op == "%") {
            auto *lhs = lowerExpr(*node.children.at(0));
            auto *rhs = lowerExpr(*node.children.at(1));
            auto *quot = new tree::Binop(tree::Type::INT, "/", lhs, rhs);
            auto *prod = new tree::Binop(tree::Type::INT, "*", quot, lowerExpr(*node.children.at(1)));
            return new tree::Binop(tree::Type::INT, "-", lowerExpr(*node.children.at(0)), prod);
        }
        return new tree::Binop(tree::Type::INT, op, lowerExpr(*node.children.at(0)),
                               lowerExpr(*node.children.at(1)));
    }

    tree::Exp *lowerComparison(const Node &node) {
        auto *result = newTemp();
        auto *trueLabel = newLabel();
        auto *falseLabel = newLabel();
        auto *doneLabel = newLabel();
        auto *stms = new std::vector<tree::Stm *>();
        stms->push_back(new tree::Cjump(node.text, lowerExpr(*node.children.at(0)),
                                        lowerExpr(*node.children.at(1)), trueLabel, falseLabel));
        stms->push_back(new tree::LabelStm(trueLabel));
        stms->push_back(new tree::Move(tempExp(result), new tree::Const(1)));
        stms->push_back(new tree::Jump(doneLabel));
        stms->push_back(new tree::LabelStm(falseLabel));
        stms->push_back(new tree::Move(tempExp(result), zero()));
        stms->push_back(new tree::Jump(doneLabel));
        stms->push_back(new tree::LabelStm(doneLabel));
        return new tree::Eseq(tree::Type::INT, new tree::Seq(stms), tempExp(result));
    }

    tree::Exp *lowerLogical(const Node &node) {
        auto *result = newTemp();
        auto *trueLabel = newLabel();
        auto *falseLabel = newLabel();
        auto *doneLabel = newLabel();
        auto *stms = new std::vector<tree::Stm *>();
        emitCond(node, trueLabel, falseLabel, stms);
        stms->push_back(new tree::LabelStm(trueLabel));
        stms->push_back(new tree::Move(tempExp(result), new tree::Const(1)));
        stms->push_back(new tree::Jump(doneLabel));
        stms->push_back(new tree::LabelStm(falseLabel));
        stms->push_back(new tree::Move(tempExp(result), zero()));
        stms->push_back(new tree::Jump(doneLabel));
        stms->push_back(new tree::LabelStm(doneLabel));
        return new tree::Eseq(tree::Type::INT, new tree::Seq(stms), tempExp(result));
    }

    tree::Exp *lowerCall(const Node &node) {
        std::string name = node.text;
        if (name == "starttime") {
            name = "_sysy_starttime";
        } else if (name == "stoptime") {
            name = "_sysy_stoptime";
        }
        if (!isRuntimeFunction(name) && name != "putf") {
            auto *args = new std::vector<tree::Exp *>();
            for (const auto &child : node.children) {
                args->push_back(lowerExpr(*child));
            }
            return new tree::ExtCall(tree::Type::INT, name, args);
        }
        if (name == "putf") {
            throw LoweringError(node.loc, "native backend does not support putf/string literals yet");
        }
        if (name == "getarray" || name == "putarray") {
            throw LoweringError(node.loc, "native backend does not support array runtime calls yet");
        }
        auto *args = new std::vector<tree::Exp *>();
        for (const auto &child : node.children) {
            args->push_back(lowerExpr(*child));
        }
        return new tree::ExtCall(tree::Type::INT, name, args);
    }

    void emitCond(const Node &node, tree::Label *trueLabel, tree::Label *falseLabel,
                  std::vector<tree::Stm *> *stms) {
        if (node.kind == NodeKind::BinaryExpr && node.text == "&&") {
            auto *mid = newLabel();
            emitCond(*node.children.at(0), mid, falseLabel, stms);
            stms->push_back(new tree::LabelStm(mid));
            emitCond(*node.children.at(1), trueLabel, falseLabel, stms);
            return;
        }
        if (node.kind == NodeKind::BinaryExpr && node.text == "||") {
            auto *mid = newLabel();
            emitCond(*node.children.at(0), trueLabel, mid, stms);
            stms->push_back(new tree::LabelStm(mid));
            emitCond(*node.children.at(1), trueLabel, falseLabel, stms);
            return;
        }
        if (node.kind == NodeKind::BinaryExpr &&
            (node.text == "==" || node.text == "!=" || node.text == "<" || node.text == ">" ||
             node.text == "<=" || node.text == ">=")) {
            stms->push_back(new tree::Cjump(node.text, lowerExpr(*node.children.at(0)),
                                            lowerExpr(*node.children.at(1)), trueLabel, falseLabel));
            return;
        }
        if (node.kind == NodeKind::UnaryExpr && node.text == "!") {
            emitCond(*node.children.at(0), falseLabel, trueLabel, stms);
            return;
        }
        stms->push_back(new tree::Cjump("!=", lowerExpr(node), zero(), trueLabel, falseLabel));
    }
};

} // namespace

LoweringError::LoweringError(SourceLocation loc, const std::string &message)
    : std::runtime_error(message), loc_(loc) {}

tree::Program *lowerToTree(const Node &root) {
    Lowerer lowerer;
    return lowerer.lower(root);
}

std::string emitGlobalDataSection(const Node &root) {
    Lowerer lowerer;
    std::string out;
    for (const auto &child : root.children) {
        if (child->kind != NodeKind::ConstDecl && child->kind != NodeKind::VarDecl) {
            continue;
        }
        if (child->text != "int") {
            throw LoweringError(child->loc, "native backend does not support float globals yet");
        }
        if (out.empty()) {
            out += "\n.section .data\n.balign 4\n";
        }
        for (const auto &def : child->children) {
            std::vector<int> dims;
            for (const auto &defChild : def->children) {
                if (defChild->kind != NodeKind::ArrayDim) {
                    continue;
                }
                if (defChild->children.empty()) {
                    throw LoweringError(defChild->loc, "native backend does not support omitted global dimensions yet");
                }
                int dim = constIntValue(*defChild->children.at(0));
                if (dim <= 0) {
                    throw LoweringError(defChild->loc, "array dimension must be positive for native backend");
                }
                dims.push_back(dim);
            }
            out += ".global " + globalLabel(def->text) + "\n";
            out += globalLabel(def->text) + ":\n";
            if (dims.empty()) {
                int init = 0;
                if (!def->children.empty()) {
                    init = constIntValue(*def->children.back());
                }
                out += "    .word " + std::to_string(init) + "\n";
            } else {
                std::vector<int> values(dimProduct(dims), 0);
                const Node *init = nullptr;
                for (const auto &defChild : def->children) {
                    if (defChild->kind != NodeKind::ArrayDim) {
                        init = defChild.get();
                        break;
                    }
                }
                if (init != nullptr) {
                    fillArrayInitializer(*init, dims, values, [](const Node &scalar) {
                        return constIntValue(scalar);
                    });
                }
                for (int value : values) {
                    out += "    .word " + std::to_string(value) + "\n";
                }
            }
        }
    }
    return out;
}

} // namespace sysy
