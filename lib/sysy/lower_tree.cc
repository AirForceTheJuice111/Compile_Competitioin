#include "lower_tree.hh"

#include "parallel_plan.hh"
#include "temp.hh"

#include <algorithm>
#include <cctype>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iomanip>
#include <map>
#include <cmath>
#include <numeric>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
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
    if (text.size() >= 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        return text.find('.') != std::string::npos ||
               text.find('p') != std::string::npos ||
               text.find('P') != std::string::npos;
    }
    return text.find('.') != std::string::npos ||
           text.find('e') != std::string::npos ||
           text.find('E') != std::string::npos;
}

int parseIntLiteral(const std::string &text) {
    char *end = nullptr;
    long value = std::strtol(text.c_str(), &end, 0);
    return static_cast<int>(value);
}

float parseFloatLiteral(const std::string &text) {
    char *end = nullptr;
    return std::strtof(text.c_str(), &end);
}

int floatBits(float value) {
    static_assert(sizeof(float) == sizeof(std::uint32_t), "float must be 32-bit");
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    return static_cast<int>(bits);
}

float bitsFloat(int bits) {
    std::uint32_t raw = static_cast<std::uint32_t>(bits);
    float value = 0.0f;
    std::memcpy(&value, &raw, sizeof(value));
    return value;
}

enum class BaseType {
    Int,
    Float,
    Void
};

tree::Type treeType(BaseType type) {
    return type == BaseType::Float ? tree::Type::FLOAT : tree::Type::INT;
}

BaseType baseTypeFromText(const std::string &text) {
    if (text == "float") {
        return BaseType::Float;
    }
    if (text == "void") {
        return BaseType::Void;
    }
    return BaseType::Int;
}

bool isFloatType(BaseType type) {
    return type == BaseType::Float;
}

struct ConstScalar {
    BaseType type = BaseType::Int;
    int raw = 0;
};

int constAsInt(ConstScalar value) {
    if (value.type == BaseType::Float) {
        return static_cast<int>(bitsFloat(value.raw));
    }
    return value.raw;
}

float constAsFloat(ConstScalar value) {
    if (value.type == BaseType::Float) {
        return bitsFloat(value.raw);
    }
    return static_cast<float>(value.raw);
}

bool constAsBool(ConstScalar value) {
    if (value.type == BaseType::Float) {
        return bitsFloat(value.raw) != 0.0f;
    }
    return value.raw != 0;
}

ConstScalar constScalarValue(const Node &node,
                             const std::function<std::optional<ConstScalar>(const std::string &)> &lookupConst = {}) {
    switch (node.kind) {
    case NodeKind::Number:
        if (isFloatText(node.text)) {
            return ConstScalar{BaseType::Float, floatBits(parseFloatLiteral(node.text))};
        }
        return ConstScalar{BaseType::Int, parseIntLiteral(node.text)};
    case NodeKind::LVal:
        if (!node.children.empty()) {
            throw LoweringError(node.loc, "native backend only supports scalar constants in constant expressions");
        }
        if (lookupConst) {
            std::optional<ConstScalar> value = lookupConst(node.text);
            if (value.has_value()) {
                return *value;
            }
        }
        break;
    case NodeKind::UnaryExpr: {
        ConstScalar value = constScalarValue(*node.children.at(0), lookupConst);
        if (node.text == "-") {
            if (value.type == BaseType::Float) {
                return ConstScalar{BaseType::Float, floatBits(-bitsFloat(value.raw))};
            }
            return ConstScalar{BaseType::Int, -value.raw};
        }
        if (node.text == "+") {
            return value;
        }
        if (node.text == "!") {
            return ConstScalar{BaseType::Int, constAsBool(value) ? 0 : 1};
        }
        break;
    }
    case NodeKind::BinaryExpr: {
        ConstScalar lhs = constScalarValue(*node.children.at(0), lookupConst);
        ConstScalar rhs = constScalarValue(*node.children.at(1), lookupConst);
        bool useFloat = lhs.type == BaseType::Float || rhs.type == BaseType::Float;
        if (node.text == "&&") return ConstScalar{BaseType::Int, constAsBool(lhs) && constAsBool(rhs)};
        if (node.text == "||") return ConstScalar{BaseType::Int, constAsBool(lhs) || constAsBool(rhs)};
        if (useFloat) {
            float l = constAsFloat(lhs);
            float r = constAsFloat(rhs);
            if (node.text == "+") return ConstScalar{BaseType::Float, floatBits(l + r)};
            if (node.text == "-") return ConstScalar{BaseType::Float, floatBits(l - r)};
            if (node.text == "*") return ConstScalar{BaseType::Float, floatBits(l * r)};
            if (node.text == "/") return ConstScalar{BaseType::Float, floatBits(l / r)};
            if (node.text == "==") return ConstScalar{BaseType::Int, l == r};
            if (node.text == "!=") return ConstScalar{BaseType::Int, l != r};
            if (node.text == "<") return ConstScalar{BaseType::Int, l < r};
            if (node.text == ">") return ConstScalar{BaseType::Int, l > r};
            if (node.text == "<=") return ConstScalar{BaseType::Int, l <= r};
            if (node.text == ">=") return ConstScalar{BaseType::Int, l >= r};
        } else {
            int l = lhs.raw;
            int r = rhs.raw;
            if (node.text == "+") return ConstScalar{BaseType::Int, l + r};
            if (node.text == "-") return ConstScalar{BaseType::Int, l - r};
            if (node.text == "*") return ConstScalar{BaseType::Int, l * r};
            if (node.text == "/") return ConstScalar{BaseType::Int, r == 0 ? 0 : l / r};
            if (node.text == "%") return ConstScalar{BaseType::Int, r == 0 ? 0 : l % r};
            if (node.text == "==") return ConstScalar{BaseType::Int, l == r};
            if (node.text == "!=") return ConstScalar{BaseType::Int, l != r};
            if (node.text == "<") return ConstScalar{BaseType::Int, l < r};
            if (node.text == ">") return ConstScalar{BaseType::Int, l > r};
            if (node.text == "<=") return ConstScalar{BaseType::Int, l <= r};
            if (node.text == ">=") return ConstScalar{BaseType::Int, l >= r};
        }
        break;
    }
    default:
        break;
    }
    throw LoweringError(node.loc, "native backend only supports constant scalar initializers for globals");
}

int constIntValue(const Node &node,
                  const std::function<std::optional<ConstScalar>(const std::string &)> &lookupConst = {}) {
    return constAsInt(constScalarValue(node, lookupConst));
}

bool isRuntimeFunction(const std::string &name) {
    static const std::set<std::string> runtime = {
        "getint", "getch", "getarray", "getfloat", "getfarray",
        "putint", "putch", "putarray", "putfloat", "putfarray",
        "putf", "starttime", "stoptime", "_sysy_starttime", "_sysy_stoptime"
    };
    return runtime.count(name) != 0;
}

bool isParallelStartTimingStmt(const Node &node) {
    return node.kind == NodeKind::ExprStmt && node.children.size() == 1 &&
           node.children.front()->kind == NodeKind::CallExpr &&
           (node.children.front()->text == "starttime" ||
            node.children.front()->text == "_sysy_starttime");
}

struct Symbol {
    tree::Temp *temp = nullptr;
    bool global = false;
    std::string label;
    std::vector<int> dims;
    BaseType base = BaseType::Int;
    bool constScalar = false;
    ConstScalar constValue = {};
    bool arrayParam = false;
};

struct FunctionSignature {
    BaseType ret = BaseType::Int;
    std::vector<BaseType> params;
    std::vector<int> paramDims;
};

std::string globalLabel(const std::string &name) {
    return "__sysy_global_" + name;
}

std::string stringLiteralLabel(const std::string &text) {
    std::ostringstream out;
    out << "__sysy_str_";
    for (unsigned char ch : text) {
        out << std::hex << std::setfill('0') << std::setw(2) << static_cast<unsigned int>(ch);
    }
    return out.str();
}

int hexValue(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    return -1;
}

std::vector<unsigned char> decodeStringLiteral(const std::string &text) {
    std::vector<unsigned char> bytes;
    std::size_t i = (!text.empty() && text.front() == '"') ? 1 : 0;
    std::size_t end = text.size();
    if (end > i && text[end - 1] == '"') {
        --end;
    }
    while (i < end) {
        unsigned char ch = static_cast<unsigned char>(text[i++]);
        if (ch != '\\' || i >= end) {
            bytes.push_back(ch);
            continue;
        }

        char esc = text[i++];
        switch (esc) {
        case 'a': bytes.push_back('\a'); break;
        case 'b': bytes.push_back('\b'); break;
        case 'f': bytes.push_back('\f'); break;
        case 'n': bytes.push_back('\n'); break;
        case 'r': bytes.push_back('\r'); break;
        case 't': bytes.push_back('\t'); break;
        case 'v': bytes.push_back('\v'); break;
        case '\\': bytes.push_back('\\'); break;
        case '\'': bytes.push_back('\''); break;
        case '"': bytes.push_back('"'); break;
        case '?': bytes.push_back('?'); break;
        case 'x': {
            int value = 0;
            int digits = 0;
            while (i < end) {
                int digit = hexValue(text[i]);
                if (digit < 0) break;
                value = (value << 4) | digit;
                ++i;
                ++digits;
            }
            bytes.push_back(static_cast<unsigned char>(digits == 0 ? 'x' : value));
            break;
        }
        default:
            if (esc >= '0' && esc <= '7') {
                int value = esc - '0';
                int digits = 1;
                while (digits < 3 && i < end && text[i] >= '0' && text[i] <= '7') {
                    value = value * 8 + (text[i++] - '0');
                    ++digits;
                }
                bytes.push_back(static_cast<unsigned char>(value));
            } else {
                bytes.push_back(static_cast<unsigned char>(esc));
            }
            break;
        }
    }
    return bytes;
}

std::vector<char> putfFormatSpecifiers(const std::string &literal, SourceLocation loc) {
    std::vector<char> specs;
    std::vector<unsigned char> bytes = decodeStringLiteral(literal);
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        if (bytes[i] != '%') {
            continue;
        }
        if (i + 1 >= bytes.size()) {
            throw LoweringError(loc, "unterminated putf format specifier");
        }
        unsigned char spec = bytes[++i];
        if (spec == '%') {
            continue;
        }
        if (spec == 'd' || spec == 'c' || spec == 'f') {
            specs.push_back(static_cast<char>(spec));
            continue;
        }
        throw LoweringError(loc, "unsupported putf format specifier");
    }
    return specs;
}

