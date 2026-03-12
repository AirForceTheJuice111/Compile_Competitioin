#define DEBUG
#undef DEBUG

#include "ASTheader.hh"
#include "FDMJAST.hh"
#include "namemaps.hh"
#include <algorithm>
#include <iostream>
#include <map>
#include <variant>
#include <vector>

using namespace std;
using namespace fdmj;

// 遍历 Program 节点：
// 第一遍：先注册所有类名（解决前向引用问题）
// 第二遍：处理继承关系、类变量、方法等细节
void AST_Name_Map_Visitor::visit(Program *node) {
#ifdef DEBUG
    std::cout << "Visiting Program" << std::endl;
#endif
    if (node == nullptr) return;

    // 第一遍：注册所有类名
    if (node->cdl != nullptr) {
        for (auto cl : *(node->cdl)) {
            if (cl != nullptr && cl->id != nullptr) {
                if (!name_maps->add_class(cl->id->id)) {
                    cerr << "Error: at position " << cl->getPos()->print() << endl;
                    cerr << "Error: Duplicate class name: " << cl->id->id << endl;
                }
            }
        }
    }

    // 第二遍：处理继承关系
    if (node->cdl != nullptr) {
        for (auto cl : *(node->cdl)) {
            if (cl != nullptr && cl->eid != nullptr) {
                string child = cl->id->id;
                string parent = cl->eid->id;
                if (!name_maps->is_class(parent)) {
                    cerr << "Error: at position " << cl->getPos()->print() << endl;
                    cerr << "Error: Parent class not found: " << parent << endl;
                } else {
                    name_maps->add_class_hiearchy(child, parent);
                }
            }
        }
    }

    // 访问 main 方法
    if (node->main != nullptr) node->main->accept(*this);

    // 第三遍：遍历类的内部（变量、方法）
    if (node->cdl != nullptr)
        for (auto cl : *(node->cdl)) cl->accept(*this);
}

// MainMethod：以特殊类名 "__main__" 和方法名 "main" 注册，
// 收集局部变量声明
void AST_Name_Map_Visitor::visit(MainMethod *node) {
#ifdef DEBUG
    std::cout << "Visiting MainMethod" << std::endl;
#endif
    if (node == nullptr) return;

    // 将 main 方法视为特殊类 "__main__" 的 "main" 方法
    current_visiting_class = "__main__";
    current_visiting_method = "main";
    name_maps->add_class("__main__");
    name_maps->add_method("__main__", "main");

    // 为 main 方法创建返回类型的伪 Formal（int 类型，名称 "__return__"）
    // 作为 methodFormalList 的最后一个元素
    Pos *retPos = node->getPos()->clone();
    Type *retType = new Type(retPos); // 默认 INT 类型
    IdExp *retId = new IdExp(retPos->clone(), "__return__");
    Formal *retFormal = new Formal(retPos->clone(), retType, retId);
    name_maps->add_method_formal("__main__", "main", "__return__", retFormal);

    // methodFormalList 只包含返回类型（main 没有参数）
    vector<string> formalNames;
    formalNames.push_back("__return__");
    name_maps->add_method_formal_list("__main__", "main", formalNames);

    // 遍历局部变量声明
    if (node->vdl != nullptr)
        for (auto vd : *(node->vdl)) vd->accept(*this);

    current_visiting_class = "";
    current_visiting_method = "";
}

// ClassDecl：类名和继承已在 Program 中处理，
// 这里只处理类变量和方法声明
void AST_Name_Map_Visitor::visit(ClassDecl *node) {
#ifdef DEBUG
    std::cout << "Visiting ClassDecl: " << node->id->id << std::endl;
#endif
    if (node == nullptr) return;

    current_visiting_class = node->id->id;
    current_visiting_method = "";

    // 遍历类变量声明
    if (node->vdl != nullptr)
        for (auto vd : *(node->vdl)) vd->accept(*this);

    // 遍历方法声明
    if (node->mdl != nullptr)
        for (auto md : *(node->mdl)) md->accept(*this);

    current_visiting_class = "";
}

void AST_Name_Map_Visitor::visit(Type *node) {}

// VarDecl：根据上下文将变量注册为类变量或方法局部变量
void AST_Name_Map_Visitor::visit(VarDecl *node) {
#ifdef DEBUG
    std::cout << "Visiting VarDecl: " << node->id->id << std::endl;
#endif
    if (node == nullptr) return;

    string varName = node->id->id;

    if (current_visiting_method.empty()) {
        // 类变量
        if (!name_maps->add_class_var(current_visiting_class, varName, node)) {
            cerr << "Error: at position " << node->getPos()->print() << endl;
            cerr << "Error: Duplicate class variable: " << current_visiting_class << "." << varName << endl;
        }
    } else {
        // 方法局部变量
        if (!name_maps->add_method_var(current_visiting_class, current_visiting_method, varName, node)) {
            cerr << "Error: at position " << node->getPos()->print() << endl;
            cerr << "Error: Duplicate method variable: " << current_visiting_class << "." << current_visiting_method << "." << varName << endl;
        }
    }
}

