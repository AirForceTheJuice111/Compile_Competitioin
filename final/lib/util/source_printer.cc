#include "source_printer.hh"

#include <sstream>
#include <variant>

using namespace std;
using namespace fdmj;

namespace {

string indent(int level) {
    return string(static_cast<size_t>(level) * 4, ' ');
}

string idToSource(const IdExp *id) {
    return id == nullptr ? "<null>" : id->id;
}

string intToSource(const IntExp *value) {
    return value == nullptr ? "0" : to_string(value->val);
}

ASTKind astKind(const AST *node) {
    return const_cast<AST *>(node)->getASTKind();
}

string typeToSource(const Type *type) {
    if (type == nullptr) return "int";
    switch (type->typeKind) {
    case TypeKind::INT:
        return "int";
    case TypeKind::ARRAY:
        if (type->arity != nullptr && type->arity->val != 0) {
            return "int[" + to_string(type->arity->val) + "]";
        }
        return "int[]";
    case TypeKind::CLASS:
        return "class " + idToSource(type->cid);
    }
    return "int";
}

string expToSource(const Exp *exp);

string expListToSource(const vector<Exp *> *args) {
    if (args == nullptr) return "";
    ostringstream out;
    for (size_t i = 0; i < args->size(); ++i) {
        if (i != 0) out << ", ";
        out << expToSource(args->at(i));
    }
    return out.str();
}

string expToSource(const Exp *exp) {
    if (exp == nullptr) return "0";

    switch (astKind(exp)) {
    case ASTKind::BinaryOp: {
        auto *node = static_cast<const BinaryOp *>(exp);
        return "(" + expToSource(node->left) + " " + expToSource(node->op) + " " + expToSource(node->right) + ")";
    }
    case ASTKind::UnaryOp: {
        auto *node = static_cast<const UnaryOp *>(exp);
        return "(" + expToSource(node->op) + expToSource(node->exp) + ")";
    }
    case ASTKind::ArrayExp: {
        auto *node = static_cast<const ArrayExp *>(exp);
        return expToSource(node->arr) + "[" + expToSource(node->index) + "]";
    }
    case ASTKind::CallExp: {
        auto *node = static_cast<const CallExp *>(exp);
        return expToSource(node->obj) + "." + idToSource(node->name) + "(" + expListToSource(node->par) + ")";
    }
    case ASTKind::ClassVar: {
        auto *node = static_cast<const ClassVar *>(exp);
        return expToSource(node->obj) + "." + idToSource(node->id);
    }
    case ASTKind::This:
        return "this";
    case ASTKind::Length: {
        auto *node = static_cast<const Length *>(exp);
        return "length(" + expToSource(node->exp) + ")";
    }
    case ASTKind::NewArray: {
        auto *node = static_cast<const NewArray *>(exp);
        return "new int[" + expToSource(node->size) + "]";
    }
    case ASTKind::NewObject: {
        auto *node = static_cast<const NewObject *>(exp);
        return "new " + idToSource(node->id) + "()";
    }
    case ASTKind::GetInt:
        return "getint()";
    case ASTKind::GetCh:
        return "getch()";
    case ASTKind::GetArray: {
        auto *node = static_cast<const GetArray *>(exp);
        return "getarray(" + expToSource(node->exp) + ")";
    }
    case ASTKind::IdExp:
        return static_cast<const IdExp *>(exp)->id;
    case ASTKind::IntExp:
        return to_string(static_cast<const IntExp *>(exp)->val);
    case ASTKind::OpExp:
        return static_cast<const OpExp *>(exp)->op;
    default:
        return "0";
    }
}

void writeStm(ostringstream &out, const Stm *stm, int level);

void writeNestedBody(ostringstream &out, const vector<Stm *> *stms, int level) {
    out << "{\n";
    if (stms != nullptr) {
        for (auto *stm : *stms) writeStm(out, stm, level + 1);
    }
    out << indent(level) << "}";
}

void writeBodyAfterHeader(ostringstream &out, const Stm *stm, int level) {
    if (stm != nullptr && astKind(stm) == ASTKind::Nested) {
        auto *nested = static_cast<const Nested *>(stm);
        writeNestedBody(out, nested->sl, level);
        out << "\n";
        return;
    }

    out << "{\n";
    if (stm != nullptr) {
        writeStm(out, stm, level + 1);
    }
    out << indent(level) << "}\n";
}

void writeStm(ostringstream &out, const Stm *stm, int level) {
    if (stm == nullptr) return;
    const string ind = indent(level);

    switch (astKind(stm)) {
    case ASTKind::Nested: {
        auto *node = static_cast<const Nested *>(stm);
        out << ind;
        writeNestedBody(out, node->sl, level);
        out << "\n";
        break;
    }
    case ASTKind::If: {
        auto *node = static_cast<const If *>(stm);
        out << ind << "if (" << expToSource(node->exp) << ") ";
        writeBodyAfterHeader(out, node->stm1, level);
        if (node->stm2 != nullptr) {
            out << ind << "else ";
            writeBodyAfterHeader(out, node->stm2, level);
        }
        break;
    }
    case ASTKind::While: {
        auto *node = static_cast<const While *>(stm);
        out << ind << "while (" << expToSource(node->exp) << ")";
        if (node->stm == nullptr) {
            out << ";\n";
        } else {
            out << " ";
            writeBodyAfterHeader(out, node->stm, level);
        }
        break;
    }
    case ASTKind::Assign: {
        auto *node = static_cast<const Assign *>(stm);
        out << ind << expToSource(node->left) << " = " << expToSource(node->exp) << ";\n";
        break;
    }
    case ASTKind::CallStm: {
        auto *node = static_cast<const CallStm *>(stm);
        out << ind << expToSource(node->obj) << "." << idToSource(node->name) << "(" << expListToSource(node->par) << ");\n";
        break;
    }
    case ASTKind::Continue:
        out << ind << "continue;\n";
        break;
    case ASTKind::Break:
        out << ind << "break;\n";
        break;
    case ASTKind::Return: {
        auto *node = static_cast<const Return *>(stm);
        out << ind << "return " << expToSource(node->exp) << ";\n";
        break;
    }
    case ASTKind::PutInt: {
        auto *node = static_cast<const PutInt *>(stm);
        out << ind << "putint(" << expToSource(node->exp) << ");\n";
        break;
    }
    case ASTKind::PutCh: {
        auto *node = static_cast<const PutCh *>(stm);
        out << ind << "putch(" << expToSource(node->exp) << ");\n";
        break;
    }
    case ASTKind::PutArray: {
        auto *node = static_cast<const PutArray *>(stm);
        out << ind << "putarray(" << expToSource(node->n) << ", " << expToSource(node->arr) << ");\n";
        break;
    }
    case ASTKind::Starttime:
        out << ind << "starttime();\n";
        break;
    case ASTKind::Stoptime:
        out << ind << "stoptime();\n";
        break;
    default:
        break;
    }
}

void writeVarDecl(ostringstream &out, const VarDecl *decl, int level) {
    if (decl == nullptr) return;
    out << indent(level) << typeToSource(decl->type) << " " << idToSource(decl->id);
    if (holds_alternative<IntExp *>(decl->init)) {
        out << " = " << intToSource(get<IntExp *>(decl->init));
    } else if (holds_alternative<vector<IntExp *> *>(decl->init)) {
        out << " = {";
        auto *values = get<vector<IntExp *> *>(decl->init);
        if (values != nullptr) {
            for (size_t i = 0; i < values->size(); ++i) {
                if (i != 0) out << ", ";
                out << intToSource(values->at(i));
            }
        }
        out << "}";
    }
    out << ";\n";
}

void writeFormalList(ostringstream &out, const vector<Formal *> *formals) {
    if (formals == nullptr) return;
    for (size_t i = 0; i < formals->size(); ++i) {
        auto *formal = formals->at(i);
        if (i != 0) out << ", ";
        out << typeToSource(formal->type) << " " << idToSource(formal->id);
    }
}

void writeMethodDecl(ostringstream &out, const MethodDecl *method, int level) {
    if (method == nullptr) return;
    out << indent(level) << "public " << typeToSource(method->type) << " " << idToSource(method->id) << "(";
    writeFormalList(out, method->fl);
    out << ") {\n";
    if (method->vdl != nullptr) {
        for (auto *decl : *method->vdl) writeVarDecl(out, decl, level + 1);
    }
    if (method->sl != nullptr) {
        for (auto *stm : *method->sl) writeStm(out, stm, level + 1);
    }
    out << indent(level) << "}\n";
}

void writeMainMethod(ostringstream &out, const MainMethod *main) {
    out << "public int main() {\n";
    if (main != nullptr && main->vdl != nullptr) {
        for (auto *decl : *main->vdl) writeVarDecl(out, decl, 1);
    }
    if (main != nullptr && main->sl != nullptr) {
        for (auto *stm : *main->sl) writeStm(out, stm, 1);
    }
    out << "}\n";
}

void writeClassDecl(ostringstream &out, const ClassDecl *klass) {
    if (klass == nullptr) return;
    out << "public class " << idToSource(klass->id);
    if (klass->eid != nullptr) out << " extends " << idToSource(klass->eid);
    out << " {\n";
    if (klass->vdl != nullptr) {
        for (auto *decl : *klass->vdl) writeVarDecl(out, decl, 1);
    }
    if (klass->mdl != nullptr) {
        for (size_t i = 0; i < klass->mdl->size(); ++i) {
            if (i != 0 || (klass->vdl != nullptr && !klass->vdl->empty())) out << "\n";
            writeMethodDecl(out, klass->mdl->at(i), 1);
        }
    }
    out << "}\n";
}

} // namespace

string astToSource(const Program *program) {
    ostringstream out;
    if (program == nullptr) return "";

    writeMainMethod(out, program->main);
    if (program->cdl != nullptr) {
        for (auto *klass : *program->cdl) {
            out << "\n";
            writeClassDecl(out, klass);
        }
    }
    return out.str();
}
