#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "ASTheader.hh"
#include "FDMJAST.hh"
#include "semant.hh"
#include "xml2ast.hh"

using namespace std;
namespace fs = std::filesystem;

#ifndef DEFAULT_PARSER
#define DEFAULT_PARSER "vendor/parser/parser"
#endif

namespace {

string shellQuote(const string &s) {
    string out = "'";
    for (char c : s) {
        if (c == '\'') out += "'\\''";
        else out += c;
    }
    out += "'";
    return out;
}

string stripSuffix(const string &path, const string &suffix) {
    if (path.size() >= suffix.size() && path.substr(path.size() - suffix.size()) == suffix) {
        return path.substr(0, path.size() - suffix.size());
    }
    return path;
}

int runParser(const string &base, const string &parserOverride) {
    const char *envParser = std::getenv("FMJ_PARSER");
    string parser = !parserOverride.empty() ? parserOverride : (envParser != nullptr ? envParser : DEFAULT_PARSER);
    string command = shellQuote(parser) + " " + shellQuote(base) + " 1>&2";
    return std::system(command.c_str());
}

int32_t wrapAdd(int32_t a, int32_t b) {
    return static_cast<int32_t>(static_cast<uint32_t>(a) + static_cast<uint32_t>(b));
}

int32_t wrapSub(int32_t a, int32_t b) {
    return static_cast<int32_t>(static_cast<uint32_t>(a) - static_cast<uint32_t>(b));
}

int32_t wrapMul(int32_t a, int32_t b) {
    return static_cast<int32_t>(static_cast<uint32_t>(a) * static_cast<uint32_t>(b));
}

struct ArrayValue;
struct ObjectValue;

struct Value {
    enum class Kind { INT, ARRAY, OBJECT, NIL };

    Kind kind = Kind::NIL;
    int32_t intValue = 0;
    shared_ptr<ArrayValue> arrayValue;
    shared_ptr<ObjectValue> objectValue;

    static Value intv(int32_t v) {
        Value out;
        out.kind = Kind::INT;
        out.intValue = v;
        return out;
    }

    static Value arrayv(shared_ptr<ArrayValue> v) {
        Value out;
        out.kind = v ? Kind::ARRAY : Kind::NIL;
        out.arrayValue = std::move(v);
        return out;
    }

    static Value objectv(shared_ptr<ObjectValue> v) {
        Value out;
        out.kind = v ? Kind::OBJECT : Kind::NIL;
        out.objectValue = std::move(v);
        return out;
    }

    static Value nil() { return Value(); }
};

struct ArrayValue {
    vector<int32_t> data;
};

struct ObjectValue {
    string className;
    map<string, Value> fields;
};

struct ReturnSignal {
    Value value;
};

struct BreakSignal {};
struct ContinueSignal {};

struct RuntimeExit {
    int32_t code;
};

struct Frame {
    string className;
    string methodName;
    shared_ptr<ObjectValue> thisObject;
    unordered_map<string, Value> locals;
};

class IOState {
  public:
    bool fuzzMode = false;
    string fuzzKind = "generic";
    uint32_t rngState = 0x5eed1234u;
    unsigned long long inputCalls = 0;
    int inputSlot = 0;

    uint64_t hashState = 1469598103934665603ull;
    bool hashMode = false;

    explicit IOState(istream *in)
        : input(in) {}

    void resetIteration() {
        inputSlot = 0;
    }

    int32_t getInt() {
        inputCalls++;
        if (fuzzMode) return generatedScalar();
        int32_t value = 0;
        if (!((*input) >> value)) return 0;
        return value;
    }

    int32_t getCh() {
        inputCalls++;
        if (fuzzMode) return generatedScalar();
        char c = 0;
        if (!input->get(c)) return 0;
        return static_cast<unsigned char>(c);
    }

