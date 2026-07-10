#include "parallel_plan.hh"

#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace sysy {

namespace {

struct ArrayAccess {
    std::string base;
    const Node *lval = nullptr;
};

bool isScalarLVal(const Node &node) {
    return node.kind == NodeKind::LVal && node.children.empty();
}

bool isScalarDef(const Node &def) {
    if (def.kind != NodeKind::VarDef && def.kind != NodeKind::ConstDef) {
        return false;
    }
    for (const auto &child : def.children) {
        if (child->kind == NodeKind::ArrayDim) {
            return false;
        }
    }
    return true;
}

const Node *initializerOf(const Node &def) {
    for (const auto &child : def.children) {
        if (child->kind != NodeKind::ArrayDim) {
            return child.get();
        }
    }
    return nullptr;
}

bool exprContainsCall(const Node &node) {
    if (node.kind == NodeKind::CallExpr) {
        return true;
    }
    for (const auto &child : node.children) {
        if (exprContainsCall(*child)) {
            return true;
        }
    }
    return false;
}

int estimateExprCost(const Node &node) {
    switch (node.kind) {
    case NodeKind::Number:
    case NodeKind::StringLiteral:
    case NodeKind::Identifier:
        return 0;
    case NodeKind::LVal:
        return node.children.empty() ? 1 : 4 + static_cast<int>(node.children.size()) * 2;
    case NodeKind::CallExpr:
        return 64;
    case NodeKind::UnaryExpr:
        return 1 + estimateExprCost(*node.children.at(0));
    case NodeKind::BinaryExpr: {
        int opCost = (node.text == "*" || node.text == "/" || node.text == "%") ? 4 : 1;
        return opCost + estimateExprCost(*node.children.at(0)) +
               estimateExprCost(*node.children.at(1));
    }
    default: {
        int cost = 0;
        for (const auto &child : node.children) {
            cost += estimateExprCost(*child);
        }
        return cost;
    }
    }
}

int estimateStmtCost(const Node &node) {
    switch (node.kind) {
    case NodeKind::AssignStmt:
        return 2 + estimateExprCost(*node.children.at(0)) +
               estimateExprCost(*node.children.at(1));
    case NodeKind::VarDecl:
    case NodeKind::ConstDecl: {
        int cost = 1;
        for (const auto &def : node.children) {
            if (const Node *init = initializerOf(*def)) {
                cost += estimateExprCost(*init);
            }
        }
        return cost;
    }
    case NodeKind::IfStmt:
        return 8 + estimateExprCost(*node.children.at(0));
    case NodeKind::WhileStmt:
        return 64 + estimateExprCost(*node.children.at(0));
    default: {
        int cost = 1;
        for (const auto &child : node.children) {
            cost += estimateStmtCost(*child);
        }
        return cost;
    }
    }
}

bool intConstValue(const Node &node, int &value) {
    if (node.kind != NodeKind::Number) {
        return false;
    }
    const std::string &text = node.text;
    if (text.find('.') != std::string::npos || text.find('e') != std::string::npos ||
        text.find('E') != std::string::npos || text.find('p') != std::string::npos ||
        text.find('P') != std::string::npos) {
        return false;
    }
    char *end = nullptr;
    long parsed = std::strtol(text.c_str(), &end, 0);
    if (end == nullptr || *end != '\0') {
        return false;
    }
    value = static_cast<int>(parsed);
    return true;
}

bool affineCoeff(const Node &node, const std::string &var, int &coeff) {
    if (node.kind == NodeKind::Number) {
        coeff = 0;
        return true;
    }
    if (node.kind == NodeKind::LVal) {
        coeff = node.text == var ? 1 : 0;
        return true;
    }
    if (node.kind == NodeKind::UnaryExpr) {
        int inner = 0;
        if (!affineCoeff(*node.children.at(0), var, inner)) {
            return false;
        }
        if (node.text == "+") {
            coeff = inner;
            return true;
        }
        if (node.text == "-") {
            coeff = -inner;
            return true;
        }
        return false;
    }
    if (node.kind != NodeKind::BinaryExpr) {
        return false;
    }
    int left = 0;
    int right = 0;
    if (node.text == "+" || node.text == "-") {
        if (!affineCoeff(*node.children.at(0), var, left) ||
            !affineCoeff(*node.children.at(1), var, right)) {
            return false;
        }
        coeff = node.text == "+" ? left + right : left - right;
        return true;
    }
    if (node.text == "*") {
        int c = 0;
        if (intConstValue(*node.children.at(0), c) &&
            affineCoeff(*node.children.at(1), var, right)) {
            coeff = c * right;
            return true;
        }
        if (intConstValue(*node.children.at(1), c) &&
            affineCoeff(*node.children.at(0), var, left)) {
            coeff = c * left;
            return true;
        }
    }
    return false;
}

bool lvalHasAffineLoopIndex(const Node &lval, const std::string &var) {
    if (lval.kind != NodeKind::LVal || lval.children.empty()) {
        return false;
    }
    bool hasLoopIndex = false;
    for (const auto &index : lval.children) {
        int coeff = 0;
        if (!affineCoeff(*index, var, coeff)) {
            return false;
        }
        if (coeff != 0) {
            hasLoopIndex = true;
        }
    }
    return hasLoopIndex;
}

bool lvalFirstIndexIsPartitionedByLoopVar(const Node &lval, const std::string &var) {
    if (lval.kind != NodeKind::LVal || lval.children.empty()) {
        return false;
    }
    int coeff = 0;
    return affineCoeff(*lval.children.front(), var, coeff) && coeff != 0;
}

bool isIncrementOf(const Node &stmt, const std::string &var) {
    if (stmt.kind != NodeKind::AssignStmt || stmt.children.size() != 2) {
        return false;
    }
    const Node &lhs = *stmt.children.at(0);
    const Node &rhs = *stmt.children.at(1);
    if (!isScalarLVal(lhs) || lhs.text != var || rhs.kind != NodeKind::BinaryExpr ||
        rhs.text != "+") {
        return false;
    }
    int c = 0;
    return ((rhs.children.at(0)->kind == NodeKind::LVal &&
             rhs.children.at(0)->text == var &&
             intConstValue(*rhs.children.at(1), c) && c == 1) ||
            (rhs.children.at(1)->kind == NodeKind::LVal &&
             rhs.children.at(1)->text == var &&
             intConstValue(*rhs.children.at(0), c) && c == 1));
}

bool canonicalWhile(const Node &loop, const ParallelLoopInit &init, const Node *&endExpr,
                    std::vector<const Node *> &body) {
    if (!init.valid || loop.kind != NodeKind::WhileStmt || loop.children.size() != 2) {
        return false;
    }
    const Node &cond = *loop.children.at(0);
    if (cond.kind != NodeKind::BinaryExpr || cond.text != "<" ||
        cond.children.at(0)->kind != NodeKind::LVal ||
        cond.children.at(0)->text != init.var ||
        !cond.children.at(0)->children.empty()) {
        return false;
    }
    const Node &bodyNode = *loop.children.at(1);
    if (bodyNode.kind != NodeKind::Block || bodyNode.children.empty()) {
        return false;
    }
    const Node &last = *bodyNode.children.back();
    if (!isIncrementOf(last, init.var)) {
        return false;
    }
    endExpr = cond.children.at(1).get();
    body.clear();
    for (std::size_t i = 0; i + 1 < bodyNode.children.size(); ++i) {
        body.push_back(bodyNode.children[i].get());
    }
    return true;
}

void collectLocalScalars(const Node &node, std::unordered_set<std::string> &locals) {
    if (node.kind == NodeKind::VarDecl || node.kind == NodeKind::ConstDecl) {
        for (const auto &def : node.children) {
            if (isScalarDef(*def)) {
                locals.insert(def->text);
            }
        }
    }
    for (const auto &child : node.children) {
        collectLocalScalars(*child, locals);
    }
}

void addReduction(std::vector<ParallelReduction> &reductions, ParallelReduction info) {
    auto found = std::find_if(reductions.begin(), reductions.end(),
                              [&](const ParallelReduction &existing) {
                                  return existing.var == info.var;
                              });
    if (found == reductions.end()) {
        reductions.push_back(info);
    }
}

void collectArrayReads(const Node &node, std::vector<ArrayAccess> &reads) {
    if (node.kind == NodeKind::LVal && !node.children.empty()) {
        reads.push_back(ArrayAccess{node.text, &node});
    }
    for (const auto &child : node.children) {
        collectArrayReads(*child, reads);
    }
}

void collectArrayWrites(const Node &node, std::vector<ArrayAccess> &writes) {
    if (node.kind == NodeKind::AssignStmt && node.children.size() == 2) {
        const Node *lhs = node.children.at(0).get();
        if (lhs != nullptr && lhs->kind == NodeKind::LVal && !lhs->children.empty()) {
            writes.push_back(ArrayAccess{lhs->text, lhs});
        }
    }
    for (const auto &child : node.children) {
        collectArrayWrites(*child, writes);
    }
}

void noteCapture(ParallelLoopPlan &plan, const std::string &name, const std::string &type,
                 bool array, bool read, bool write) {
    auto found = std::find_if(plan.captures.begin(), plan.captures.end(),
                              [&](const ParallelCapture &capture) {
                                  return capture.name == name && capture.array == array;
                              });
    if (found == plan.captures.end()) {
        plan.captures.push_back(ParallelCapture{name, type, array, read, write});
        return;
    }
    if (found->type.empty()) {
        found->type = type;
    }
    found->read = found->read || read;
    found->write = found->write || write;
}

bool isReductionVar(const ParallelLoopPlan &plan, const std::string &name) {
    return std::any_of(plan.reductions.begin(), plan.reductions.end(),
                       [&](const ParallelReduction &reduction) {
                           return reduction.var == name;
                       });
}

void collectCapturesFromExpr(const Node &node, const std::string &loopVar,
                             const std::unordered_set<std::string> &locals,
                             const ParallelTypeLookup &lookupType,
                             ParallelLoopPlan &plan) {
    if (node.kind == NodeKind::LVal) {
        if (!node.children.empty()) {
            noteCapture(plan, node.text, lookupType ? lookupType(node.text) : "", true, true, false);
            for (const auto &index : node.children) {
                collectCapturesFromExpr(*index, loopVar, locals, lookupType, plan);
            }
            return;
        }
        if (node.text != loopVar && locals.find(node.text) == locals.end() &&
            !isReductionVar(plan, node.text)) {
            noteCapture(plan, node.text, lookupType ? lookupType(node.text) : "", false, true, false);
        }
        return;
    }
    for (const auto &child : node.children) {
        collectCapturesFromExpr(*child, loopVar, locals, lookupType, plan);
    }
}

void collectCapturesFromStmt(const Node &node, const std::string &loopVar,
                             const std::unordered_set<std::string> &locals,
                             const ParallelTypeLookup &lookupType,
                             ParallelLoopPlan &plan) {
    if (node.kind == NodeKind::LVal || node.kind == NodeKind::BinaryExpr ||
        node.kind == NodeKind::UnaryExpr || node.kind == NodeKind::CallExpr ||
        node.kind == NodeKind::InitList) {
        collectCapturesFromExpr(node, loopVar, locals, lookupType, plan);
        return;
    }
    if (node.kind == NodeKind::AssignStmt && node.children.size() == 2) {
        const Node &lhs = *node.children.at(0);
        if (lhs.kind == NodeKind::LVal && !lhs.children.empty()) {
            noteCapture(plan, lhs.text, lookupType ? lookupType(lhs.text) : "", true, false, true);
            for (const auto &index : lhs.children) {
                collectCapturesFromExpr(*index, loopVar, locals, lookupType, plan);
            }
        } else if (lhs.kind == NodeKind::LVal && lhs.text != loopVar &&
                   locals.find(lhs.text) == locals.end() && !isReductionVar(plan, lhs.text)) {
            noteCapture(plan, lhs.text, lookupType ? lookupType(lhs.text) : "", false, false, true);
        }
        collectCapturesFromExpr(*node.children.at(1), loopVar, locals, lookupType, plan);
        return;
    }
    if (node.kind == NodeKind::VarDecl || node.kind == NodeKind::ConstDecl) {
        for (const auto &def : node.children) {
            if (const Node *init = initializerOf(*def)) {
                collectCapturesFromExpr(*init, loopVar, locals, lookupType, plan);
            }
        }
        return;
    }
    for (const auto &child : node.children) {
        collectCapturesFromStmt(*child, loopVar, locals, lookupType, plan);
    }
}

bool sameArrayReadsArePartitioned(const Node &rhs, const std::string &writtenArray,
                                  const std::string &loopVar) {
    std::vector<ArrayAccess> reads;
    collectArrayReads(rhs, reads);
    for (const ArrayAccess &read : reads) {
        if (read.base != writtenArray) {
            continue;
        }
        if (read.lval == nullptr ||
            !lvalFirstIndexIsPartitionedByLoopVar(*read.lval, loopVar)) {
            return false;
        }
    }
    return true;
}

bool analyzeNode(const Node &node, const std::string &loopVar,
                 const std::unordered_set<std::string> &locals,
                 const ParallelTypeLookup &lookupType,
                 ParallelLoopPlan &plan) {
    if (exprContainsCall(node)) {
        plan.rejectReason = "call in loop body";
        return false;
    }
    switch (node.kind) {
    case NodeKind::BreakStmt:
    case NodeKind::ContinueStmt:
    case NodeKind::ReturnStmt:
        plan.rejectReason = "control transfer in loop body";
        return false;
    case NodeKind::VarDecl:
    case NodeKind::ConstDecl:
        for (const auto &def : node.children) {
            if (!isScalarDef(*def)) {
                plan.rejectReason = "non-scalar local declaration";
                return false;
            }
        }
        return true;
    case NodeKind::AssignStmt: {
        const Node &lhs = *node.children.at(0);
        if (isScalarLVal(lhs)) {
            if (lhs.text == loopVar) {
                plan.rejectReason = "loop induction variable write";
                return false;
            }
            if (locals.find(lhs.text) != locals.end()) {
                return true;
            }
            if (lookupType && lookupType(lhs.text) == "int") {
                const Node *add = parallelReductionAddend(node, lhs.text);
                if (add != nullptr) {
                    addReduction(plan.reductions, ParallelReduction{lhs.text, add});
                    return true;
                }
            }
            plan.rejectReason = "unsafe scalar write";
            return false;
        }
        if (!lvalHasAffineLoopIndex(lhs, loopVar)) {
            plan.rejectReason = "non-affine array write";
            return false;
        }
        if (!sameArrayReadsArePartitioned(*node.children.at(1), lhs.text, loopVar)) {
            plan.rejectReason = "same-array read is not partitioned";
            return false;
        }
        plan.hasArrayWrite = true;
        return true;
    }
    default:
        for (const auto &child : node.children) {
            if (!analyzeNode(*child, loopVar, locals, lookupType, plan)) {
                return false;
            }
        }
        return true;
    }
}

bool profitableParallelLoop(const ParallelLoopPlan &plan) {
    if (!plan.valid) {
        return false;
    }
    if (!plan.reductions.empty()) {
        return true;
    }
    if (plan.hasNestedLoop) {
        return true;
    }
    return plan.estimatedCost >= 48;
}

} // namespace

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