void collectStringLiterals(const Node &node, std::set<std::string> &out) {
    if (node.kind == NodeKind::StringLiteral) {
        out.insert(node.text);
    }
    for (const auto &child : node.children) {
        collectStringLiterals(*child, out);
    }
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

// Find global arrays whose storage is never observed.  A candidate survives
// only when every source-level use is a fully-indexed assignment destination
// and evaluating that assignment cannot have side effects.  This deliberately
// treats reads, partially-indexed values (including array arguments), and
// calls in either the index or value expression as escapes.  The analysis is
// whole-program and name-conservative; local shadowing can therefore cause a
// missed opportunity, but never makes a global use disappear.
std::unordered_set<std::string> findWriteOnlyGlobalArrays(const Node &root) {
    std::unordered_map<std::string, std::size_t> candidates;
    if (root.kind != NodeKind::CompUnit) {
        return {};
    }
    for (const auto &decl : root.children) {
        if (decl->kind != NodeKind::ConstDecl && decl->kind != NodeKind::VarDecl) {
            continue;
        }
        for (const auto &def : decl->children) {
            std::size_t dimensions = static_cast<std::size_t>(std::count_if(
                def->children.begin(), def->children.end(), [](const auto &child) {
                    return child->kind == NodeKind::ArrayDim;
                }));
            if (dimensions != 0) {
                candidates[def->text] = dimensions;
            }
        }
    }

    std::unordered_set<std::string> observed;
    std::function<void(const Node &)> visit = [&](const Node &node) {
        if (node.kind == NodeKind::AssignStmt && node.children.size() == 2 &&
            node.children.front()->kind == NodeKind::LVal) {
            const Node &lhs = *node.children.front();
            auto candidate = candidates.find(lhs.text);
            if (candidate != candidates.end()) {
                if (lhs.children.size() != candidate->second ||
                    exprContainsCall(node)) {
                    observed.insert(lhs.text);
                }
                // The outer LVal is the permitted store destination.  Its
                // index expressions and the RHS can still read/escape a
                // same-named global, so inspect those normally.
                for (const auto &index : lhs.children) {
                    visit(*index);
                }
                visit(*node.children.at(1));
                return;
            }
        }
        if (node.kind == NodeKind::LVal && candidates.count(node.text) != 0) {
            observed.insert(node.text);
        }
        for (const auto &child : node.children) {
            visit(*child);
        }
    };
    visit(root);

    std::unordered_set<std::string> writeOnly;
    for (const auto &candidate : candidates) {
        if (observed.count(candidate.first) == 0) {
            writeOnly.insert(candidate.first);
        }
    }
    return writeOnly;
}

void appendStringBytes(std::string &out, const std::vector<unsigned char> &bytes) {
    std::vector<unsigned char> withNull = bytes;
    withNull.push_back(0);
    constexpr std::size_t kPerLine = 16;
    for (std::size_t i = 0; i < withNull.size(); i += kPerLine) {
        out += "    .byte ";
        for (std::size_t j = i; j < withNull.size() && j < i + kPerLine; ++j) {
            if (j != i) out += ", ";
            out += std::to_string(static_cast<unsigned int>(withNull[j]));
        }
        out += "\n";
    }
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

void appendCompressedWords(std::string &out, const std::vector<int> &values) {
    std::size_t i = 0;
    while (i < values.size()) {
        if (values[i] != 0) {
            out += "    .word " + std::to_string(values[i]) + "\n";
            ++i;
            continue;
        }
        std::size_t begin = i;
        while (i < values.size() && values[i] == 0) {
            ++i;
        }
        out += "    .zero " + std::to_string((i - begin) * 4) + "\n";
    }
}

class Lowerer {
public:
    explicit Lowerer(LoweringOptions options = {}) : options_(options) {}

    tree::Program *lower(const Node &root) {
        if (root.kind != NodeKind::CompUnit) {
            throw LoweringError(root.loc, "expected compilation unit");
        }

        collectFunctions(root);
        collectGlobals(root);
        writeOnlyGlobalArrays_ = findWriteOnlyGlobalArrays(root);
        parallelFunctionSummaries_ = summarizeParallelScalarFunctions(root);
        generatedFunctions_.clear();
        parallelWorkerId_ = 0;

        auto *funcs = new std::vector<tree::FuncDecl *>();
        for (const auto &child : root.children) {
            if (child->kind == NodeKind::FuncDef) {
                funcs->push_back(lowerFunction(*child));
            }
        }
        funcs->insert(funcs->end(), generatedFunctions_.begin(), generatedFunctions_.end());
        return new tree::Program(funcs);
    }

    std::string emitGlobalData(const Node &root) {
        if (root.kind != NodeKind::CompUnit) {
            throw LoweringError(root.loc, "expected compilation unit");
        }
        collectFunctions(root);
        collectGlobals(root);
        writeOnlyGlobalArrays_ = findWriteOnlyGlobalArrays(root);

        std::string out;
        std::set<std::string> stringLiterals;
        collectStringLiterals(root, stringLiterals);
        std::string currentSection;
        if (!stringLiterals.empty()) {
            out += "\n.section .rodata\n.balign 4\n";
            currentSection = ".rodata";
            for (const auto &literal : stringLiterals) {
                out += stringLiteralLabel(literal) + ":\n";
                appendStringBytes(out, decodeStringLiteral(literal));
            }
        }
        auto selectSection = [&](const std::string &section) {
            if (currentSection == section) {
                return;
            }
            out += "\n.section " + section + "\n.balign 4\n";
            currentSection = section;
        };

        for (const auto &child : root.children) {
            if (child->kind != NodeKind::ConstDecl && child->kind != NodeKind::VarDecl) {
                continue;
            }
            BaseType base = baseTypeFromText(child->text);
            for (const auto &def : child->children) {
                if (writeOnlyGlobalArrays_.count(def->text) != 0) {
                    continue;
                }
                std::vector<int> dims = arrayDims(*def);
                const Node *initNode = initializerNode(*def);
                if (initNode == nullptr) {
                    // Uninitialized SysY objects are zero-filled.  Put them in
                    // NOBITS storage and avoid materializing a vector with one
                    // host integer per target element; the preliminary stencil
                    // cases contain arrays with hundreds of millions of words.
                    selectSection(".bss");
                    out += ".global " + globalLabel(def->text) + "\n";
                    out += globalLabel(def->text) + ":\n";
                    std::int64_t words = dims.empty()
                        ? 1
                        : std::accumulate(dims.begin(), dims.end(),
                                          std::int64_t{1}, std::multiplies<std::int64_t>());
                    out += "    .zero " + std::to_string(words * 4) + "\n";
                    continue;
                }

                selectSection(".data");
                out += ".global " + globalLabel(def->text) + "\n";
                out += globalLabel(def->text) + ":\n";
                if (dims.empty()) {
                    ConstScalar init{base, base == BaseType::Float ? floatBits(0.0f) : 0};
                    init = evalConstScalar(*initNode, base);
                    out += "    .word " + std::to_string(init.raw) + "\n";
                } else {
                    std::vector<int> values(dimProduct(dims), 0);
                    fillArrayInitializer(*initNode, dims, values, [this, base](const Node &scalar) {
                        return evalConstScalar(scalar, base).raw;
                    });
                    appendCompressedWords(out, values);
                }
            }
        }
        return out;
    }

private:
    struct ParallelContextField {
        ParallelCapture capture;
        Symbol symbol;
        int offset = 0;
    };

    struct ParallelScratchWriteback {
        Symbol symbol;
        tree::Temp *finalValue = nullptr;
    };

    using ParallelAliasPair = std::pair<std::size_t, std::size_t>;

    LoweringOptions options_;
    std::map<std::string, Symbol> globalSymbols_;
    std::unordered_set<std::string> writeOnlyGlobalArrays_;
    std::map<std::string, FunctionSignature> functions_;
    ParallelFunctionSummaries parallelFunctionSummaries_;
    std::vector<std::unordered_map<std::string, Symbol>> scopes_;
    tree::Temp_map temps_;
    std::vector<tree::Label *> breakLabels_;
    std::vector<tree::Label *> continueLabels_;
    std::vector<tree::FuncDecl *> generatedFunctions_;
    BaseType currentReturnType_ = BaseType::Int;
    std::string currentFunctionName_;
    int parallelWorkerId_ = 0;
    bool suppressParallelLowering_ = false;
    std::unordered_set<const Node *> suppressedParallelIvUpdates_;
    std::string workerModReductionVar_;
    int workerModulus_ = 0;

    tree::Temp *newTemp() { return temps_.newtemp(); }
    tree::Label *newLabel() { return temps_.newlabel(); }

    tree::Exp *tempExp(tree::Temp *temp, BaseType type = BaseType::Int) {
        return new tree::TempExp(treeType(type), new tree::Temp(temp->num));
    }

    tree::Exp *ptrTempExp(tree::Temp *temp) {
        return new tree::TempExp(tree::Type::PTR, new tree::Temp(temp->num));
    }

    tree::Exp *zero(BaseType type = BaseType::Int) {
        if (type == BaseType::Float) {
            return new tree::Const(floatBits(0.0f), tree::Type::FLOAT);
        }
        return new tree::Const(0);
    }

    tree::Exp *intRemainder(tree::Exp *lhsValue, tree::Exp *rhsValue) {
        auto *lhsTemp = newTemp();
        auto *rhsTemp = newTemp();
        auto *stms = new std::vector<tree::Stm *>({
            new tree::Move(tempExp(lhsTemp), lhsValue),
            new tree::Move(tempExp(rhsTemp), rhsValue)
        });
        auto *quotient = new tree::Binop(tree::Type::INT, "/",
                                         tempExp(lhsTemp), tempExp(rhsTemp));
        auto *product = new tree::Binop(tree::Type::INT, "*", quotient,
                                        tempExp(rhsTemp));
        return new tree::Eseq(
            tree::Type::INT, new tree::Seq(stms),
            new tree::Binop(tree::Type::INT, "-", tempExp(lhsTemp), product));
    }

    void pushScope() { scopes_.push_back({}); }

    void popScope() { scopes_.pop_back(); }

    void declareLocal(const std::string &name, tree::Temp *temp, BaseType base, SourceLocation loc) {
        if (scopes_.empty()) {
            throw LoweringError(loc, "internal lowering scope error");
        }
        scopes_.back()[name] = Symbol{temp, false, {}, {}, base};
    }

    void declareLocalArray(const std::string &name, tree::Temp *temp,
                           std::vector<int> dims, BaseType base, SourceLocation loc,
                           bool arrayParam = false) {
        if (scopes_.empty()) {
            throw LoweringError(loc, "internal lowering scope error");
        }
        scopes_.back()[name] = Symbol{temp, false, {}, std::move(dims), base, false, {}, arrayParam};
    }

    void declareLocalConstScalar(const std::string &name, BaseType base, ConstScalar value, SourceLocation loc) {
        if (scopes_.empty()) {
            throw LoweringError(loc, "internal lowering scope error");
        }
        if (base == BaseType::Int && value.type == BaseType::Float) {
            value = ConstScalar{BaseType::Int, constAsInt(value)};
        } else if (base == BaseType::Float && value.type == BaseType::Int) {
            value = ConstScalar{BaseType::Float, floatBits(static_cast<float>(value.raw))};
        }
        scopes_.back()[name] = Symbol{nullptr, false, {}, {}, base, true, value};
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

    std::optional<ConstScalar> lookupConstScalar(const std::string &name) const {
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                if (found->second.constScalar) {
                    return found->second.constValue;
                }
                return std::nullopt;
            }
        }
        auto global = globalSymbols_.find(name);
        if (global != globalSymbols_.end() && global->second.constScalar) {
            return global->second.constValue;
        }
        return std::nullopt;
    }

    int evalConstInt(const Node &node) const {
        return constIntValue(node, [this](const std::string &name) {
            return lookupConstScalar(name);
        });
    }

    ConstScalar evalConstScalar(const Node &node, BaseType target = BaseType::Int) const {
        ConstScalar value = constScalarValue(node, [this](const std::string &name) {
            return lookupConstScalar(name);
        });
        if (target == BaseType::Float && value.type == BaseType::Int) {
            return ConstScalar{BaseType::Float, floatBits(static_cast<float>(value.raw))};
        }
        if (target == BaseType::Int && value.type == BaseType::Float) {
            return ConstScalar{BaseType::Int, constAsInt(value)};
        }
        return value;
    }

    void addRuntimeFunctions() {
        functions_.clear();
        functions_["getint"] = FunctionSignature{BaseType::Int, {}, {}};
        functions_["getch"] = FunctionSignature{BaseType::Int, {}, {}};
        functions_["getarray"] = FunctionSignature{BaseType::Int, {BaseType::Int}, {1}};
        functions_["getfloat"] = FunctionSignature{BaseType::Float, {}, {}};
        functions_["getfarray"] = FunctionSignature{BaseType::Int, {BaseType::Float}, {1}};
        functions_["putint"] = FunctionSignature{BaseType::Void, {BaseType::Int}, {0}};
        functions_["putch"] = FunctionSignature{BaseType::Void, {BaseType::Int}, {0}};
        functions_["putarray"] = FunctionSignature{BaseType::Void, {BaseType::Int, BaseType::Int}, {0, 1}};
        functions_["putfloat"] = FunctionSignature{BaseType::Void, {BaseType::Float}, {0}};
        functions_["putfarray"] = FunctionSignature{BaseType::Void, {BaseType::Int, BaseType::Float}, {0, 1}};
        functions_["putf"] = FunctionSignature{BaseType::Void, {}, {}};
        functions_["starttime"] = FunctionSignature{BaseType::Void, {}, {}};
        functions_["stoptime"] = FunctionSignature{BaseType::Void, {}, {}};
        functions_["_sysy_starttime"] = FunctionSignature{BaseType::Void, {BaseType::Int}, {0}};
        functions_["_sysy_stoptime"] = FunctionSignature{BaseType::Void, {BaseType::Int}, {0}};
        functions_["__sysy_parallel_for_range"] =
            FunctionSignature{BaseType::Void,
                              {BaseType::Int, BaseType::Int, BaseType::Int, BaseType::Int,
                               BaseType::Int},
                              {0, 0, 0, 0, 0}};
        functions_["__sysy_parallel_reduce_int_range"] =
            FunctionSignature{BaseType::Int,
                              {BaseType::Int, BaseType::Int, BaseType::Int, BaseType::Int,
                               BaseType::Int},
                              {0, 0, 0, 0, 0}};
        functions_["__sysy_parallel_reduce_mod_int_range"] =
            FunctionSignature{BaseType::Int,
                              {BaseType::Int, BaseType::Int, BaseType::Int, BaseType::Int,
                               BaseType::Int, BaseType::Int},
                              {0, 0, 0, 0, 0, 0}};
        functions_["__sysy_parallel_trip_count"] =
            FunctionSignature{BaseType::Int,
                              {BaseType::Int, BaseType::Int, BaseType::Int,
                               BaseType::Int},
                              {0, 0, 0, 0}};
        functions_["free"] = FunctionSignature{BaseType::Void, {BaseType::Int}, {0}};
    }

    void collectFunctions(const Node &root) {
        addRuntimeFunctions();
        for (const auto &child : root.children) {
            if (child->kind != NodeKind::FuncDef) {
                continue;
            }
            FunctionSignature sig;
            sig.ret = baseTypeFromText(firstWord(child->text));
            for (const auto &param : child->children) {
                if (param->kind != NodeKind::FuncParam) {
                    continue;
                }
                sig.params.push_back(baseTypeFromText(firstWord(param->text)));
                int dims = static_cast<int>(std::count_if(param->children.begin(), param->children.end(),
                                                          [](const auto &dim) {
                                                              return dim->kind == NodeKind::ArrayDim;
                                                          }));
                sig.paramDims.push_back(dims);
            }
            functions_[secondWord(child->text)] = std::move(sig);
        }
    }

    void collectGlobals(const Node &root) {
        globalSymbols_.clear();
        for (const auto &child : root.children) {
            if (child->kind != NodeKind::ConstDecl && child->kind != NodeKind::VarDecl) {
                continue;
            }
            BaseType base = baseTypeFromText(child->text);
            bool isConstDecl = child->kind == NodeKind::ConstDecl;
            for (const auto &def : child->children) {
                std::vector<int> dims = arrayDims(*def);
                ConstScalar init{base, base == BaseType::Float ? floatBits(0.0f) : 0};
                const Node *initNode = initializerNode(*def);
                if (dims.empty()) {
                    if (initNode != nullptr) {
                        init = evalConstScalar(*initNode, base);
                    }
                }
                bool isConstScalar = isConstDecl && dims.empty();
                globalSymbols_[def->text] = Symbol{nullptr, true, globalLabel(def->text), dims,
                                                   base, isConstScalar, init};
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
            int dim = evalConstInt(*child->children.at(0));
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

    BaseType lvalueBaseType(const Node &node) const {
        return lookup(node.text, node.loc).base;
    }

    BaseType callReturnType(const std::string &name) const {
        auto found = functions_.find(name);
        if (found == functions_.end()) {
            return BaseType::Int;
        }
        return found->second.ret;
    }

    BaseType exprBaseType(const Node &node) const {
        switch (node.kind) {
        case NodeKind::Number:
            return isFloatText(node.text) ? BaseType::Float : BaseType::Int;
        case NodeKind::LVal:
            return lvalueBaseType(node);
        case NodeKind::UnaryExpr:
            if (node.text == "!") {
                return BaseType::Int;
            }
            return exprBaseType(*node.children.at(0));
        case NodeKind::BinaryExpr:
            if (node.text == "==" || node.text == "!=" || node.text == "<" || node.text == ">" ||
                node.text == "<=" || node.text == ">=" || node.text == "&&" || node.text == "||" ||
                node.text == "%") {
                return BaseType::Int;
            }
            if (exprBaseType(*node.children.at(0)) == BaseType::Float ||
                exprBaseType(*node.children.at(1)) == BaseType::Float) {
                return BaseType::Float;
            }
            return BaseType::Int;
        case NodeKind::CallExpr: {
            std::string name = node.text;
            if (name == "starttime") {
                name = "_sysy_starttime";
            } else if (name == "stoptime") {
                name = "_sysy_stoptime";
            }
            return callReturnType(name);
        }
        default:
            return BaseType::Int;
        }
    }

    tree::Exp *convertExpr(tree::Exp *expr, BaseType target) {
        if (expr == nullptr || target == BaseType::Void) {
            return expr;
        }
        BaseType actual = expr->type == tree::Type::FLOAT ? BaseType::Float : BaseType::Int;
        if (actual == target) {
            return expr;
        }
        auto *args = new std::vector<tree::Exp *>({expr});
        if (target == BaseType::Float) {
            return new tree::ExtCall(tree::Type::FLOAT, "__sysy_i2f_bits", args);
        }
        return new tree::ExtCall(tree::Type::INT, "__sysy_f2i_bits", args);
    }

    tree::Exp *lowerExprAs(const Node &node, BaseType target) {
        return convertExpr(lowerExpr(node), target);
    }

    std::vector<tree::Exp *> arrayInitializerExprs(const Node &def, const std::vector<int> &dims, BaseType base) {
        std::vector<tree::Exp *> values(static_cast<std::size_t>(dimProduct(dims)), nullptr);
        const Node *init = initializerNode(def);
        if (init == nullptr) {
            for (auto *&value : values) {
                value = zero(base);
            }
            return values;
        }
        fillArrayInitializer(*init, dims, values, [this, base](const Node &scalar) {
            return convertExpr(lowerExpr(scalar), base);
        });
        for (auto *&value : values) {
            if (value == nullptr) {
                value = zero(base);
            }
        }
        return values;
    }

    tree::FuncDecl *lowerFunction(const Node &node) {
        std::string ret = firstWord(node.text);
        std::string name = secondWord(node.text);
        currentReturnType_ = baseTypeFromText(ret);
        currentFunctionName_ = name;

        pushScope();
        auto *params = new std::vector<tree::Temp *>();
        auto *paramTypes = new std::vector<tree::Type>();
        auto *stms = new std::vector<tree::Stm *>();
        stms->push_back(new tree::LabelStm(newLabel()));

        for (const auto &child : node.children) {
            if (child->kind != NodeKind::FuncParam) {
                continue;
            }
            BaseType paramBase = baseTypeFromText(firstWord(child->text));
            auto *param = newTemp();
            params->push_back(param);
            std::vector<int> dims;
            if (!child->children.empty()) {
                for (const auto &dim : child->children) {
                    if (dim->kind != NodeKind::ArrayDim) {
                        continue;
                    }
                    dims.push_back(dim->children.empty() ? -1 : evalConstInt(*dim->children.at(0)));
                }
            }
            if (dims.empty()) {
                paramTypes->push_back(treeType(paramBase));
                declareLocal(secondWord(child->text), param, paramBase, child->loc);
            } else {
                paramTypes->push_back(tree::Type::PTR);
                declareLocalArray(secondWord(child->text), param, std::move(dims), paramBase, child->loc, true);
            }
        }

        for (const auto &child : node.children) {
            if (child->kind == NodeKind::Block) {
                lowerBlock(*child, stms, false);
            }
        }

        if (blockFallsThrough(stms)) {
            stms->push_back(new tree::Return(zero(currentReturnType_)));
        }
        popScope();

        return new tree::FuncDecl(name, params, new tree::Seq(stms),
                                  ret == "void" ? tree::Type::INT : treeType(currentReturnType_),
                                  temps_.next_temp - 1, temps_.next_label - 1,
                                  paramTypes);
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
        for (std::size_t index = 0; index < node.children.size(); ++index) {
            if (!fallsThrough) {
                break;
            }
            const auto &child = node.children[index];
            std::size_t loopIndex = index + 1;
            const Node *betweenInitAndLoop = nullptr;
            if (index + 2 < node.children.size() &&
                isParallelStartTimingStmt(*node.children[index + 1])) {
                loopIndex = index + 2;
                betweenInitAndLoop = node.children[index + 1].get();
            }
            if (options_.parallelLoops && !suppressParallelLowering_ &&
                loopIndex < node.children.size() &&
                parseParallelLoopInit(*node.children[index]).valid &&
                node.children[loopIndex]->kind == NodeKind::WhileStmt) {
                ParallelLoopPlan plan = analyzeParallelLoopPair(
                    *node.children[index], *node.children[loopIndex],
                    [this](const std::string &name) { return parallelTypeName(name); },
                    [this](const std::string &name)
                        -> const ParallelScalarFunctionSummary * {
                        auto found = parallelFunctionSummaries_.find(name);
                        return found == parallelFunctionSummaries_.end()
                                   ? nullptr
                                   : &found->second;
                    },
                    [this](const std::string &name) -> std::optional<int> {
                        std::optional<ConstScalar> value = lookupConstScalar(name);
                        if (!value || value->type != BaseType::Int) {
                            return std::nullopt;
                        }
                        return value->raw;
                    });
                if (lowerParallelLoop(plan, stms, betweenInitAndLoop)) {
                    index = loopIndex;
                    continue;
                }
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
        BaseType base = baseTypeFromText(node.text);
        bool isConstDecl = node.kind == NodeKind::ConstDecl;
        for (const auto &def : node.children) {
            std::vector<int> dims = arrayDims(*def);
            if (isConstDecl && dims.empty()) {
                const Node *init = initializerNode(*def);
                if (init == nullptr) {
                    throw LoweringError(def->loc, "const scalar needs initializer");
                }
                declareLocalConstScalar(def->text, base, evalConstScalar(*init, base), def->loc);
                continue;
            }
            auto *temp = newTemp();
            if (!dims.empty()) {
                declareLocalArray(def->text, temp, dims, base, def->loc);
                int totalBytes = dimProduct(dims) * 4;
                auto *mallocArgs = new std::vector<tree::Exp *>({new tree::Const(totalBytes)});
                stms->push_back(new tree::Move(new tree::TempExp(tree::Type::PTR, new tree::Temp(temp->num)),
                                               new tree::ExtCall(tree::Type::PTR, "malloc", mallocArgs)));
                if (initializerNode(*def) != nullptr) {
                    auto *memsetArgs = new std::vector<tree::Exp *>({
                        new tree::TempExp(tree::Type::PTR, new tree::Temp(temp->num)),
                        new tree::Const(0),
                        new tree::Const(totalBytes)
                    });
                    stms->push_back(new tree::ExpStm(new tree::ExtCall(tree::Type::INT, "memset", memsetArgs)));
                    std::vector<tree::Exp *> values = arrayInitializerExprs(*def, dims, base);
                    for (std::size_t i = 0; i < values.size(); ++i) {
                        auto *constValue = dynamic_cast<tree::Const *>(values[i]);
                        if (constValue != nullptr && constValue->constVal == 0) {
                            continue;
                        }
                        stms->push_back(new tree::Move(
                            new tree::Mem(treeType(base),
                                          arrayElementAddress(Symbol{temp, false, {}, dims, base}, i)),
                            values[i]));
                    }
                }
                continue;
            }
            declareLocal(def->text, temp, base, def->loc);
            tree::Exp *init = zero(base);
            if (!def->children.empty()) {
                init = lowerExprAs(*def->children.back(), base);
            }
            stms->push_back(new tree::Move(tempExp(temp, base), init));
        }
    }

    std::string parallelTypeName(const std::string &name) const {
        Symbol sym = lookup(name, SourceLocation{});
        if (sym.base == BaseType::Float) {
            return "float";
        }
        if (sym.base == BaseType::Void) {
            return "void";
        }
        return "int";
    }

    tree::Exp *ctxFieldAddr(tree::Temp *ctx, int offset) {
        return new tree::Binop(tree::Type::PTR, "+", ptrTempExp(ctx), new tree::Const(offset));
    }

    int pointerBytes() const {
        return options_.pointerBytes <= 4 ? 4 : 8;
    }

    int alignTo(int value, int align) const {
        if (align <= 1) {
            return value;
        }
        int rem = value % align;
        return rem == 0 ? value : value + align - rem;
    }

    int contextFieldSize(const ParallelCapture &capture) const {
        return capture.array ? pointerBytes() : 4;
    }

    int contextFieldAlign(const ParallelCapture &capture) const {
        return capture.array ? pointerBytes() : 4;
    }

    tree::Exp *symbolScalarValue(const Symbol &sym) {
        if (sym.constScalar) {
            return new tree::Const(sym.constValue.raw, treeType(sym.base));
        }
        if (sym.global) {
            return new tree::Mem(treeType(sym.base), new tree::Name(new tree::String_Label(sym.label)));
        }
        return tempExp(sym.temp, sym.base);
    }

    bool buildParallelContextFields(const ParallelLoopPlan &plan,
                                    std::vector<ParallelContextField> &fields) {
        fields.clear();
        int offset = 0;
        for (const ParallelCapture &capture : plan.captures) {
            Symbol sym = lookup(capture.name, plan.init.initExpr == nullptr ? SourceLocation{} : plan.init.initExpr->loc);
            if (capture.array && !isArraySymbol(sym)) {
                return false;
            }
            if (!capture.array && isArraySymbol(sym)) {
                return false;
            }
            offset = alignTo(offset, contextFieldAlign(capture));
            fields.push_back(ParallelContextField{capture, sym, offset});
            offset += contextFieldSize(capture);
        }
        return true;
    }

    int parallelContextBytes(const std::vector<ParallelContextField> &fields) const {
        int bytes = 0;
        for (const ParallelContextField &field : fields) {
            bytes = std::max(bytes, field.offset + contextFieldSize(field.capture));
        }
        return alignTo(bytes, pointerBytes());
    }

    // A plain multi-reduction reuses the existing scalar reduction runtime:
    // the first accumulator is returned in w0, while the worker writes the
    // remaining per-worker partials into this hidden tail of the context.
    // Keep the layout private to lowering so ordinary captures and the public
    // runtime ABI remain unchanged.
    int parallelReductionBaseOffset(
        const std::vector<ParallelContextField> &fields) const {
        return alignTo(parallelContextBytes(fields), 4);
    }

    int parallelReductionSlotOffset(
        const std::vector<ParallelContextField> &fields,
        std::size_t reductionCount, std::size_t workerSlot,
        std::size_t reductionIndex) const {
        int base = parallelReductionBaseOffset(fields);
        return base + static_cast<int>((workerSlot * reductionCount +
                                        reductionIndex) * sizeof(std::int32_t));
    }

    int parallelReductionBeginOffset(
        const std::vector<ParallelContextField> &fields,
        std::size_t reductionCount) const {
        return parallelReductionBaseOffset(fields) +
               static_cast<int>(2 * reductionCount * sizeof(std::int32_t));
    }

    int parallelContextBytes(const std::vector<ParallelContextField> &fields,
                             std::size_t reductionCount) const {
        int bytes = parallelContextBytes(fields);
        if (reductionCount <= 1) {
            return bytes;
        }
        return alignTo(parallelReductionBeginOffset(fields, reductionCount) +
                           static_cast<int>(sizeof(std::int32_t)),
                       pointerBytes());
    }

    int parallelDynamicInitialOffset(
        const std::vector<ParallelContextField> &fields,
        std::size_t reductionCount) const {
        return alignTo(parallelContextBytes(fields, reductionCount), 4);
    }

    int parallelContextBytes(const std::vector<ParallelContextField> &fields,
                             std::size_t reductionCount,
                             bool dynamicLogicalRange) const {
        int bytes = parallelContextBytes(fields, reductionCount);
        if (dynamicLogicalRange) {
            bytes = parallelDynamicInitialOffset(fields, reductionCount) +
                    static_cast<int>(sizeof(std::int32_t));
        }
        return alignTo(bytes, pointerBytes());
    }

    bool aliasPairNeedsRuntimeGuard(const ParallelContextField &lhs,
                                    const ParallelContextField &rhs,
                                    bool &needsGuard) const {
        needsGuard = false;
        if (!lhs.capture.array || !rhs.capture.array ||
            (!lhs.capture.write && !rhs.capture.write)) {
            return true;
        }
        if (lhs.symbol.base != rhs.symbol.base) {
            return true;
        }
        if (!lhs.symbol.arrayParam && !rhs.symbol.arrayParam) {
            return true;
        }
        if ((!lhs.symbol.arrayParam && !lhs.symbol.global) ||
            (!rhs.symbol.arrayParam && !rhs.symbol.global)) {
            return true;
        }
        if (lhs.symbol.dims.size() != rhs.symbol.dims.size()) {
            return false;
        }
        needsGuard = true;
        return true;
    }

    bool buildParallelAliasGuardPairs(const std::vector<ParallelContextField> &fields,
                                      std::vector<ParallelAliasPair> &pairs) const {
        pairs.clear();
        for (std::size_t i = 0; i < fields.size(); ++i) {
            for (std::size_t j = i + 1; j < fields.size(); ++j) {
                bool needsGuard = false;
                if (!aliasPairNeedsRuntimeGuard(fields[i], fields[j], needsGuard)) {
                    return false;
                }
                if (needsGuard) {
                    pairs.push_back({i, j});
                }
            }
        }
        return true;
    }

    bool reductionDestinationIsInt(const ParallelLoopPlan &plan) const {
        if (plan.reductions.empty()) {
            return true;
        }
        for (const ParallelReduction &reduction : plan.reductions) {
            // Float reductions deliberately remain sequential: changing the
            // association of IEEE-754 additions is observable.  The planner
            // currently admits only integer scalar reductions, but retain the
            // check here as a lowering-side safety net.
            if (reduction.modular && plan.reductions.size() > 1) {
                return false;
            }
            Symbol dst = lookup(reduction.var, plan.init.initExpr->loc);
            if (isArraySymbol(dst) || dst.base != BaseType::Int) {
                return false;
            }
        }
        return true;
    }

    bool buildParallelScratchSymbols(const ParallelLoopPlan &plan,
                                     std::vector<Symbol> &symbols) const {
        symbols.clear();
        for (const ParallelPrivatizedScalar &scalar : plan.privatizedScalars) {
            if (scalar.type != "int" || scalar.initExpr == nullptr ||
                scalar.endExpr == nullptr) {
                return false;
            }
            Symbol symbol = lookup(scalar.var, scalar.initExpr->loc);
            if (symbol.base != BaseType::Int || symbol.global || symbol.constScalar ||
                symbol.temp == nullptr || isArraySymbol(symbol)) {
                return false;
            }
            symbols.push_back(symbol);
        }
        return true;
    }

    std::vector<ParallelScratchWriteback> buildParallelScratchWritebacks(
        const ParallelLoopPlan &plan, const std::vector<Symbol> &symbols,
        std::vector<tree::Stm *> *stms) {
        std::vector<ParallelScratchWriteback> writebacks;
        for (std::size_t index = 0; index < plan.privatizedScalars.size(); ++index) {
            const ParallelPrivatizedScalar &scalar = plan.privatizedScalars[index];
            auto *finalTemp = newTemp();
            auto *endTemp = newTemp();
            stms->push_back(new tree::Move(
                tempExp(finalTemp), lowerExprAs(*scalar.initExpr, BaseType::Int)));
            tree::Exp *exclusiveEnd = lowerExprAs(*scalar.endExpr, BaseType::Int);
            if (scalar.inclusiveEnd) {
                exclusiveEnd = new tree::Binop(tree::Type::INT, "+", exclusiveEnd,
                                               new tree::Const(1));
            }
            stms->push_back(new tree::Move(tempExp(endTemp), exclusiveEnd));

            auto *setEndLabel = newLabel();
            auto *doneLabel = newLabel();
            stms->push_back(new tree::Cjump("<", tempExp(finalTemp), tempExp(endTemp),
                                            setEndLabel, doneLabel));
            stms->push_back(new tree::LabelStm(setEndLabel));
            stms->push_back(new tree::Move(tempExp(finalTemp), tempExp(endTemp)));
            stms->push_back(new tree::Jump(doneLabel));
            stms->push_back(new tree::LabelStm(doneLabel));
            writebacks.push_back(ParallelScratchWriteback{symbols[index], finalTemp});
        }
        return writebacks;
    }

    void emitParallelScratchWritebacks(
        const std::vector<ParallelScratchWriteback> &writebacks,
        tree::Temp *beginTemp, tree::Temp *endTemp,
        std::vector<tree::Stm *> *stms) {
        if (writebacks.empty()) {
            return;
        }
        auto *writeLabel = newLabel();
        auto *doneLabel = newLabel();
        stms->push_back(new tree::Cjump("<", tempExp(beginTemp), tempExp(endTemp),
                                        writeLabel, doneLabel));
        stms->push_back(new tree::LabelStm(writeLabel));
        for (const ParallelScratchWriteback &writeback : writebacks) {
            stms->push_back(new tree::Move(tempExp(writeback.symbol.temp),
                                            tempExp(writeback.finalValue)));
        }
        stms->push_back(new tree::Jump(doneLabel));
        stms->push_back(new tree::LabelStm(doneLabel));
    }

    void emitRuntimeAliasGuard(const std::vector<ParallelContextField> &fields,
                               const std::vector<ParallelAliasPair> &pairs,
                               tree::Label *aliasLabel,
                               tree::Label *noAliasLabel,
                               std::vector<tree::Stm *> *stms) {
        for (const ParallelAliasPair &pair : pairs) {
            auto *nextLabel = newLabel();
            stms->push_back(new tree::Cjump("==",
                                            arrayBase(fields[pair.first].symbol),
                                            arrayBase(fields[pair.second].symbol),
                                            aliasLabel,
                                            nextLabel));
            stms->push_back(new tree::LabelStm(nextLabel));
        }
        stms->push_back(new tree::Jump(noAliasLabel));
    }

    void lowerParallelSequentialFallback(const ParallelLoopPlan &plan,
                                         tree::Temp *ivTemp,
                                         tree::Temp *endTemp,
                                         const std::string &relop,
                                         std::vector<tree::Stm *> *stms) {
        auto *testLabel = newLabel();
        auto *bodyLabel = newLabel();
        auto *latchLabel = newLabel();
        auto *doneLabel = newLabel();
        stms->push_back(new tree::LabelStm(testLabel));
        stms->push_back(new tree::Cjump(relop, tempExp(ivTemp), tempExp(endTemp),
                                        bodyLabel, doneLabel));
        stms->push_back(new tree::LabelStm(bodyLabel));

        bool savedSuppress = suppressParallelLowering_;
        auto savedSuppressedIvUpdates = suppressedParallelIvUpdates_;
        suppressParallelLowering_ = true;
        suppressedParallelIvUpdates_.clear();
        suppressedParallelIvUpdates_.insert(
            plan.canonicalContinueUpdates.begin(),
            plan.canonicalContinueUpdates.end());
        continueLabels_.push_back(latchLabel);
        pushScope();
        bool bodyFallsThrough = true;
        for (const Node *stmt : plan.body) {
            if (!bodyFallsThrough) {
                break;
            }
            if (stmt->kind == NodeKind::ConstDecl || stmt->kind == NodeKind::VarDecl) {
                lowerDecl(*stmt, stms);
                bodyFallsThrough = true;
            } else {
                bodyFallsThrough = lowerStmt(*stmt, stms);
            }
        }
        popScope();
        continueLabels_.pop_back();
        suppressParallelLowering_ = savedSuppress;
        suppressedParallelIvUpdates_ =
            std::move(savedSuppressedIvUpdates);

        // Both ordinary fallthrough and a validated candidate-loop continue
        // reach this latch.  Its explicit source update was suppressed above,
        // so the IV advances exactly once on either path.
        stms->push_back(new tree::LabelStm(latchLabel));
        stms->push_back(new tree::Move(
            tempExp(ivTemp),
            new tree::Binop(tree::Type::INT, "+", tempExp(ivTemp),
                            new tree::Const(plan.step))));
        stms->push_back(new tree::Jump(testLabel));
        stms->push_back(new tree::LabelStm(doneLabel));
    }

    void emitParallelRuntimeCall(const ParallelLoopPlan &plan,
                                 const std::vector<ParallelContextField> &fields,
                                 const std::string &workerName,
                                 tree::Temp *beginTemp,
                                 tree::Temp *endTemp,
                                 tree::Temp *sourceInitialTemp,
                                 std::vector<tree::Stm *> *stms,
                                 tree::Label *modUnsafeLabel = nullptr) {
        const bool multiReduction = plan.reductions.size() > 1;
        tree::Temp *ctxTemp = nullptr;
        tree::Exp *ctxArg = new tree::Const(0, tree::Type::PTR);
        // Multi-reduction workers use a private tail in the context for the
        // second worker's partials.  Force a context allocation even when the
        // source body has no ordinary captures.
        if (!fields.empty() || multiReduction || plan.dynamicLogicalRange) {
            ctxTemp = newTemp();
            const std::size_t reductionCount =
                multiReduction ? plan.reductions.size() : 0;
            int ctxBytes = parallelContextBytes(fields, reductionCount,
                                                plan.dynamicLogicalRange);
            stms->push_back(new tree::Move(
                ptrTempExp(ctxTemp),
                new tree::ExtCall(tree::Type::PTR, "malloc",
                                  new std::vector<tree::Exp *>({new tree::Const(ctxBytes)}))));
            ctxArg = ptrTempExp(ctxTemp);
            for (const ParallelContextField &field : fields) {
                tree::Exp *value = field.capture.array ? arrayBase(field.symbol)
                                                       : symbolScalarValue(field.symbol);
                tree::Type fieldType = field.capture.array ? tree::Type::PTR : treeType(field.symbol.base);
                stms->push_back(new tree::Move(
                    new tree::Mem(fieldType, ctxFieldAddr(ctxTemp, field.offset)),
                    value));
            }
            if (multiReduction) {
                const std::size_t reductionCount = plan.reductions.size();
                // The worker chooses slot zero for the first half (or a
                // direct fallback) by comparing its begin parameter with this
                // original range start.  Zero both slots so a defensive
                // direct path still has a complete result if a worker exits
                // early in the future.
                stms->push_back(new tree::Move(
                    new tree::Mem(
                        tree::Type::INT,
                        ctxFieldAddr(ctxTemp,
                                     parallelReductionBeginOffset(fields,
                                                                  reductionCount))),
                    tempExp(beginTemp)));
                for (std::size_t slot = 0; slot < 2; ++slot) {
                    for (std::size_t index = 0; index < reductionCount; ++index) {
                        stms->push_back(new tree::Move(
                            new tree::Mem(
                                tree::Type::INT,
                                ctxFieldAddr(
                                    ctxTemp,
                                    parallelReductionSlotOffset(
                                        fields, reductionCount, slot, index))),
                            new tree::Const(0)));
                    }
                }
            }
            if (plan.dynamicLogicalRange) {
                stms->push_back(new tree::Move(
                    new tree::Mem(
                        tree::Type::INT,
                        ctxFieldAddr(
                            ctxTemp,
                            parallelDynamicInitialOffset(fields,
                                                         reductionCount))),
                    tempExp(sourceInitialTemp)));
            }
        }

        int emittedWorkCost = std::max(1, plan.runtimeWorkCost);
        // A candidate selected inside sequential loops may invoke the helper
        // once per enclosing iteration.  Discount each such level so a costly
        // inner worker is not mistaken for one coarse parallel region.  The
        // planner's nested-body estimate remains boosted; only repeated helper
        // invocation is discounted here.
        for (std::size_t depth = 0; depth < breakLabels_.size(); ++depth) {
            emittedWorkCost = emittedWorkCost / 8 +
                              (emittedWorkCost % 8 == 0 ? 0 : 1);
            emittedWorkCost = std::max(1, emittedWorkCost);
        }

        auto *runtimeArgs = new std::vector<tree::Exp *>({
            tempExp(beginTemp),
            tempExp(endTemp),
            ctxArg,
            new tree::Name(new tree::String_Label(workerName)),
            new tree::Const(emittedWorkCost)
        });
        if (!plan.reductions.empty() && plan.reductions.front().modular) {
            runtimeArgs->push_back(new tree::Const(plan.reductions.front().modulus));
        }

        if (plan.reductions.empty()) {
            stms->push_back(new tree::ExpStm(
                new tree::ExtCall(tree::Type::INT, "__sysy_parallel_for_range", runtimeArgs)));
        } else if (plan.reductions.front().modular) {
            auto *partialTemp = newTemp();
            stms->push_back(new tree::Move(
                tempExp(partialTemp),
                new tree::ExtCall(tree::Type::INT,
                                  "__sysy_parallel_reduce_mod_int_range",
                                  runtimeArgs)));
            if (ctxTemp != nullptr) {
                stms->push_back(new tree::ExpStm(
                    new tree::ExtCall(tree::Type::INT, "free",
                                      new std::vector<tree::Exp *>({ptrTempExp(ctxTemp)}))));
                ctxTemp = nullptr;
            }
            if (modUnsafeLabel != nullptr) {
                auto *safePartialLabel = newLabel();
                stms->push_back(new tree::Cjump(
                    "==", tempExp(partialTemp), new tree::Const(INT_MIN),
                    modUnsafeLabel, safePartialLabel));
                stms->push_back(new tree::LabelStm(safePartialLabel));
            }
            const ParallelReduction &reduction = plan.reductions.front();
            const std::string &var = reduction.var;
            auto *combineLabel = newLabel();
            auto *doneLabel = newLabel();
            stms->push_back(new tree::Cjump("<", tempExp(beginTemp), tempExp(endTemp),
                                            combineLabel, doneLabel));
            stms->push_back(new tree::LabelStm(combineLabel));
            auto *dst = lowerLValue(
                Node{NodeKind::LVal, plan.init.initExpr->loc, var});
            auto *initialResidue = intRemainder(
                lowerExpr(Node{NodeKind::LVal, plan.init.initExpr->loc, var}),
                new tree::Const(reduction.modulus));
            auto *sum = new tree::Binop(tree::Type::INT, "+", initialResidue,
                                        tempExp(partialTemp));
            stms->push_back(new tree::Move(
                dst, intRemainder(sum, new tree::Const(reduction.modulus))));
            stms->push_back(new tree::Jump(doneLabel));
            stms->push_back(new tree::LabelStm(doneLabel));
        } else if (multiReduction) {
            // Keep the existing scalar runtime ABI: it returns the first
            // partial sum in w0 and invokes the same worker for both halves.
            // Additional accumulators are written by the worker into the
            // hidden context slots and combined here after the runtime's
            // completion barrier.
            auto *firstPartial = newTemp();
            stms->push_back(new tree::Move(
                tempExp(firstPartial),
                new tree::ExtCall(tree::Type::INT,
                                  "__sysy_parallel_reduce_int_range",
                                  runtimeArgs)));
            const std::size_t reductionCount = plan.reductions.size();
            for (std::size_t index = 0; index < reductionCount; ++index) {
                tree::Exp *partial = nullptr;
                if (index == 0) {
                    partial = tempExp(firstPartial);
                } else {
                    auto *slot0 = new tree::Mem(
                        tree::Type::INT,
                        ctxFieldAddr(
                            ctxTemp,
                            parallelReductionSlotOffset(fields, reductionCount,
                                                        0, index)));
                    auto *slot1 = new tree::Mem(
                        tree::Type::INT,
                        ctxFieldAddr(
                            ctxTemp,
                            parallelReductionSlotOffset(fields, reductionCount,
                                                        1, index)));
                    partial = new tree::Binop(tree::Type::INT, "+", slot0, slot1);
                }
                const std::string &var = plan.reductions[index].var;
                auto *dst = lowerLValue(
                    Node{NodeKind::LVal, plan.init.initExpr->loc, var});
                stms->push_back(new tree::Move(
                    dst,
                    new tree::Binop(
                        tree::Type::INT, "+",
                        lowerExpr(Node{NodeKind::LVal,
                                       plan.init.initExpr->loc, var}),
                        partial)));
            }
        } else {
            auto *partialTemp = newTemp();
            stms->push_back(new tree::Move(
                tempExp(partialTemp),
                new tree::ExtCall(tree::Type::INT, "__sysy_parallel_reduce_int_range", runtimeArgs)));
            const std::string &var = plan.reductions.front().var;
            auto *dst = lowerLValue(Node{NodeKind::LVal, plan.init.initExpr->loc, var});
            stms->push_back(new tree::Move(
                dst,
                new tree::Binop(tree::Type::INT, "+",
                                lowerExpr(Node{NodeKind::LVal, plan.init.initExpr->loc, var}),
                                tempExp(partialTemp))));
        }

        if (ctxTemp != nullptr) {
            stms->push_back(new tree::ExpStm(
                new tree::ExtCall(tree::Type::INT, "free",
                                  new std::vector<tree::Exp *>({ptrTempExp(ctxTemp)}))));
        }
    }

    void emitParallelFinalIvUpdate(const ParallelLoopPlan &plan,
                                   tree::Temp *ivTemp,
                                   tree::Temp *beginTemp,
                                   tree::Temp *endTemp,
                                   std::vector<tree::Stm *> *stms) {
        if (plan.logicalTripCount >= 0) {
            stms->push_back(new tree::Move(tempExp(ivTemp),
                                           new tree::Const(plan.finalIv)));
            return;
        }
        if (plan.dynamicLogicalRange) {
            stms->push_back(new tree::Move(
                tempExp(ivTemp),
                new tree::Binop(
                    tree::Type::INT, "+", tempExp(ivTemp),
                    new tree::Binop(tree::Type::INT, "*", tempExp(endTemp),
                                    new tree::Const(plan.step)))));
            return;
        }
        auto *setEndLabel = newLabel();
        auto *doneLabel = newLabel();
        stms->push_back(new tree::Cjump("<", tempExp(beginTemp), tempExp(endTemp),
                                        setEndLabel, doneLabel));
        stms->push_back(new tree::LabelStm(setEndLabel));
        stms->push_back(new tree::Move(tempExp(ivTemp), tempExp(endTemp)));
        stms->push_back(new tree::Jump(doneLabel));
        stms->push_back(new tree::LabelStm(doneLabel));
    }

    std::string sanitizedFunctionName(const std::string &name) const {
        std::string out;
        for (char ch : name) {
            if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_') {
                out.push_back(ch);
            } else {
                out.push_back('_');
            }
        }
        return out.empty() ? "fn" : out;
    }

    tree::FuncDecl *buildParallelWorkerFunction(const std::string &workerName,
                                                const ParallelLoopPlan &plan,
                                                const std::vector<ParallelContextField> &fields) {
        auto savedScopes = scopes_;
        auto savedBreakLabels = breakLabels_;
        auto savedContinueLabels = continueLabels_;
        BaseType savedReturnType = currentReturnType_;
        bool savedSuppress = suppressParallelLowering_;
        auto savedSuppressedIvUpdates = suppressedParallelIvUpdates_;
        std::string savedModVar = workerModReductionVar_;
        int savedModulus = workerModulus_;

        scopes_.clear();
        breakLabels_.clear();
        continueLabels_.clear();
        currentReturnType_ = BaseType::Int;
        suppressParallelLowering_ = true;
        suppressedParallelIvUpdates_.clear();
        suppressedParallelIvUpdates_.insert(
            plan.canonicalContinueUpdates.begin(),
            plan.canonicalContinueUpdates.end());
        workerModReductionVar_.clear();
        workerModulus_ = 0;
        if (!plan.reductions.empty() && plan.reductions.front().modular) {
            workerModReductionVar_ = plan.reductions.front().var;
            workerModulus_ = plan.reductions.front().modulus;
        }

        auto *beginParam = newTemp();
        auto *endParam = newTemp();
        auto *ctxParam = newTemp();
        auto *params = new std::vector<tree::Temp *>({beginParam, endParam, ctxParam});
        auto *paramTypes = new std::vector<tree::Type>(
            {tree::Type::INT, tree::Type::INT, tree::Type::PTR});
        auto *stms = new std::vector<tree::Stm *>();
        stms->push_back(new tree::LabelStm(newLabel()));

        pushScope();
        declareLocal("__sysy_begin", beginParam, BaseType::Int, plan.init.initExpr->loc);
        declareLocal("__sysy_end", endParam, BaseType::Int, plan.init.initExpr->loc);

        for (const ParallelContextField &field : fields) {
            auto *captureTemp = newTemp();
            if (field.capture.array) {
                declareLocalArray(field.capture.name, captureTemp, field.symbol.dims,
                                  field.symbol.base, plan.init.initExpr->loc,
                                  field.symbol.arrayParam);
                stms->push_back(new tree::Move(
                    ptrTempExp(captureTemp),
                    new tree::Mem(tree::Type::PTR, ctxFieldAddr(ctxParam, field.offset))));
            } else {
                declareLocal(field.capture.name, captureTemp, field.symbol.base, plan.init.initExpr->loc);
                stms->push_back(new tree::Move(
                    tempExp(captureTemp, field.symbol.base),
                    new tree::Mem(treeType(field.symbol.base), ctxFieldAddr(ctxParam, field.offset))));
            }
        }

        for (const ParallelPrivatizedScalar &scalar : plan.privatizedScalars) {
            auto *scratchTemp = newTemp();
            declareLocal(scalar.var, scratchTemp, BaseType::Int, scalar.initExpr->loc);
            stms->push_back(new tree::Move(tempExp(scratchTemp), new tree::Const(0)));
        }

        std::vector<tree::Temp *> reductionTemps;
        reductionTemps.reserve(plan.reductions.size());
        for (const ParallelReduction &reduction : plan.reductions) {
            auto *reductionTemp = newTemp();
            reductionTemps.push_back(reductionTemp);
            declareLocal(reduction.var, reductionTemp, BaseType::Int,
                         plan.init.initExpr->loc);
            stms->push_back(new tree::Move(tempExp(reductionTemp),
                                           new tree::Const(0)));
        }

        auto *ivTemp = newTemp();
        declareLocal(plan.init.var, ivTemp, BaseType::Int, plan.init.initExpr->loc);
        tree::Temp *logicalIvTemp = nullptr;
        const bool logicalRange =
            plan.logicalTripCount >= 0 || plan.dynamicLogicalRange;
        if (logicalRange) {
            logicalIvTemp = newTemp();
            stms->push_back(new tree::Move(tempExp(logicalIvTemp), tempExp(beginParam)));
            tree::Exp *initialValue = nullptr;
            if (plan.dynamicLogicalRange) {
                const std::size_t reductionCount =
                    plan.reductions.size() > 1 ? plan.reductions.size() : 0;
                initialValue = new tree::Mem(
                    tree::Type::INT,
                    ctxFieldAddr(
                        ctxParam,
                        parallelDynamicInitialOffset(fields, reductionCount)));
            } else {
                initialValue = new tree::Const(plan.initialIv);
            }
            stms->push_back(new tree::Move(
                tempExp(ivTemp),
                new tree::Binop(
                    tree::Type::INT, "+", initialValue,
                    new tree::Binop(tree::Type::INT, "*", tempExp(beginParam),
                                    new tree::Const(plan.step)))));
        } else {
            stms->push_back(new tree::Move(tempExp(ivTemp), tempExp(beginParam)));
        }

        auto *testLabel = newLabel();
        auto *bodyLabel = newLabel();
        auto *latchLabel = newLabel();
        auto *doneLabel = newLabel();
        stms->push_back(new tree::LabelStm(testLabel));
        stms->push_back(new tree::Cjump(
            "<", logicalRange ? tempExp(logicalIvTemp) : tempExp(ivTemp),
            tempExp(endParam), bodyLabel, doneLabel));
        stms->push_back(new tree::LabelStm(bodyLabel));

        pushScope();
        continueLabels_.push_back(latchLabel);
        bool bodyFallsThrough = true;
        for (const Node *stmt : plan.body) {
            if (!bodyFallsThrough) {
                break;
            }
            if (stmt->kind == NodeKind::ConstDecl || stmt->kind == NodeKind::VarDecl) {
                lowerDecl(*stmt, stms);
                bodyFallsThrough = true;
            } else {
                bodyFallsThrough = lowerStmt(*stmt, stms);
            }
        }
        continueLabels_.pop_back();
        popScope();

        stms->push_back(new tree::LabelStm(latchLabel));
        stms->push_back(new tree::Move(
            tempExp(ivTemp),
            new tree::Binop(tree::Type::INT, "+", tempExp(ivTemp),
                            new tree::Const(plan.step))));
        if (logicalIvTemp != nullptr) {
            stms->push_back(new tree::Move(
                tempExp(logicalIvTemp),
                new tree::Binop(tree::Type::INT, "+", tempExp(logicalIvTemp),
                                new tree::Const(1))));
        }
        stms->push_back(new tree::Jump(testLabel));
        stms->push_back(new tree::LabelStm(doneLabel));
        if (plan.reductions.size() > 1) {
            const std::size_t reductionCount = plan.reductions.size();
            auto *slotZeroLabel = newLabel();
            auto *slotOneLabel = newLabel();
            auto *slotDoneLabel = newLabel();
            int beginOffset = parallelReductionBeginOffset(fields,
                                                           reductionCount);
            stms->push_back(new tree::Cjump(
                "==", tempExp(beginParam),
                new tree::Mem(tree::Type::INT,
                              ctxFieldAddr(ctxParam, beginOffset)),
                slotZeroLabel, slotOneLabel));
            stms->push_back(new tree::LabelStm(slotZeroLabel));
            for (std::size_t index = 0; index < reductionCount; ++index) {
                stms->push_back(new tree::Move(
                    new tree::Mem(
                        tree::Type::INT,
                        ctxFieldAddr(
                            ctxParam,
                            parallelReductionSlotOffset(fields, reductionCount,
                                                        0, index))),
                    tempExp(reductionTemps[index])));
            }
            stms->push_back(new tree::Jump(slotDoneLabel));
            stms->push_back(new tree::LabelStm(slotOneLabel));
            for (std::size_t index = 0; index < reductionCount; ++index) {
                stms->push_back(new tree::Move(
                    new tree::Mem(
                        tree::Type::INT,
                        ctxFieldAddr(
                            ctxParam,
                            parallelReductionSlotOffset(fields, reductionCount,
                                                        1, index))),
                    tempExp(reductionTemps[index])));
            }
            stms->push_back(new tree::LabelStm(slotDoneLabel));
        }
        stms->push_back(new tree::Return(
            reductionTemps.empty() ? new tree::Const(0)
                                   : tempExp(reductionTemps.front())));
        popScope();

        auto *worker = new tree::FuncDecl(workerName, params, new tree::Seq(stms),
                                          tree::Type::INT, temps_.next_temp - 1,
                                          temps_.next_label - 1, paramTypes);

        scopes_ = std::move(savedScopes);
        breakLabels_ = std::move(savedBreakLabels);
        continueLabels_ = std::move(savedContinueLabels);
        currentReturnType_ = savedReturnType;
        suppressParallelLowering_ = savedSuppress;
        suppressedParallelIvUpdates_ =
            std::move(savedSuppressedIvUpdates);
        workerModReductionVar_ = std::move(savedModVar);
        workerModulus_ = savedModulus;
        return worker;
    }

    const Node *workerModularAddend(const Node &assign) const {
        if (workerModReductionVar_.empty() || assign.kind != NodeKind::AssignStmt ||
            assign.children.size() != 2) {
            return nullptr;
        }
        const Node &lhs = *assign.children.at(0);
        const Node &rhs = *assign.children.at(1);
        if (lhs.kind != NodeKind::LVal || !lhs.children.empty() ||
            lhs.text != workerModReductionVar_ || rhs.kind != NodeKind::BinaryExpr ||
            rhs.text != "%" || rhs.children.size() != 2) {
            return nullptr;
        }
        const Node &sum = *rhs.children.at(0);
        if (sum.kind != NodeKind::BinaryExpr || sum.text != "+" ||
            sum.children.size() != 2) {
            return nullptr;
        }
        const Node *left = sum.children.at(0).get();
        const Node *right = sum.children.at(1).get();
        if (left->kind == NodeKind::LVal && left->children.empty() &&
            left->text == workerModReductionVar_) {
            return right;
        }
        if (right->kind == NodeKind::LVal && right->children.empty() &&
            right->text == workerModReductionVar_) {
            return left;
        }
        return nullptr;
    }

    bool lowerParallelLoop(const ParallelLoopPlan &plan, std::vector<tree::Stm *> *stms,
                           const Node *betweenInitAndLoop = nullptr) {
        if (!options_.parallelLoops || suppressParallelLowering_ || !plan.valid ||
            plan.init.initExpr == nullptr || plan.endExpr == nullptr) {
            return false;
        }
        if (exprContainsCall(*plan.endExpr)) {
            return false;
        }
        // The source comparison follows the usual int/float conversion rules,
        // while the native range runtime accepts integer endpoints only.  In
        // particular, truncating `i < 600.5` to the integer endpoint 600 would
        // drop the final source iteration.  Keep mixed/float bounds on the
        // ordinary sequential lowering path.
        if (exprBaseType(*plan.endExpr) != BaseType::Int) {
            return false;
        }
        if (!plan.init.type.empty() && plan.init.type != "int") {
            return false;
        }
        if (!plan.init.declaration) {
            Symbol iv = lookup(plan.init.var, plan.init.initExpr->loc);
            if (iv.base != BaseType::Int || iv.global || iv.temp == nullptr) {
                return false;
            }
        }

        // A planner capture turns a global array into a worker parameter.  Do
        // not generate that worker when the source stores are about to be
        // removed, or it would retain a store to an omitted global symbol.
        for (const ParallelCapture &capture : plan.captures) {
            if (writeOnlyGlobalArrays_.count(capture.name) == 0) {
                continue;
            }
            Symbol captured = lookup(capture.name, plan.init.initExpr->loc);
            if (captured.global && captured.label == globalLabel(capture.name)) {
                return false;
            }
        }

        std::vector<ParallelContextField> fields;
        std::vector<ParallelAliasPair> aliasPairs;
        std::vector<Symbol> scratchSymbols;
        if (!buildParallelContextFields(plan, fields) ||
            !reductionDestinationIsInt(plan) ||
            !buildParallelScratchSymbols(plan, scratchSymbols) ||
            !buildParallelAliasGuardPairs(fields, aliasPairs)) {
            return false;
        }

        std::string workerName = "__sysy_parallel_worker_" +
                                 sanitizedFunctionName(currentFunctionName_) + "_" +
                                 std::to_string(parallelWorkerId_++);
        generatedFunctions_.push_back(buildParallelWorkerFunction(workerName, plan, fields));

        tree::Temp *ivTemp = nullptr;
        if (plan.init.declaration) {
            ivTemp = newTemp();
            declareLocal(plan.init.var, ivTemp, BaseType::Int, plan.init.initExpr->loc);
        } else {
            Symbol iv = lookup(plan.init.var, plan.init.initExpr->loc);
            ivTemp = iv.temp;
        }
        stms->push_back(new tree::Move(tempExp(ivTemp), lowerExprAs(*plan.init.initExpr, BaseType::Int)));
        if (betweenInitAndLoop != nullptr) {
            lowerStmt(*betweenInitAndLoop, stms);
        }

        auto *beginTemp = newTemp();
        auto *rawEndTemp = newTemp();
        auto *endTemp = newTemp();
        const bool logicalRange =
            plan.logicalTripCount >= 0 || plan.dynamicLogicalRange;
        stms->push_back(new tree::Move(
            tempExp(beginTemp), logicalRange
                                    ? static_cast<tree::Exp *>(new tree::Const(0))
                                    : static_cast<tree::Exp *>(tempExp(ivTemp))));
        stms->push_back(new tree::Move(
            tempExp(rawEndTemp), lowerExprAs(*plan.endExpr, BaseType::Int)));

        tree::Label *inclusiveDoneLabel = nullptr;
        tree::Label *dynamicDoneLabel = nullptr;
        if (plan.logicalTripCount >= 0) {
            stms->push_back(new tree::Move(tempExp(endTemp),
                                           new tree::Const(plan.logicalTripCount)));
        } else if (plan.dynamicLogicalRange) {
            int comparisonKind = 0;
            if (plan.comparison == "<=") comparisonKind = 1;
            else if (plan.comparison == ">") comparisonKind = 2;
            else if (plan.comparison == ">=") comparisonKind = 3;
            else if (plan.comparison == "!=") comparisonKind = 4;
            stms->push_back(new tree::Move(
                tempExp(endTemp),
                new tree::ExtCall(
                    tree::Type::INT, "__sysy_parallel_trip_count",
                    new std::vector<tree::Exp *>({
                        tempExp(ivTemp), tempExp(rawEndTemp),
                        new tree::Const(plan.step),
                        new tree::Const(comparisonKind)}))));
            auto *unsafeLabel = newLabel();
            auto *safeLabel = newLabel();
            dynamicDoneLabel = newLabel();
            stms->push_back(new tree::Cjump("<", tempExp(endTemp),
                                            new tree::Const(0), unsafeLabel,
                                            safeLabel));
            stms->push_back(new tree::LabelStm(unsafeLabel));
            lowerParallelSequentialFallback(plan, ivTemp, rawEndTemp,
                                            plan.comparison, stms);
            stms->push_back(new tree::Jump(dynamicDoneLabel));
            stms->push_back(new tree::LabelStm(safeLabel));
        } else if (plan.inclusiveEnd) {
            // Normalizing <= to a half-open range needs end + 1.  Preserve the
            // original wrapping-loop behavior when a dynamic endpoint is
            // INT_MAX by taking a sequential path that uses the source <=
            // comparison instead of overflowing the normalized endpoint.
            auto *overflowLabel = newLabel();
            auto *normalLabel = newLabel();
            inclusiveDoneLabel = newLabel();
            stms->push_back(new tree::Cjump("==", tempExp(rawEndTemp),
                                            new tree::Const(INT_MAX),
                                            overflowLabel, normalLabel));
            stms->push_back(new tree::LabelStm(overflowLabel));
            lowerParallelSequentialFallback(plan, ivTemp, rawEndTemp, "<=", stms);
            stms->push_back(new tree::Jump(inclusiveDoneLabel));
            stms->push_back(new tree::LabelStm(normalLabel));
            stms->push_back(new tree::Move(
                tempExp(endTemp),
                new tree::Binop(tree::Type::INT, "+", tempExp(rawEndTemp),
                                new tree::Const(1))));
        } else {
            stms->push_back(new tree::Move(tempExp(endTemp), tempExp(rawEndTemp)));
        }

        std::vector<ParallelScratchWriteback> scratchWritebacks =
            buildParallelScratchWritebacks(plan, scratchSymbols, stms);

        if (!aliasPairs.empty()) {
            auto *sequentialLabel = newLabel();
            auto *parallelLabel = newLabel();
            auto *doneLabel = newLabel();
            emitRuntimeAliasGuard(fields, aliasPairs, sequentialLabel, parallelLabel, stms);

            stms->push_back(new tree::LabelStm(sequentialLabel));
            lowerParallelSequentialFallback(plan, ivTemp,
                                            logicalRange || plan.inclusiveEnd
                                                ? rawEndTemp
                                                : endTemp,
                                            plan.comparison, stms);
            stms->push_back(new tree::Jump(doneLabel));

            stms->push_back(new tree::LabelStm(parallelLabel));
            emitParallelRuntimeCall(plan, fields, workerName, beginTemp, endTemp,
                                    ivTemp, stms);
            emitParallelFinalIvUpdate(plan, ivTemp, beginTemp, endTemp, stms);
            emitParallelScratchWritebacks(scratchWritebacks, beginTemp, endTemp, stms);
            stms->push_back(new tree::Jump(doneLabel));

            stms->push_back(new tree::LabelStm(doneLabel));
        } else if (!plan.reductions.empty() && plan.reductions.front().modular) {
            const ParallelReduction &reduction = plan.reductions.front();
            auto *sequentialLabel = newLabel();
            auto *checkUpperLabel = newLabel();
            auto *parallelLabel = newLabel();
            auto *doneLabel = newLabel();
            tree::Exp *reductionValue = lowerExpr(
                Node{NodeKind::LVal, plan.init.initExpr->loc, reduction.var});
            stms->push_back(new tree::Cjump("<", reductionValue,
                                            new tree::Const(0), sequentialLabel,
                                            checkUpperLabel));
            stms->push_back(new tree::LabelStm(checkUpperLabel));
            stms->push_back(new tree::Cjump(">=", lowerExpr(
                                                Node{NodeKind::LVal,
                                                     plan.init.initExpr->loc,
                                                     reduction.var}),
                                            new tree::Const(reduction.modulus),
                                            sequentialLabel, parallelLabel));

            stms->push_back(new tree::LabelStm(parallelLabel));
            emitParallelRuntimeCall(plan, fields, workerName, beginTemp, endTemp,
                                    ivTemp, stms, sequentialLabel);
            emitParallelFinalIvUpdate(plan, ivTemp, beginTemp, endTemp, stms);
            stms->push_back(new tree::Jump(doneLabel));

            stms->push_back(new tree::LabelStm(sequentialLabel));
            lowerParallelSequentialFallback(
                plan, ivTemp,
                logicalRange || plan.inclusiveEnd ? rawEndTemp : endTemp,
                plan.comparison, stms);
            stms->push_back(new tree::Jump(doneLabel));
            stms->push_back(new tree::LabelStm(doneLabel));
        } else {
            emitParallelRuntimeCall(plan, fields, workerName, beginTemp, endTemp,
                                    ivTemp, stms);
            emitParallelFinalIvUpdate(plan, ivTemp, beginTemp, endTemp, stms);
            emitParallelScratchWritebacks(scratchWritebacks, beginTemp, endTemp, stms);
        }

        if (inclusiveDoneLabel != nullptr) {
            stms->push_back(new tree::LabelStm(inclusiveDoneLabel));
        }
        if (dynamicDoneLabel != nullptr) {
            stms->push_back(new tree::LabelStm(dynamicDoneLabel));
        }
        return true;
    }

    bool lowerStmt(const Node &node, std::vector<tree::Stm *> *stms) {
        switch (node.kind) {
        case NodeKind::Block:
            return lowerBlock(node, stms, true);
        case NodeKind::AssignStmt: {
            if (suppressedParallelIvUpdates_.find(&node) !=
                suppressedParallelIvUpdates_.end()) {
                return true;
            }
            if (node.children.size() == 2 &&
                node.children.front()->kind == NodeKind::LVal &&
                writeOnlyGlobalArrays_.count(node.children.front()->text) != 0) {
                Symbol destination = lookup(node.children.front()->text,
                                            node.children.front()->loc);
                if (destination.global &&
                    destination.label == globalLabel(node.children.front()->text)) {
                    // Candidate discovery proved that the indices and RHS are
                    // call-free, so removing the entire assignment preserves
                    // all source-visible effects.
                    return true;
                }
            }
            if (const Node *addend = workerModularAddend(node)) {
                // SysY integer addition wraps, so modular reassociation is
                // valid only while the source addition cannot overflow.  A
                // nonnegative addend no larger than INT_MAX-(m-1), together
                // with a canonical accumulator, proves that condition.  The
                // INT_MIN sentinel requests an exact sequential retry.
                auto *addendTemp = newTemp();
                stms->push_back(new tree::Move(tempExp(addendTemp),
                                                lowerExprAs(*addend, BaseType::Int)));
                auto *checkUpperLabel = newLabel();
                auto *safeLabel = newLabel();
                auto *unsafeLabel = newLabel();
                stms->push_back(new tree::Cjump("<", tempExp(addendTemp),
                                                new tree::Const(0), unsafeLabel,
                                                checkUpperLabel));
                stms->push_back(new tree::LabelStm(checkUpperLabel));
                stms->push_back(new tree::Cjump(
                    ">", tempExp(addendTemp),
                    new tree::Const(INT_MAX - (workerModulus_ - 1)),
                    unsafeLabel, safeLabel));
                stms->push_back(new tree::LabelStm(unsafeLabel));
                stms->push_back(new tree::Return(new tree::Const(INT_MIN)));
                stms->push_back(new tree::LabelStm(safeLabel));
                auto *dst = lowerLValue(*node.children.at(0));
                auto *mod = new tree::Const(workerModulus_);
                auto *addendResidue = intRemainder(
                    tempExp(addendTemp), new tree::Const(workerModulus_));
                auto *sum = new tree::Binop(tree::Type::INT, "+",
                                            lowerExpr(*node.children.at(0)),
                                            addendResidue);
                stms->push_back(new tree::Move(
                    dst, intRemainder(sum, mod)));
                return true;
            }
            auto *dst = lowerLValue(*node.children.at(0));
            BaseType dstBase = dst->type == tree::Type::FLOAT ? BaseType::Float : BaseType::Int;
            stms->push_back(new tree::Move(dst, lowerExprAs(*node.children.at(1), dstBase)));
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
                stms->push_back(new tree::Return(zero(currentReturnType_)));
            } else {
                stms->push_back(new tree::Return(lowerExprAs(*node.children.at(0), currentReturnType_)));
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
        if (sym.constScalar) {
            if (!node.children.empty()) {
                throw LoweringError(node.loc, "cannot index scalar const in native backend");
            }
            return new tree::Const(sym.constValue.raw, treeType(sym.base));
        }
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
                return new tree::Mem(treeType(sym.base), addr);
            }
            return addr;
        }
        if (!node.children.empty()) {
            throw LoweringError(node.loc, "cannot index scalar in native backend");
        }
        if (sym.global) {
            return new tree::Mem(treeType(sym.base), new tree::Name(new tree::String_Label(sym.label)));
        }
        return tempExp(sym.temp, sym.base);
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
                return new tree::Const(floatBits(parseFloatLiteral(node.text)), tree::Type::FLOAT);
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
            return new tree::Name(new tree::String_Label(stringLiteralLabel(node.text)));
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
            if (operand->type == tree::Type::FLOAT) {
                auto *args = new std::vector<tree::Exp *>({operand});
                return new tree::ExtCall(tree::Type::FLOAT, "__sysy_fneg_bits", args);
            }
            return new tree::Binop(tree::Type::INT, "-", zero(), operand);
        }
        if (node.text == "!") {
            auto *result = newTemp();
            auto *trueLabel = newLabel();
            auto *falseLabel = newLabel();
            auto *doneLabel = newLabel();
            auto *stms = new std::vector<tree::Stm *>();
            stms->push_back(new tree::Cjump("==", operand, zero(operand->type == tree::Type::FLOAT ? BaseType::Float : BaseType::Int),
                                            trueLabel, falseLabel));
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
            auto *lhsTemp = newTemp();
            auto *rhsTemp = newTemp();
            auto *stms = new std::vector<tree::Stm *>({
                new tree::Move(tempExp(lhsTemp), lowerExprAs(*node.children.at(0), BaseType::Int)),
                new tree::Move(tempExp(rhsTemp), lowerExprAs(*node.children.at(1), BaseType::Int))
            });
            auto *lhs = tempExp(lhsTemp);
            auto *rhs = tempExp(rhsTemp);
            auto *quot = new tree::Binop(tree::Type::INT, "/", lhs, rhs);
            auto *prod = new tree::Binop(tree::Type::INT, "*", quot, tempExp(rhsTemp));
            return new tree::Eseq(tree::Type::INT, new tree::Seq(stms),
                                  new tree::Binop(tree::Type::INT, "-", tempExp(lhsTemp), prod));
        }
        BaseType resultBase = exprBaseType(node);
        if (resultBase == BaseType::Float) {
            auto *args = new std::vector<tree::Exp *>({
                lowerExprAs(*node.children.at(0), BaseType::Float),
                lowerExprAs(*node.children.at(1), BaseType::Float)
            });
            std::string helper = "__sysy_fadd_bits";
            if (op == "-") {
                helper = "__sysy_fsub_bits";
            } else if (op == "*") {
                helper = "__sysy_fmul_bits";
            } else if (op == "/") {
                helper = "__sysy_fdiv_bits";
            }
            return new tree::ExtCall(tree::Type::FLOAT, helper, args);
        }
        return new tree::Binop(tree::Type::INT, op, lowerExprAs(*node.children.at(0), BaseType::Int),
                               lowerExprAs(*node.children.at(1), BaseType::Int));
    }

    tree::Exp *lowerComparison(const Node &node) {
        BaseType cmpBase = (exprBaseType(*node.children.at(0)) == BaseType::Float ||
                            exprBaseType(*node.children.at(1)) == BaseType::Float)
                               ? BaseType::Float
                               : BaseType::Int;
        if (cmpBase == BaseType::Float) {
            std::string helper = "__sysy_fcmpeq_bits";
            if (node.text == "!=") {
                helper = "__sysy_fcmpne_bits";
            } else if (node.text == "<") {
                helper = "__sysy_fcmplt_bits";
            } else if (node.text == ">") {
                helper = "__sysy_fcmpgt_bits";
            } else if (node.text == "<=") {
                helper = "__sysy_fcmple_bits";
            } else if (node.text == ">=") {
                helper = "__sysy_fcmpge_bits";
            }
            auto *args = new std::vector<tree::Exp *>({
                lowerExprAs(*node.children.at(0), BaseType::Float),
                lowerExprAs(*node.children.at(1), BaseType::Float)
            });
            return new tree::ExtCall(tree::Type::INT, helper, args);
        }
        auto *result = newTemp();
        auto *trueLabel = newLabel();
        auto *falseLabel = newLabel();
        auto *doneLabel = newLabel();
        auto *stms = new std::vector<tree::Stm *>();
        stms->push_back(new tree::Cjump(node.text, lowerExprAs(*node.children.at(0), cmpBase),
                                        lowerExprAs(*node.children.at(1), cmpBase), trueLabel, falseLabel));
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
        if (name == "putf") {
            if (node.children.empty() || node.children.front()->kind != NodeKind::StringLiteral) {
                throw LoweringError(node.loc, "putf requires a string literal format");
            }
            std::vector<char> specs = putfFormatSpecifiers(node.children.front()->text, node.children.front()->loc);
            if (specs.size() + 1 != node.children.size()) {
                throw LoweringError(node.loc, "putf argument count does not match format string");
            }

            std::string encodedName = "putf$";
            auto *args = new std::vector<tree::Exp *>({lowerExpr(*node.children.front())});
            for (std::size_t i = 0; i < specs.size(); ++i) {
                BaseType target = specs[i] == 'f' ? BaseType::Float : BaseType::Int;
                encodedName.push_back(specs[i]);
                args->push_back(lowerExprAs(*node.children.at(i + 1), target));
            }
            return new tree::ExtCall(tree::Type::INT, encodedName, args);
        }
        auto sigIt = functions_.find(name);
        FunctionSignature sig = sigIt == functions_.end() ? FunctionSignature{} : sigIt->second;
        auto *args = new std::vector<tree::Exp *>();
        for (std::size_t i = 0; i < node.children.size(); ++i) {
            BaseType target = i < sig.params.size() ? sig.params[i] : exprBaseType(*node.children[i]);
            int dims = i < sig.paramDims.size() ? sig.paramDims[i] : 0;
            if (dims > 0) {
                args->push_back(lowerExpr(*node.children[i]));
            } else {
                args->push_back(lowerExprAs(*node.children[i], target));
            }
        }
        return new tree::ExtCall(treeType(sig.ret), name, args);
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
            BaseType cmpBase = (exprBaseType(*node.children.at(0)) == BaseType::Float ||
                                exprBaseType(*node.children.at(1)) == BaseType::Float)
                                   ? BaseType::Float
                                   : BaseType::Int;
            if (cmpBase == BaseType::Float) {
                stms->push_back(new tree::Cjump("!=", lowerComparison(node), zero(), trueLabel, falseLabel));
                return;
            }
            stms->push_back(new tree::Cjump(node.text, lowerExprAs(*node.children.at(0), cmpBase),
                                            lowerExprAs(*node.children.at(1), cmpBase), trueLabel, falseLabel));
            return;
        }
        if (node.kind == NodeKind::UnaryExpr && node.text == "!") {
            emitCond(*node.children.at(0), falseLabel, trueLabel, stms);
            return;
        }
        BaseType condBase = exprBaseType(node);
        stms->push_back(new tree::Cjump("!=", lowerExpr(node), zero(condBase), trueLabel, falseLabel));
    }
};

} // namespace

LoweringError::LoweringError(SourceLocation loc, const std::string &message)
    : std::runtime_error(message), loc_(loc) {}

tree::Program *lowerToTree(const Node &root, const LoweringOptions &options) {
    Lowerer lowerer(options);
    return lowerer.lower(root);
}

std::string emitGlobalDataSection(const Node &root) {
    Lowerer lowerer;
    return lowerer.emitGlobalData(root);
}

} // namespace sysy