    int32_t getArray(const shared_ptr<ArrayValue> &array) {
        if (!array) throw RuntimeExit{-1};
        int32_t n = 0;
        if (fuzzMode) {
            n = rangeInt(0, 10);
            inputCalls += static_cast<unsigned long long>(n) + 1ull;
            array->data.assign(static_cast<size_t>(n), 0);
            for (int32_t i = 0; i < n; ++i) array->data[static_cast<size_t>(i)] = rangeInt(-32, 32);
            return n;
        }

        inputCalls++;
        if (!((*input) >> n)) n = 0;
        if (n < 0) n = 0;
        array->data.assign(static_cast<size_t>(n), 0);
        for (int32_t i = 0; i < n; ++i) {
            int32_t value = 0;
            inputCalls++;
            if (!((*input) >> value)) value = 0;
            array->data[static_cast<size_t>(i)] = value;
        }
        return n;
    }

    void putInt(int32_t value) {
        if (hashMode) {
            hashU32(0x70757469u);
            hashU32(static_cast<uint32_t>(value));
            return;
        }
        (*output) << value;
    }

    void putCh(int32_t value) {
        if (hashMode) {
            hashU32(0x70757463u);
            hashU32(static_cast<uint32_t>(static_cast<unsigned char>(value)));
            return;
        }
        (*output) << static_cast<char>(static_cast<unsigned char>(value));
    }

    void putArray(int32_t n, const shared_ptr<ArrayValue> &array) {
        if (!array) throw RuntimeExit{-1};
        if (hashMode) {
            hashU32(0x70757461u);
            hashU32(static_cast<uint32_t>(n));
            for (int32_t i = 0; i < n; ++i) {
                int32_t value = 0;
                if (i >= 0 && static_cast<size_t>(i) < array->data.size())
                    value = array->data[static_cast<size_t>(i)];
                hashU32(static_cast<uint32_t>(value));
            }
            return;
        }
        (*output) << n << ":";
        for (int32_t i = 0; i < n; ++i) {
            int32_t value = 0;
            if (i >= 0 && static_cast<size_t>(i) < array->data.size())
                value = array->data[static_cast<size_t>(i)];
            (*output) << " " << value;
        }
        (*output) << "\n";
    }

    void hashReturn(int32_t rc, bool exitTaken) {
        hashU32(0x72657475u);
        hashU32(static_cast<uint32_t>(rc));
        hashU32(exitTaken ? 1u : 0u);
    }

    void printHashLine() {
        ios oldState(nullptr);
        oldState.copyfmt(cout);
        cout << hex << nouppercase << setw(16) << setfill('0') << hashState
             << dec << " " << inputCalls << "\n";
        cout.copyfmt(oldState);
    }

  private:
    istream *input = nullptr;
    ostream *output = &cout;

    void hashU32(uint32_t value) {
        hashState ^= value;
        hashState *= 1099511628211ull;
    }

    uint32_t nextU32() {
        uint32_t x = rngState;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        rngState = x ? x : 0x9e3779b9u;
        return rngState;
    }

    int32_t rangeInt(int32_t lo, int32_t hi) {
        uint32_t span = static_cast<uint32_t>(hi - lo + 1);
        return lo + static_cast<int32_t>(nextU32() % span);
    }

    int32_t generatedScalar() {
        int slot = inputSlot++;

        if (fuzzKind == "fibonacci") return rangeInt(-5, 12);
        if (fuzzKind == "insttest4") return rangeInt(-32, 32);
        if (fuzzKind == "loop12") return rangeInt(-20, 80);
        if (fuzzKind == "loop3") return (slot % 2) == 0 ? rangeInt(-20, 80) : rangeInt(1, 20);
        if (fuzzKind == "loop56") return rangeInt(-32, 32);
        if (fuzzKind == "extra2") return rangeInt(-20, 80);
        if (fuzzKind == "extra3") return (slot % 2) == 0 ? rangeInt(-20, 80) : rangeInt(1, 20);
        if (fuzzKind == "extra45") return (slot % 2) == 0 ? rangeInt(-20, 80) : rangeInt(0, 20);
        if (fuzzKind == "positive") return rangeInt(1, 20);
        if (fuzzKind == "comprehensive") {
            int32_t v = rangeInt(-8, 12);
            return (v == 3 || v == 4) ? 5 : v;
        }
        return rangeInt(-32, 32);
    }
};

class Interpreter {
  public:
    Interpreter(fdmj::Program *program, AST_Semant_Map *semantMap, IOState *io)
        : program(program), semantMap(semantMap), nameMaps(semantMap->getNameMaps()), io(io) {
        indexProgram();
    }