std::string jsonEscape(const std::string &text) {
    std::ostringstream out;
    for (char ch : text) {
        switch (ch) {
        case '\\':
            out << "\\\\";
            break;
        case '"':
            out << "\\\"";
            break;
        case '\n':
            out << "\\n";
            break;
        case '\r':
            out << "\\r";
            break;
        case '\t':
            out << "\\t";
            break;
        default:
            out << ch;
            break;
        }
    }
    return out.str();
}

class ParallelPlanJsonDumper {
public:
    std::string dump(const Node &root) {
        scopes_.clear();
        nextId_ = 0;
        loopDepth_ = 0;
        out_.str("");
        out_.clear();
        pushScope();
        visitRoot(root);
        popScope();
        return out_.str();
    }

private:
    void pushScope() {
        scopes_.push_back({});
    }

    void popScope() {
        scopes_.pop_back();
    }

    void declare(const std::string &name, const std::string &type) {
        if (!scopes_.empty() && !name.empty()) {
            scopes_.back()[name] = type;
        }
    }

    std::string lookupType(const std::string &name) const {
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                return found->second;
            }
        }
        return {};
    }

    void declareDecl(const Node &node) {
        if (node.kind != NodeKind::VarDecl && node.kind != NodeKind::ConstDecl) {
            return;
        }
        for (const auto &def : node.children) {
            declare(def->text, node.text);
        }
    }

    void visitRoot(const Node &node) {
        for (const auto &child : node.children) {
            if (child->kind == NodeKind::FuncDef) {
                visitFunction(*child);
            } else {
                declareDecl(*child);
            }
        }
    }

    void visitFunction(const Node &node) {
        pushScope();
        for (const auto &child : node.children) {
            if (child->kind == NodeKind::FuncParam) {
                declare(secondWord(child->text), firstWord(child->text));
            }
        }
        for (const auto &child : node.children) {
            if (child->kind == NodeKind::Block) {
                visitBlock(*child);
            }
        }
        popScope();
    }

    void visitBlock(const Node &node) {
        pushScope();
        for (std::size_t i = 0; i < node.children.size(); ++i) {
            if (i + 1 < node.children.size() &&
                parseParallelLoopInit(*node.children[i]).valid &&
                node.children[i + 1]->kind == NodeKind::WhileStmt) {
                ParallelLoopPlan plan = analyzeParallelLoopPair(
                    *node.children[i], *node.children[i + 1],
                    [this](const std::string &name) { return lookupType(name); });
                emitPlan(plan, node.children[i]->loc, node.children[i + 1]->loc);
            }
            visitStmt(*node.children[i]);
        }
        popScope();
    }

    void visitStmt(const Node &node) {
        switch (node.kind) {
        case NodeKind::VarDecl:
        case NodeKind::ConstDecl:
            declareDecl(node);
            return;
        case NodeKind::Block:
            visitBlock(node);
            return;
        case NodeKind::IfStmt:
            if (node.children.size() > 1) {
                visitStmt(*node.children[1]);
            }
            if (node.children.size() > 2) {
                visitStmt(*node.children[2]);
            }
            return;
        case NodeKind::WhileStmt:
            if (node.children.size() > 1) {
                ++loopDepth_;
                visitStmt(*node.children[1]);
                --loopDepth_;
            }
            return;
        default:
            return;
        }
    }

    void emitPlan(const ParallelLoopPlan &plan, SourceLocation initLoc, SourceLocation loopLoc) {
        std::string loweringKind;
        std::string runtimeSymbol;
        if (plan.valid) {
            if (plan.reductions.empty()) {
                loweringKind = "parallel_for";
                runtimeSymbol = "__sysy_parallel_for_range";
            } else {
                loweringKind = "parallel_reduce_int";
                runtimeSymbol = "__sysy_parallel_reduce_int_range";
            }
        }
        out_ << "{\"id\":" << nextId_++
             << ",\"init_line\":" << initLoc.line
             << ",\"init_column\":" << initLoc.column
             << ",\"loop_line\":" << loopLoc.line
             << ",\"loop_column\":" << loopLoc.column
             << ",\"loop_depth\":" << loopDepth_
             << ",\"valid\":" << (plan.valid ? "true" : "false")
             << ",\"init_var\":\"" << jsonEscape(plan.init.var) << "\""
             << ",\"init_decl\":" << (plan.init.declaration ? "true" : "false")
             << ",\"init_type\":\"" << jsonEscape(plan.init.type) << "\""
             << ",\"estimated_cost\":" << plan.estimatedCost
             << ",\"has_nested_loop\":" << (plan.hasNestedLoop ? "true" : "false")
             << ",\"has_array_write\":" << (plan.hasArrayWrite ? "true" : "false")
             << ",\"body_stmts\":" << plan.body.size()
             << ",\"lowering_kind\":\"" << loweringKind << "\""
             << ",\"runtime_symbol\":\"" << runtimeSymbol << "\""
             << ",\"captures\":[";
        for (std::size_t i = 0; i < plan.captures.size(); ++i) {
            const ParallelCapture &capture = plan.captures[i];
            if (i != 0) {
                out_ << ",";
            }
            std::string access;
            if (capture.read) {
                access += "R";
            }
            if (capture.write) {
                access += "W";
            }
            out_ << "{\"name\":\"" << jsonEscape(capture.name)
                 << "\",\"type\":\"" << jsonEscape(capture.type)
                 << "\",\"array\":" << (capture.array ? "true" : "false")
                 << ",\"access\":\"" << access << "\"}";
        }
        out_ << "],\"reductions\":[";
        for (std::size_t i = 0; i < plan.reductions.size(); ++i) {
            if (i != 0) {
                out_ << ",";
            }
            out_ << "{\"var\":\"" << jsonEscape(plan.reductions[i].var) << "\"}";
        }
        out_ << "],\"reject\":\"" << jsonEscape(plan.rejectReason) << "\"}\n";
    }

    std::vector<std::unordered_map<std::string, std::string>> scopes_;
    int nextId_ = 0;
    int loopDepth_ = 0;
    std::ostringstream out_;
};