// MethodDecl：注册方法名、处理形参和返回类型、遍历方法体
void AST_Name_Map_Visitor::visit(MethodDecl *node) {
#ifdef DEBUG
    std::cout << "Visiting MethodDecl: " << node->id->id << std::endl;
#endif
    if (node == nullptr) return;

    string methodName = node->id->id;
    current_visiting_method = methodName;

    // 注册方法（如果已存在则报错）
    if (!name_maps->add_method(current_visiting_class, methodName)) {
        cerr << "Error: at position " << node->getPos()->print() << endl;
        cerr << "Error: Duplicate method: " << current_visiting_class << "." << methodName << endl;
    }

    // 收集形参名列表,遍历形参
    vector<string> formalNames;
    if (node->fl != nullptr) {
        for (auto f : *(node->fl)) {
            formalNames.push_back(f->id->id);
            f->accept(*this);
        }
    }

    // 为返回类型创建伪 Formal（名称 "__return__"），加入形参列表末尾
    Type *retTypeClone = node->type->clone();
    Pos *retPos = node->getPos()->clone();
    IdExp *retId = new IdExp(retPos->clone(), "__return__");
    Formal *retFormal = new Formal(retPos->clone(), retTypeClone, retId);
    name_maps->add_method_formal(current_visiting_class, methodName, "__return__", retFormal);
    formalNames.push_back("__return__");

    // 注册方法的完整形参列表（含返回类型）
    name_maps->add_method_formal_list(current_visiting_class, methodName, formalNames);

    // 遍历方法局部变量声明
    if (node->vdl != nullptr)
        for (auto vd : *(node->vdl)) vd->accept(*this);

    current_visiting_method = "";
}

// Formal：注册方法形参
void AST_Name_Map_Visitor::visit(Formal *node) {
#ifdef DEBUG
    std::cout << "Visiting Formal: " << node->id->id << std::endl;
#endif
    if (node == nullptr) return;

    string formalName = node->id->id;
    if (!name_maps->add_method_formal(current_visiting_class, current_visiting_method, formalName, node)) {
        cerr << "Error: at position " << node->getPos()->print() << endl;
        cerr << "Error: Duplicate formal parameter: " << current_visiting_class << "." << current_visiting_method << "." << formalName << endl;
    }
}

// 以下是语句和表达式的 visit 方法——name map 阶段不需要处理它们
void AST_Name_Map_Visitor::visit(Nested *node) {}
void AST_Name_Map_Visitor::visit(If *node) {}
void AST_Name_Map_Visitor::visit(While *node) {}
void AST_Name_Map_Visitor::visit(Assign *node) {}
void AST_Name_Map_Visitor::visit(CallStm *node) {}
void AST_Name_Map_Visitor::visit(Continue *node) {}
void AST_Name_Map_Visitor::visit(Break *node) {}
void AST_Name_Map_Visitor::visit(Return *node) {}
void AST_Name_Map_Visitor::visit(PutInt *node) {}
void AST_Name_Map_Visitor::visit(PutCh *node) {}
void AST_Name_Map_Visitor::visit(PutArray *node) {}
void AST_Name_Map_Visitor::visit(Starttime *node) {}
void AST_Name_Map_Visitor::visit(Stoptime *node) {}
void AST_Name_Map_Visitor::visit(BinaryOp *node) {}
void AST_Name_Map_Visitor::visit(UnaryOp *node) {}
void AST_Name_Map_Visitor::visit(ArrayExp *node) {}
void AST_Name_Map_Visitor::visit(CallExp *node) {}
void AST_Name_Map_Visitor::visit(ClassVar *node) {}
void AST_Name_Map_Visitor::visit(This *node) {}
void AST_Name_Map_Visitor::visit(Length *node) {}
void AST_Name_Map_Visitor::visit(NewArray *node) {}
void AST_Name_Map_Visitor::visit(NewObject *node) {}
void AST_Name_Map_Visitor::visit(GetInt *node) {}
void AST_Name_Map_Visitor::visit(GetCh *node) {}
void AST_Name_Map_Visitor::visit(GetArray *node) {}
void AST_Name_Map_Visitor::visit(IdExp *node) {}
void AST_Name_Map_Visitor::visit(OpExp *node) {}
void AST_Name_Map_Visitor::visit(IntExp *node) {}