    int32_t runMain(bool *exitTaken) {
        *exitTaken = false;
        try {
            return runMainOnce();
        } catch (const RuntimeExit &e) {
            *exitTaken = true;
            return e.code;
        }
    }

  private:
    fdmj::Program *program = nullptr;
    AST_Semant_Map *semantMap = nullptr;
    Name_Maps *nameMaps = nullptr;
    IOState *io = nullptr;

    map<string, fdmj::ClassDecl *> classes;
    map<pair<string, string>, fdmj::MethodDecl *> methods;

    void indexProgram() {
        if (program == nullptr || program->cdl == nullptr) return;
        for (auto *klass : *program->cdl) {
            if (klass == nullptr || klass->id == nullptr) continue;
            classes[klass->id->id] = klass;
            if (klass->mdl == nullptr) continue;
            for (auto *method : *klass->mdl) {
                if (method == nullptr || method->id == nullptr) continue;
                methods[{klass->id->id, method->id->id}] = method;
            }
        }
    }

    int32_t runMainOnce() {
        Frame frame;
        frame.className = "__$main__";
        frame.methodName = "main";
        initVars(program->main != nullptr ? program->main->vdl : nullptr, frame);

        try {
            execList(program->main != nullptr ? program->main->sl : nullptr, frame);
        } catch (const ReturnSignal &ret) {
            return asInt(ret.value);
        }
        return 0;
    }

    void initVars(vector<fdmj::VarDecl *> *vars, Frame &frame) {
        if (vars == nullptr) return;
        for (auto *var : *vars) {
            if (var == nullptr || var->id == nullptr) continue;
            frame.locals[var->id->id] = initialValue(var);
        }
    }

    Value initialValue(fdmj::VarDecl *decl) {
        if (decl == nullptr || decl->type == nullptr) return Value::nil();
        if (decl->type->typeKind == fdmj::TypeKind::INT) {
            if (holds_alternative<fdmj::IntExp *>(decl->init)) {
                auto *init = get<fdmj::IntExp *>(decl->init);
                return Value::intv(init != nullptr ? init->val : 0);
            }
            return Value::intv(0);
        }
        if (decl->type->typeKind == fdmj::TypeKind::ARRAY) {
            if (holds_alternative<vector<fdmj::IntExp *> *>(decl->init)) {
                auto *init = get<vector<fdmj::IntExp *> *>(decl->init);
                if (init != nullptr) return Value::arrayv(makeArray(init));
            }
            return Value::nil();
        }
        return Value::nil();
    }

    shared_ptr<ArrayValue> makeArray(vector<fdmj::IntExp *> *init) {
        auto array = make_shared<ArrayValue>();
        if (init != nullptr) {
            array->data.reserve(init->size());
            for (auto *item : *init) array->data.push_back(item != nullptr ? item->val : 0);
        }
        return array;
    }

    shared_ptr<ArrayValue> makeArray(int32_t size) {
        if (size < 0) throw RuntimeExit{-1};
        auto array = make_shared<ArrayValue>();
        array->data.assign(static_cast<size_t>(size), 0);
        return array;
    }

    shared_ptr<ObjectValue> makeObject(const string &className) {
        auto object = make_shared<ObjectValue>();
        object->className = className;

        vector<string> chain;
        for (string cur = className; !cur.empty(); cur = nameMaps->get_parent(cur))
            chain.push_back(cur);
        reverse(chain.begin(), chain.end());

        for (const auto &klass : chain) {
            auto *vars = nameMaps->get_class_var_list(klass);
            if (vars == nullptr) continue;
            for (const auto &field : *vars) {
                auto *decl = nameMaps->get_class_var(klass, field);
                object->fields[fieldKey(klass, field)] = initialValue(decl);
            }
            delete vars;
        }

        return object;
    }

    static string fieldKey(const string &className, const string &fieldName) {
        return className + "^" + fieldName;
    }

    string resolveFieldClass(const string &staticClass, const string &fieldName) {
        for (string cur = staticClass; !cur.empty(); cur = nameMaps->get_parent(cur)) {
            if (nameMaps->is_class_var(cur, fieldName)) return cur;
        }
        throw RuntimeExit{-1};
    }

