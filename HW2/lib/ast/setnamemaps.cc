/**
 * setnamemaps.cc
 *
 * 实现 AST_Name_Map_Visitor，遍历 AST 并构建 Name_Maps 符号表。
 * 主要工作：
 *   1. 注册所有类名、类层次关系
 *   2. 注册所有方法及其形参列表（含返回类型伪形参 __return__）
 *   3. 注册所有类变量和方法局部变量
 *
 * 关键设计：
 *   - 在 visit(Program*) 里先预注册所有类名，解决父类声明在子类之后的问题
 *   - MainMethod 被视为 __main__ 类中的 main 方法
 *   - 每个方法的 formal list 最后一个元素是返回类型（用 __return__ 标识）
 */

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

bool check_immutability_by_name(string class_name) {
    return class_name.length() >= 10 && class_name.substr(class_name.length() - 10) == "_Immutable";
}


void AST_Name_Map_Visitor::visit(Program *node) {
#ifdef DEBUG
    std::cout << "Visiting Program" << std::endl;
#endif
    if (node == nullptr) return;

    // 预注册所有类名，使得父类可以在子类之后声明
    if (node->cdl != nullptr) {
        for (auto cl : *(node->cdl)) {
            if (cl != nullptr && cl->id != nullptr) {
                if (!name_maps->add_class(cl->id->id)) {
                    cerr << "Error: at position " << cl->get_pos()->to_str() << endl;
                    cerr << "Error: Duplicate class name: " << cl->id->id << endl;
                }
            }
        }
    }

    // 访问 MainMethod
    if (node->main != nullptr) node->main->accept(*this);

    // 访问各个 ClassDecl
    if (node->cdl != nullptr)
        for (auto cl : *(node->cdl)) cl->accept(*this);
}

void AST_Name_Map_Visitor::visit(MainMethod *node) {
#ifdef DEBUG
    std::cout << "Visiting MainMethod" << std::endl;
#endif
    if (node == nullptr) return;

    // MainMethod 被视为 __main__ 类中的 main 方法
    current_visiting_class = "__main__";
    current_visiting_method = "main";
    name_maps->add_class("__main__");
    name_maps->add_method("__main__", "main");

    // 注册局部变量
    if (node->vdl != nullptr)
        for (auto vd : *(node->vdl)) vd->accept(*this);

    // 创建返回类型伪形参（main 返回 int）
    Pos *pos = node->get_pos()->clone();
    Type *retType = new Type(pos); // INT 类型
    IdExp *retId = new IdExp(pos->clone(), "__return__");
    Formal *retFormal = new Formal(pos->clone(), retType, retId);
    name_maps->add_method_formal("__main__", "main", "__return__", retFormal);
    name_maps->add_method_formal_list("__main__", "main", {"__return__"});

    // 无需深入遍历语句（name map 阶段只关心声明）

    current_visiting_class = "";
    current_visiting_method = "";
}

void AST_Name_Map_Visitor::visit(ClassDecl *node) {
#ifdef DEBUG
    std::cout << "Visiting ClassDecl" << std::endl;
#endif
    if (node == nullptr) return;

    string class_name = node->id->id;
    current_visiting_class = class_name;
    // 类名已经在 visit(Program*) 中预注册
    
    if(check_immutability_by_name(class_name)) {
        name_maps->set_class_immutable(class_name, true);
    }

    // 注册继承关系
    if (node->eid != nullptr) {
        string parent_name = node->eid->id;
        if (!name_maps->is_class(parent_name)) {
            cerr << "Error: at position " << node->eid->get_pos()->to_str() << endl;
            cerr << "Error: Parent class " << parent_name << " not found" << endl;
        } else {
            if (!check_immutability_by_name(class_name) && check_immutability_by_name(parent_name)) {
                cerr << "Error: at position " << node->eid->get_pos()->to_str() << endl;
                cerr << "Error: Class " << class_name << " extends immutable class " << parent_name << " but is not marked as immutable. Immutability is hereditary." << endl;
            }
            if (check_immutability_by_name(class_name) && !check_immutability_by_name(parent_name)) {
                cerr << "Error: at position " << node->eid->get_pos()->to_str() << endl;
                cerr << "Error: Class " << class_name << " extends mutable class " << parent_name << " but is marked as immutable. Immutability is hereditary." << endl;
            }
            name_maps->add_class_hiearchy(class_name, parent_name);
        }
    }

    // 注册类变量（此时 current_visiting_method 为空，VarDecl 会被注册为类变量）
    current_visiting_method = "";
    if (node->vdl != nullptr)
        for (auto vd : *(node->vdl)) vd->accept(*this);

    // 注册方法
    if (node->mdl != nullptr)
        for (auto md : *(node->mdl)) md->accept(*this);

    current_visiting_class = "";
}