ParallelLoopInit parseParallelLoopInit(const Node &node) {
    ParallelLoopInit init;
    if (node.kind == NodeKind::AssignStmt && node.children.size() == 2 &&
        isScalarLVal(*node.children.at(0))) {
        init.valid = true;
        init.var = node.children.at(0)->text;
        init.initExpr = node.children.at(1).get();
        return init;
    }
    if (node.kind == NodeKind::VarDecl && node.children.size() == 1 &&
        isScalarDef(*node.children.at(0))) {
        const Node *initExpr = initializerOf(*node.children.at(0));
        if (initExpr != nullptr) {
            init.valid = true;
            init.declaration = true;
            init.type = node.text;
            init.var = node.children.at(0)->text;
            init.initExpr = initExpr;
        }
    }
    return init;
}

const Node *parallelReductionAddend(const Node &assign, const std::string &var) {
    if (assign.kind != NodeKind::AssignStmt || assign.children.size() != 2) {
        return nullptr;
    }
    const Node &lhs = *assign.children.at(0);
    const Node &rhs = *assign.children.at(1);
    if (!isScalarLVal(lhs) || lhs.text != var || rhs.kind != NodeKind::BinaryExpr ||
        rhs.text != "+") {
        return nullptr;
    }
    const Node *left = rhs.children.at(0).get();
    const Node *right = rhs.children.at(1).get();
    if (left->kind == NodeKind::LVal && left->text == var && left->children.empty()) {
        return right;
    }
    if (right->kind == NodeKind::LVal && right->text == var && right->children.empty()) {
        return left;
    }
    return nullptr;
}