    fdmj::MethodDecl *resolveMethod(const string &dynamicClass, const string &methodName, string *declaringClass) {
        for (string cur = dynamicClass; !cur.empty(); cur = nameMaps->get_parent(cur)) {
            auto it = methods.find({cur, methodName});
            if (it != methods.end()) {
                if (declaringClass != nullptr) *declaringClass = cur;
                return it->second;
            }
        }
        throw RuntimeExit{-1};
    }

    string staticClassOf(fdmj::Exp *expr, Frame &frame) {
        if (expr == nullptr) return "";
        if (expr->getASTKind() == fdmj::ASTKind::This) return frame.className;
        auto *sem = semantMap->getSemant(expr);
        if (sem != nullptr && sem->get_type() == fdmj::TypeKind::CLASS) {
            auto par = sem->get_type_par();
            if (holds_alternative<string>(par)) return get<string>(par);
        }
        return "";
    }

    int32_t asInt(const Value &value) {
        if (value.kind == Value::Kind::INT) return value.intValue;
        return 0;
    }

    shared_ptr<ArrayValue> asArray(const Value &value) {
        if (value.kind != Value::Kind::ARRAY || !value.arrayValue) throw RuntimeExit{-1};
        return value.arrayValue;
    }

    shared_ptr<ObjectValue> asObject(const Value &value) {
        if (value.kind != Value::Kind::OBJECT || !value.objectValue) throw RuntimeExit{-1};
        return value.objectValue;
    }

    void checkArrayIndex(const shared_ptr<ArrayValue> &array, int32_t index) {
        if (!array || index < 0 || static_cast<size_t>(index) >= array->data.size())
            throw RuntimeExit{-1};
    }

    void execList(vector<fdmj::Stm *> *statements, Frame &frame) {
        if (statements == nullptr) return;
        for (auto *stm : *statements) execStm(stm, frame);
    }

    void execStm(fdmj::Stm *stm, Frame &frame) {
        if (stm == nullptr) return;

        switch (stm->getASTKind()) {
        case fdmj::ASTKind::Nested: {
            auto *node = static_cast<fdmj::Nested *>(stm);
            execList(node->sl, frame);
            return;
        }
        case fdmj::ASTKind::If: {
            auto *node = static_cast<fdmj::If *>(stm);
            if (asInt(evalExp(node->exp, frame)) != 0) execStm(node->stm1, frame);
            else execStm(node->stm2, frame);
            return;
        }
        case fdmj::ASTKind::While: {
            auto *node = static_cast<fdmj::While *>(stm);
            while (asInt(evalExp(node->exp, frame)) != 0) {
                try {
                    execStm(node->stm, frame);
                } catch (const ContinueSignal &) {
                    continue;
                } catch (const BreakSignal &) {
                    break;
                }
            }
            return;
        }
        case fdmj::ASTKind::Assign: {
            auto *node = static_cast<fdmj::Assign *>(stm);
            LValue dst = evalLValue(node->left, frame);
            Value src = evalExp(node->exp, frame);
            dst.set(src);
            return;
        }
        case fdmj::ASTKind::CallStm: {
            auto *node = static_cast<fdmj::CallStm *>(stm);
            evalCall(node->obj, node->name, node->par, frame);
            return;
        }
        case fdmj::ASTKind::Continue:
            throw ContinueSignal{};
        case fdmj::ASTKind::Break:
            throw BreakSignal{};
        case fdmj::ASTKind::Return: {
            auto *node = static_cast<fdmj::Return *>(stm);
            throw ReturnSignal{node->exp != nullptr ? evalExp(node->exp, frame) : Value::intv(0)};
        }
        case fdmj::ASTKind::PutInt: {
            auto *node = static_cast<fdmj::PutInt *>(stm);
            io->putInt(asInt(evalExp(node->exp, frame)));
            return;
        }
        case fdmj::ASTKind::PutCh: {
            auto *node = static_cast<fdmj::PutCh *>(stm);
            io->putCh(asInt(evalExp(node->exp, frame)));
            return;
        }
        case fdmj::ASTKind::PutArray: {
            auto *node = static_cast<fdmj::PutArray *>(stm);
            int32_t n = asInt(evalExp(node->n, frame));
            auto array = asArray(evalExp(node->arr, frame));
            io->putArray(n, array);
            return;
        }
        case fdmj::ASTKind::Starttime:
        case fdmj::ASTKind::Stoptime:
            return;
        default:
            throw RuntimeExit{-1};
        }
    }