void AST_Name_Map_Visitor::visit(MethodDecl *node) {
#ifdef DEBUG
    std::cout << "Visiting MethodDecl" << std::endl;
#endif
    if (node == nullptr) return;

    string method_name = node->id->id;
    current_visiting_method = method_name;

    if (!name_maps->add_method(current_visiting_class, method_name)) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: Duplicate method name: " << method_name
             << " in class " << current_visiting_class << endl;
    }

    // 注册形参
    vector<string> formal_names;
    if (node->fl != nullptr) {
        for (auto f : *(node->fl)) {
            f->accept(*this);
            formal_names.push_back(f->id->id);
        }
    }

    // 创建返回类型伪形参
    Type *retType = node->type->clone();
    // 如果是 ARRAY 类型但没有 arity，补上 arity=0
    if (retType->typeKind == TypeKind::ARRAY && retType->arity == nullptr)
        retType->arity = new IntExp(new Pos(0, 0, 0, 0), 0);

    Pos *pos = node->get_pos()->clone();
    IdExp *retId = new IdExp(pos, "__return__");
    Formal *retFormal = new Formal(pos->clone(), retType, retId);
    name_maps->add_method_formal(current_visiting_class, method_name, "__return__", retFormal);
    formal_names.push_back("__return__");
    name_maps->add_method_formal_list(current_visiting_class, method_name, formal_names);

    // 注册方法局部变量
    if (node->vdl != nullptr)
        for (auto vd : *(node->vdl)) vd->accept(*this);

    // 无需深入遍历语句

    current_visiting_method = "";
}

void AST_Name_Map_Visitor::visit(VarDecl *node) {
#ifdef DEBUG
    std::cout << "Visiting VarDecl" << std::endl;
#endif
    if (node == nullptr) return;

    string var_name = node->id->id;

    if (current_visiting_method.empty()) {
        // 类级别变量
        if (!name_maps->add_class_var(current_visiting_class, var_name, node)) {
            cerr << "Error: at position " << node->get_pos()->to_str() << endl;
            cerr << "Error: Duplicate class variable: " << var_name
                 << " in class " << current_visiting_class << endl;
        }
    } else {
        // 方法局部变量
        if (!name_maps->add_method_var(current_visiting_class, current_visiting_method, var_name, node)) {
            cerr << "Error: at position " << node->get_pos()->to_str() << endl;
            cerr << "Error: Duplicate method variable: " << var_name
                 << " in method " << current_visiting_class << "." << current_visiting_method << endl;
        }
    }
}

void AST_Name_Map_Visitor::visit(Formal *node) {
#ifdef DEBUG
    std::cout << "Visiting Formal" << std::endl;
#endif
    if (node == nullptr) return;

    string formal_name = node->id->id;
    if (!name_maps->add_method_formal(current_visiting_class, current_visiting_method, formal_name, node)) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: Duplicate formal parameter: " << formal_name
             << " in method " << current_visiting_class << "." << current_visiting_method << endl;
    }
}

// 以下 visitor 在 name map 阶段不需要做任何处理，仅保留空实现
void AST_Name_Map_Visitor::visit(Type *node) {}
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