ParallelLoopPlan analyzeParallelLoopPair(const Node &initStmt, const Node &loopStmt,
                                         const ParallelTypeLookup &lookupType) {
    ParallelLoopPlan plan;
    plan.init = parseParallelLoopInit(initStmt);
    if (!plan.init.valid) {
        plan.rejectReason = "not a canonical loop initializer";
        return plan;
    }
    if (!canonicalWhile(loopStmt, plan.init, plan.endExpr, plan.body)) {
        plan.rejectReason = "not a canonical while loop";
        return plan;
    }

    plan.valid = true;
    std::unordered_set<std::string> locals;
    locals.insert(plan.init.var);
    for (const Node *stmt : plan.body) {
        plan.estimatedCost += estimateStmtCost(*stmt);
        if (stmt->kind == NodeKind::WhileStmt) {
            plan.hasNestedLoop = true;
        }
        collectLocalScalars(*stmt, locals);
    }
    for (const Node *stmt : plan.body) {
        if (!analyzeNode(*stmt, plan.init.var, locals, lookupType, plan)) {
            plan.valid = false;
            return plan;
        }
    }
    for (const Node *stmt : plan.body) {
        collectCapturesFromStmt(*stmt, plan.init.var, locals, lookupType, plan);
    }

    std::vector<ArrayAccess> reads;
    std::vector<ArrayAccess> writes;
    for (const Node *stmt : plan.body) {
        collectArrayReads(*stmt, reads);
        collectArrayWrites(*stmt, writes);
    }
    for (const ArrayAccess &write : writes) {
        for (const ArrayAccess &read : reads) {
            if (write.base == read.base && read.lval != nullptr &&
                !lvalFirstIndexIsPartitionedByLoopVar(*read.lval, plan.init.var)) {
                plan.valid = false;
                plan.rejectReason = "loop-carried array dependence";
                return plan;
            }
        }
    }
    if (!plan.hasArrayWrite && plan.reductions.empty()) {
        plan.valid = false;
        plan.rejectReason = "no parallel side effect";
        return plan;
    }
    if (plan.reductions.size() > 1) {
        plan.valid = false;
        plan.rejectReason = "multiple reductions";
        return plan;
    }
    if (!profitableParallelLoop(plan)) {
        plan.valid = false;
        plan.rejectReason = "not profitable";
    }
    return plan;
}

std::string dumpParallelPlansJsonl(const Node &root) {
    ParallelPlanJsonDumper dumper;
    return dumper.dump(root);
}

} // namespace sysy