    struct LValue {
        enum class Kind { LOCAL, FIELD, ARRAY_ELEM };
        Kind kind = Kind::LOCAL;
        Frame *frame = nullptr;
        string name;
        shared_ptr<ObjectValue> object;
        string key;
        shared_ptr<ArrayValue> array;
        int32_t index = 0;

        Value get() const {
            if (kind == Kind::LOCAL) return frame->locals[name];
            if (kind == Kind::FIELD) return object->fields[key];
            if (kind == Kind::ARRAY_ELEM) return Value::intv(array->data[static_cast<size_t>(index)]);
            return Value::nil();
        }

        void set(const Value &value) {
            if (kind == Kind::LOCAL) frame->locals[name] = value;
            else if (kind == Kind::FIELD) object->fields[key] = value;
            else if (kind == Kind::ARRAY_ELEM) array->data[static_cast<size_t>(index)] = asStoredInt(value);
        }

        static int32_t asStoredInt(const Value &value) {
            return value.kind == Value::Kind::INT ? value.intValue : 0;
        }
    };

    LValue evalLValue(fdmj::Exp *expr, Frame &frame) {
        if (expr == nullptr) throw RuntimeExit{-1};
        switch (expr->getASTKind()) {
        case fdmj::ASTKind::IdExp: {
            auto *id = static_cast<fdmj::IdExp *>(expr);
            if (frame.locals.find(id->id) == frame.locals.end()) throw RuntimeExit{-1};
            LValue out;
            out.kind = LValue::Kind::LOCAL;
            out.frame = &frame;
            out.name = id->id;
            return out;
        }
        case fdmj::ASTKind::ClassVar: {
            auto *node = static_cast<fdmj::ClassVar *>(expr);
            auto object = asObject(evalExp(node->obj, frame));
            string staticClass = staticClassOf(node->obj, frame);
            string declaringClass = resolveFieldClass(staticClass, node->id->id);
            LValue out;
            out.kind = LValue::Kind::FIELD;
            out.object = object;
            out.key = fieldKey(declaringClass, node->id->id);
            if (object->fields.find(out.key) == object->fields.end()) throw RuntimeExit{-1};
            return out;
        }
        case fdmj::ASTKind::ArrayExp: {
            auto *node = static_cast<fdmj::ArrayExp *>(expr);
            auto array = asArray(evalExp(node->arr, frame));
            int32_t index = asInt(evalExp(node->index, frame));
            checkArrayIndex(array, index);
            LValue out;
            out.kind = LValue::Kind::ARRAY_ELEM;
            out.array = array;
            out.index = index;
            return out;
        }
        default:
            throw RuntimeExit{-1};
        }
    }

    Value evalExp(fdmj::Exp *expr, Frame &frame) {
        if (expr == nullptr) return Value::intv(0);

        switch (expr->getASTKind()) {
        case fdmj::ASTKind::BinaryOp:
            return evalBinary(static_cast<fdmj::BinaryOp *>(expr), frame);
        case fdmj::ASTKind::UnaryOp:
            return evalUnary(static_cast<fdmj::UnaryOp *>(expr), frame);
        case fdmj::ASTKind::ArrayExp:
            return evalLValue(expr, frame).get();
        case fdmj::ASTKind::CallExp: {
            auto *node = static_cast<fdmj::CallExp *>(expr);
            return evalCall(node->obj, node->name, node->par, frame);
        }
        case fdmj::ASTKind::ClassVar:
            return evalLValue(expr, frame).get();
        case fdmj::ASTKind::This:
            return Value::objectv(frame.thisObject);
        case fdmj::ASTKind::Length: {
            auto *node = static_cast<fdmj::Length *>(expr);
            auto array = asArray(evalExp(node->exp, frame));
            return Value::intv(static_cast<int32_t>(array->data.size()));
        }
        case fdmj::ASTKind::NewArray: {
            auto *node = static_cast<fdmj::NewArray *>(expr);
            return Value::arrayv(makeArray(asInt(evalExp(node->size, frame))));
        }
        case fdmj::ASTKind::NewObject: {
            auto *node = static_cast<fdmj::NewObject *>(expr);
            return Value::objectv(makeObject(node->id->id));
        }
        case fdmj::ASTKind::GetInt:
            return Value::intv(io->getInt());
        case fdmj::ASTKind::GetCh:
            return Value::intv(io->getCh());
        case fdmj::ASTKind::GetArray: {
            auto *node = static_cast<fdmj::GetArray *>(expr);
            auto dst = evalLValue(node->exp, frame);
            auto array = asArray(dst.get());
            return Value::intv(io->getArray(array));
        }
        case fdmj::ASTKind::IdExp: {
            auto *id = static_cast<fdmj::IdExp *>(expr);
            auto it = frame.locals.find(id->id);
            if (it == frame.locals.end()) throw RuntimeExit{-1};
            return it->second;
        }
        case fdmj::ASTKind::IntExp:
            return Value::intv(static_cast<fdmj::IntExp *>(expr)->val);
        default:
            throw RuntimeExit{-1};
        }
    }

    Value evalBinary(fdmj::BinaryOp *node, Frame &frame) {
        string op = node->op != nullptr ? node->op->op : "";
        if (op == "&&") {
            int32_t left = asInt(evalExp(node->left, frame));
            if (left == 0) return Value::intv(0);
            return Value::intv(asInt(evalExp(node->right, frame)) != 0 ? 1 : 0);
        }
        if (op == "||") {
            int32_t left = asInt(evalExp(node->left, frame));
            if (left != 0) return Value::intv(1);
            return Value::intv(asInt(evalExp(node->right, frame)) != 0 ? 1 : 0);
        }

        int32_t left = asInt(evalExp(node->left, frame));
        int32_t right = asInt(evalExp(node->right, frame));

        if (op == "+") return Value::intv(wrapAdd(left, right));
        if (op == "-") return Value::intv(wrapSub(left, right));
        if (op == "*") return Value::intv(wrapMul(left, right));
        if (op == "/") {
            if (right == 0) throw RuntimeExit{-1};
            if (left == INT32_MIN && right == -1) return Value::intv(INT32_MIN);
            return Value::intv(left / right);
        }
        if (op == "<") return Value::intv(left < right ? 1 : 0);
        if (op == ">") return Value::intv(left > right ? 1 : 0);
        if (op == "<=") return Value::intv(left <= right ? 1 : 0);
        if (op == ">=") return Value::intv(left >= right ? 1 : 0);
        if (op == "==") return Value::intv(left == right ? 1 : 0);
        if (op == "!=") return Value::intv(left != right ? 1 : 0);
        throw RuntimeExit{-1};
    }

    Value evalUnary(fdmj::UnaryOp *node, Frame &frame) {
        string op = node->op != nullptr ? node->op->op : "";
        int32_t value = asInt(evalExp(node->exp, frame));
        if (op == "-") return Value::intv(wrapSub(0, value));
        if (op == "!") return Value::intv(value == 0 ? 1 : 0);
        return Value::intv(value);
    }

    Value evalCall(fdmj::Exp *objExp, fdmj::IdExp *name, vector<fdmj::Exp *> *args, Frame &caller) {
        auto object = asObject(evalExp(objExp, caller));
        string declaringClass;
        auto *method = resolveMethod(object->className, name != nullptr ? name->id : "", &declaringClass);

        vector<Value> argValues;
        if (args != nullptr) {
            for (auto *arg : *args) argValues.push_back(evalExp(arg, caller));
        }

        return runMethod(object, declaringClass, method, argValues);
    }

    Value runMethod(const shared_ptr<ObjectValue> &object, const string &declaringClass,
                    fdmj::MethodDecl *method, const vector<Value> &args) {
        Frame frame;
        frame.className = declaringClass;
        frame.methodName = method != nullptr && method->id != nullptr ? method->id->id : "";
        frame.thisObject = object;

        if (method != nullptr && method->fl != nullptr) {
            for (size_t i = 0; i < method->fl->size(); ++i) {
                auto *formal = method->fl->at(i);
                if (formal == nullptr || formal->id == nullptr) continue;
                frame.locals[formal->id->id] = i < args.size() ? args[i] : Value::intv(0);
            }
        }
        initVars(method != nullptr ? method->vdl : nullptr, frame);

        try {
            execList(method != nullptr ? method->sl : nullptr, frame);
        } catch (const ReturnSignal &ret) {
            return ret.value;
        }
        return defaultReturn(method != nullptr ? method->type : nullptr);
    }

    Value defaultReturn(fdmj::Type *type) {
        if (type != nullptr && type->typeKind == fdmj::TypeKind::INT) return Value::intv(0);
        return Value::nil();
    }
};

uint32_t parseU32(const string &text) {
    size_t idx = 0;
    unsigned long value = stoul(text, &idx, 0);
    if (idx != text.size()) throw invalid_argument("bad integer");
    return static_cast<uint32_t>(value);
}

unsigned long long parseU64(const string &text) {
    size_t idx = 0;
    unsigned long long value = stoull(text, &idx, 0);
    if (idx != text.size()) throw invalid_argument("bad integer");
    return value;
}

void usage(const char *argv0) {
    cerr << "Usage: " << argv0
         << " [--check] [--fuzz ITERS] [--seed SEED] [--kind KIND] [--parser PATH] <file.fmj|base>\n";
}

} // namespace

int main(int argc, char **argv) {
    bool checkOnly = false;
    bool fuzz = false;
    unsigned long long iterations = 1;
    uint32_t seed = 0x5eed1234u;
    string kind = "generic";
    string parserOverride;
    string inputPath;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        try {
            if (arg == "--check") {
                checkOnly = true;
            } else if (arg == "--fuzz" && i + 1 < argc) {
                fuzz = true;
                iterations = parseU64(argv[++i]);
            } else if (arg == "--seed" && i + 1 < argc) {
                seed = parseU32(argv[++i]);
            } else if (arg == "--kind" && i + 1 < argc) {
                kind = argv[++i];
            } else if (arg == "--parser" && i + 1 < argc) {
                parserOverride = argv[++i];
            } else if (arg == "--help" || arg == "-h") {
                usage(argv[0]);
                return 0;
            } else if (inputPath.empty()) {
                inputPath = arg;
            } else {
                cerr << "Unexpected argument: " << arg << "\n";
                usage(argv[0]);
                return 2;
            }
        } catch (const exception &) {
            cerr << "Invalid value for " << arg << "\n";
            return 2;
        }
    }

    if (inputPath.empty()) {
        usage(argv[0]);
        return 2;
    }

    string base = stripSuffix(inputPath, ".fmj");
    if (runParser(base, parserOverride) != 0) {
        cerr << "Error: parser rejected input " << base << ".fmj\n";
        return 1;
    }

    AST_Semant_Map *parsedSemant = nullptr;
    fdmj::Program *root = xml2ast(base + ".2.ast", &parsedSemant);
    if (root == nullptr) {
        cerr << "Error: failed to read parser AST\n";
        return 1;
    }

    AST_Semant_Map *semantMap = semant_analyze(root);
    if (semantMap == nullptr) {
        cerr << "Error: semantic analysis failed\n";
        return 1;
    }

    if (checkOnly) return 0;

    IOState io(&cin);
    io.fuzzMode = fuzz;
    io.hashMode = fuzz;
    io.fuzzKind = kind;
    io.rngState = seed;

    Interpreter interp(root, semantMap, &io);

    if (fuzz) {
        for (unsigned long long i = 0; i < iterations; ++i) {
            io.resetIteration();
            bool exitTaken = false;
            int32_t rc = interp.runMain(&exitTaken);
            io.hashReturn(rc, exitTaken);
        }
        io.printHashLine();
        return 0;
    }

    bool exitTaken = false;
    int32_t rc = interp.runMain(&exitTaken);
    (void)exitTaken;
    return static_cast<unsigned char>(rc);
}
