#define DEBUG
#undef DEBUG

#include "namemaps.hh"
#include "semant.hh"
#include <algorithm>
#include <iostream>
#include <map>
#include <vector>

using namespace std;
using namespace fdmj;

// ============= 辅助函数 =============

// 判断 child_class 是否是 parent_class 的子类（或相同类）
static bool is_subclass(Name_Maps *nm, const string &child_class, const string &parent_class) {
    if (child_class == parent_class) return true;
    string parent = nm->get_parent(child_class);
    if (parent.empty()) return false;
    return parent == parent_class;
    // FDMJ2026 只允许单层继承，所以只需检查一层
}

// 判断两个类型是否兼容（right 能否赋值给 left 类型）
// 规则：INT 对 INT，ARRAY 对 ARRAY，CLASS 对 CLASS（允许子类赋给父类）
static bool type_compatible(Name_Maps *nm, TypeKind lk, variant<monostate, string, int> lp, // lp: left type parameter, 可能是类名string或数组大小int
                            TypeKind rk, variant<monostate, string, int> rp) {
    if (lk != rk) return false;
    if (lk == TypeKind::INT) return true;
    if (lk == TypeKind::ARRAY) return true;
    if (lk == TypeKind::CLASS) {
        string lclass = get<string>(lp); // 从variant里get出类名string
        string rclass = get<string>(rp);
        return is_subclass(nm, rclass, lclass);
    }
    return false;
}

// 从 Type AST 节点提取 type_par
static variant<monostate, string, int> get_type_par_from_type(Type *t) {
    if (t->typeKind == TypeKind::CLASS) return t->cid->id;
    if (t->typeKind == TypeKind::ARRAY) return (t->arity != nullptr) ? t->arity->val : 0;
    return monostate{};
}

// ============= semant_analyze 入口 =============

AST_Semant_Map *semant_analyze(Program *node) {
    if (node == nullptr) return nullptr;
    Name_Maps *name_maps = makeNameMaps(node);
    AST_Semant_Visitor semant_visitor(name_maps);
    node->accept(semant_visitor);
    return semant_visitor.getSemantMap();
}

// ============= 程序结构 Visitor，visit到每个地方的时候都要递归visit子成员 =============

void AST_Semant_Visitor::visit(Program *node) {
    if (node == nullptr) return;
    if (node->main != nullptr) node->main->accept(*this);
    if (node->cdl != nullptr) {
        for (auto cl : *(node->cdl)) cl->accept(*this);
    }
}

void AST_Semant_Visitor::visit(MainMethod *node) {
    if (node == nullptr) return;
    current_visiting_class = "__main__";
    current_visiting_method = "main";

    if (node->vdl != nullptr) {
        for (auto vd : *(node->vdl)) vd->accept(*this);
    }
    if (node->sl != nullptr) {
        for (auto s : *(node->sl)) s->accept(*this);
    }

    current_visiting_class = "";
    current_visiting_method = "";
}

void AST_Semant_Visitor::visit(ClassDecl *node) {
    if (node == nullptr) return;
    string class_name = node->id->id;
    current_visiting_class = class_name;

    // 检查单层继承：父类不能再有父类
    if (node->eid != nullptr) {
        string parent = node->eid->id;
        if (name_maps->is_class(parent)) {
            string grandparent = name_maps->get_parent(parent);
            if (!grandparent.empty()) {
                cerr << "Error: at position " << node->get_pos()->to_str() << endl;
                cerr << "Error: Multi-level inheritance not allowed. "
                     << class_name << " extends " << parent
                     << " which extends " << grandparent << endl;
                exit(1);
            }
            // 检查循环继承
            if (parent == class_name) {
                cerr << "Error: at position " << node->get_pos()->to_str() << endl;
                cerr << "Error: Circular inheritance: " << class_name << endl;
                exit(1);
            }
        }
    }

    if (node->vdl != nullptr) {
        for (auto vd : *(node->vdl)) vd->accept(*this);
    }
    if (node->mdl != nullptr) {
        for (auto md : *(node->mdl)) md->accept(*this);
    }

    current_visiting_class = "";
}

void AST_Semant_Visitor::visit(MethodDecl *node) {
    if (node == nullptr) return;
    string method_name = node->id->id;
    current_visiting_method = method_name;

    // 检查方法重写：如果父类有同名方法，签名必须匹配
    string parent_class = name_maps->get_parent(current_visiting_class);
    if (!parent_class.empty() && name_maps->is_method(parent_class, method_name)) {
        // 获取父类方法的形参列表
        vector<Formal *> *parent_formals = name_maps->get_method_formal_list(parent_class, method_name);
        vector<Formal *> *child_formals = name_maps->get_method_formal_list(current_visiting_class, method_name);

        if (parent_formals != nullptr && child_formals != nullptr) {
            // 参数个数必须相同（包含 __return__）
            if (parent_formals->size() != child_formals->size()) {
                cerr << "Error: at position " << node->get_pos()->to_str() << endl;
                cerr << "Error: Method override parameter count mismatch: "
                     << current_visiting_class << "." << method_name << endl;
                exit(1);
            }
            // 检查每个参数类型（最后一个是返回类型，允许协变）
            for (size_t i = 0; i < parent_formals->size(); i++) {
                Type *pt = (*parent_formals)[i]->type;
                Type *ct = (*child_formals)[i]->type;
                bool is_return = (i == parent_formals->size() - 1);

                if (is_return) {
                    // 返回类型允许协变：子类方法返回类型可以是父类方法返回类型的子类
                    if (!type_compatible(name_maps, pt->typeKind, get_type_par_from_type(pt),
                                         ct->typeKind, get_type_par_from_type(ct))) {
                        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
                        cerr << "Error: Method override return type not covariant: "
                             << current_visiting_class << "." << method_name << endl;
                        exit(1);
                    }
                } else {
                    // 参数类型必须完全匹配
                    if (pt->typeKind != ct->typeKind) {
                        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
                        cerr << "Error: Method override parameter type mismatch: "
                             << current_visiting_class << "." << method_name << endl;
                        exit(1);
                    }
                    if (pt->typeKind == TypeKind::CLASS && pt->cid->id != ct->cid->id) {
                        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
                        cerr << "Error: Method override parameter class type mismatch: "
                             << current_visiting_class << "." << method_name << endl;
                        exit(1);
                    }
                }
            }
        }
    }

    if (node->vdl != nullptr) {
        for (auto vd : *(node->vdl)) vd->accept(*this);
    }
    if (node->sl != nullptr) {
        for (auto s : *(node->sl)) s->accept(*this);
    }

    current_visiting_method = "";
}

void AST_Semant_Visitor::visit(VarDecl *node) {
    if (node == nullptr) return;

    // 检查初始化类型匹配
    if (holds_alternative<IntExp *>(node->init)) { // holds_alternative作用是检查variant当前持有的类型是否是IntExp*
        // int 初始化只能用于 INT 类型
        if (node->type->typeKind != TypeKind::INT) {
            cerr << "Error: at position " << node->get_pos()->to_str() << endl;
            cerr << "Error: Int initializer for non-int variable: " << node->id->id << endl;
            exit(1);
        }
    } else if (holds_alternative<vector<IntExp *> *>(node->init)) {
        // 数组初始化只能用于 ARRAY 类型
        if (node->type->typeKind != TypeKind::ARRAY) {
            cerr << "Error: at position " << node->get_pos()->to_str() << endl;
            cerr << "Error: Array initializer for non-array variable: " << node->id->id << endl;
            exit(1);
        }
    }
    // 检查 CLASS 类型引用的类是否存在
    if (node->type->typeKind == TypeKind::CLASS) {
        if (!name_maps->is_class(node->type->cid->id)) {
            cerr << "Error: at position " << node->get_pos()->to_str() << endl;
            cerr << "Error: Unknown class type: " << node->type->cid->id << endl;
            exit(1);
        }
    }
}

void AST_Semant_Visitor::visit(Formal *node) {
    // 形参在 name map 阶段已处理，此处不需要额外检查
}

void AST_Semant_Visitor::visit(Type *node) {
    // 类型节点不需要语义分析
}

// ============= 语句 Visitor =============

void AST_Semant_Visitor::visit(Nested *node) { // Nested 指的是一个花括号括起来的语句块
    if (node == nullptr) return;
    if (node->sl != nullptr) {
        for (auto s : *(node->sl)) s->accept(*this);
    }
}

void AST_Semant_Visitor::visit(If *node) {
    if (node == nullptr) return;
    // 条件表达式必须存在且为 INT 类型
    if (node->exp != nullptr) {
        node->exp->accept(*this);
        AST_Semant *cond_sem = semant_map->getSemant(node->exp);
        if (cond_sem == nullptr) {
            cerr << "Error: at position " << node->get_pos()->to_str() << endl;
            cerr << "Error: If condition has no semantic information" << endl;
            exit(1);
        }
        if (cond_sem->get_type() != TypeKind::INT) {
            cerr << "Error: at position " << node->get_pos()->to_str() << endl;
            cerr << "Error: If condition must be of integer type" << endl;
            exit(1);
        }
    }
    // then 分支
    if (node->stm1 != nullptr) node->stm1->accept(*this);
    // else 分支（可选）
    if (node->stm2 != nullptr) node->stm2->accept(*this);
}

void AST_Semant_Visitor::visit(While *node) {
    if (node == nullptr) return;
    if (node->exp != nullptr) {
        node->exp->accept(*this);
        AST_Semant *cond_sem = semant_map->getSemant(node->exp);
        if (cond_sem == nullptr) {
            cerr << "Error: at position " << node->get_pos()->to_str() << endl;
            cerr << "Error: While condition has no semantic information" << endl;
            exit(1);
        }
        if (cond_sem->get_type() != TypeKind::INT) {
            cerr << "Error: at position " << node->get_pos()->to_str() << endl;
            cerr << "Error: While condition must be of integer type" << endl;
            exit(1);
        }
    }
    in_a_while_loop++;
    if (node->stm != nullptr) node->stm->accept(*this);
    in_a_while_loop--;
}

void AST_Semant_Visitor::visit(Assign *node) {
    if (node == nullptr) return;
    // 先分析左侧和右侧表达式
    node->left->accept(*this);
    node->right->accept(*this);

    AST_Semant *left_sem = semant_map->getSemant(node->left);
    AST_Semant *right_sem = semant_map->getSemant(node->right);

    // 防御性检查：左右表达式必须都有语义信息
    if (left_sem == nullptr) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: Assign node has no semantic information for its left expression" << endl;
        exit(1);
    }
    if (right_sem == nullptr) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: Assign node has no semantic information for its right expression" << endl;
        exit(1);
    }

    // 检查左侧是否是 lvalue
    if (!left_sem->is_lvalue()) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: Left-hand side of assignment is not an lvalue" << endl;
        exit(1);
    }

    // 检查类型兼容性
    if (!type_compatible(name_maps, left_sem->get_type(), left_sem->get_type_par(),
                         right_sem->get_type(), right_sem->get_type_par())) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: Assign node has a different type between left and right" << endl;
        exit(1);
    }
}

void AST_Semant_Visitor::visit(CallStm *node) { // CallStm 代表方法调用语句，CallExp 代表方法调用表达式
    if (node == nullptr) return;
    // 分析对象表达式
    if (node->obj != nullptr) node->obj->accept(*this);

    AST_Semant *obj_sem = (node->obj != nullptr) ? semant_map->getSemant(node->obj) : nullptr;

    // 对象必须是 CLASS 类型
    if (obj_sem == nullptr || obj_sem->get_type() != TypeKind::CLASS) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: Method call on non-class object" << endl;
        exit(1);
    }

    string obj_class = get<string>(obj_sem->get_type_par());
    string method_name = node->name->id;

    // 为方法名 IdExp 设置 MethodName 语义信息
    semant_map->setSemant(node->name, new AST_Semant(AST_Semant::Kind::MethodName, TypeKind::INT, monostate{}, false));

    // 查找方法（在当前类或父类中）
    string class_to_be_looked_up = obj_class;
    if (!name_maps->is_method(class_to_be_looked_up, method_name)) {
        string parent = name_maps->get_parent(class_to_be_looked_up);
        if (!parent.empty() && name_maps->is_method(parent, method_name)) class_to_be_looked_up = parent;
        else {
            cerr << "Error: at position " << node->get_pos()->to_str() << endl;
            cerr << "Error: Method " << method_name << " not found in class " << obj_class << endl;
            exit(1);
        }
    }

    // 分析参数
    if (node->par != nullptr) {
        for (auto p : *(node->par)) p->accept(*this);
    }

    // 检查参数类型匹配
    vector<Formal *> *formals = name_maps->get_method_formal_list(class_to_be_looked_up, method_name);
    if (formals != nullptr) {
        size_t expected = formals->size() - 1; // 最后一个是 __return__
        size_t actual = (node->par != nullptr) ? node->par->size() : 0;
        if (expected != actual) {
            cerr << "Error: at position " << node->get_pos()->to_str() << endl;
            cerr << "Error: Method " << method_name << " expects " << expected
                 << " arguments but got " << actual << endl;
            exit(1);
        }
        if (node->par != nullptr) { // par: parameters，参数列表
            for (size_t i = 0; i < actual; i++) {
                AST_Semant *arg_sem = semant_map->getSemant((*node->par)[i]);
                Type *formal_type = (*formals)[i]->type;
                if (arg_sem == nullptr) {
                    cerr << "Error: at position " << node->get_pos()->to_str() << endl;
                    cerr << "Error: No semantic information for the " << i << "-th argument in call to " << method_name << endl;
                    exit(1);
                }
                if (!type_compatible(name_maps, formal_type->typeKind, get_type_par_from_type(formal_type),
                                        arg_sem->get_type(), arg_sem->get_type_par())) {
                    cerr << "Error: at position " << node->get_pos()->to_str() << endl;
                    cerr << "Error: Argument type mismatch for parameter " << i
                            << " in call to " << method_name << endl;
                    exit(1);
                }
            }
        }
    }
}

void AST_Semant_Visitor::visit(Continue *node) {
    if (node == nullptr) return;
    if (in_a_while_loop <= 0) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: continue statement outside of a while loop" << endl;
        exit(1);
    }
}

void AST_Semant_Visitor::visit(Break *node) {
    if (node == nullptr) return;
    if (in_a_while_loop <= 0) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: break statement outside of a while loop" << endl;
        exit(1);
    }
}

void AST_Semant_Visitor::visit(Return *node) {
    if (node == nullptr) return;
    if (node->exp != nullptr) node->exp->accept(*this);

    // 检查返回类型是否匹配方法声明
    Formal *ret_formal = name_maps->get_method_formal(current_visiting_class, current_visiting_method, "__return__");
    if (ret_formal != nullptr && node->exp != nullptr) {
        AST_Semant *ret_sem = semant_map->getSemant(node->exp);
        if (ret_sem == nullptr) {
            cerr << "Error: at position " << node->get_pos()->to_str() << endl;
            cerr << "Error: Return node has no semantic information for its expression" << endl;
            exit(1);
        }
        Type *expected = ret_formal->type;
        if (!type_compatible(name_maps, expected->typeKind, get_type_par_from_type(expected),
                             ret_sem->get_type(), ret_sem->get_type_par())) {
            cerr << "Error: at position " << node->get_pos()->to_str() << endl;
            cerr << "Error: Return type mismatch in method "
                 << current_visiting_class << "." << current_visiting_method << endl;
            exit(1);
        }
        // 为 Return 节点本身设置语义信息（与返回表达式同类型，非 lvalue，不含 type_par 细节）
        semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value,
                              ret_sem->get_type(), monostate{}, false));
    }
}

// ============= I/O 语句 Visitor =============

void AST_Semant_Visitor::visit(PutInt *node) {
    if (node == nullptr) return;
    node->exp->accept(*this); // 没用visit是因为visit是虚函数，调用时会根据node的实际类型调用对应的visit方法，而不是固定调用某个visit方法。这个visit方法没写在AST_Semant_Visitor里，而是写在了AST_Visitor里，作为一个纯虚函数（接口），所以只能通过visit来调用，而不能直接调用某个具体的visit方法。
    AST_Semant *sem = semant_map->getSemant(node->exp);
    if (sem == nullptr) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: PutInt node has no semantic information for its expression" << endl;
        exit(1);
    }
    if (sem->get_type() != TypeKind::INT) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: putint() argument must be int" << endl;
        exit(1);
    }
}

void AST_Semant_Visitor::visit(PutCh *node) {
    if (node == nullptr) return;
    node->exp->accept(*this);
    AST_Semant *sem = semant_map->getSemant(node->exp);
    if (sem == nullptr) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: PutCh node has no semantic information for its expression" << endl;
        exit(1);
    }
    if (sem->get_type() != TypeKind::INT) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: putch() argument must be int" << endl;
        exit(1);
    }
}

void AST_Semant_Visitor::visit(PutArray *node) {
    if (node == nullptr) return;
    node->n->accept(*this);
    node->arr->accept(*this);
    AST_Semant *n_sem = semant_map->getSemant(node->n);
    AST_Semant *arr_sem = semant_map->getSemant(node->arr);
    if (n_sem == nullptr) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: PutArray node has no semantic information for its count expression" << endl;
        exit(1);
    }
    if (n_sem->get_type() != TypeKind::INT) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: putarray() first argument must be int" << endl;
        exit(1);
    }
    if (arr_sem == nullptr) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: PutArray node has no semantic information for its array expression" << endl;
        exit(1);
    }
    if (arr_sem->get_type() != TypeKind::ARRAY) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: putarray() second argument must be array" << endl;
        exit(1);
    }
}

void AST_Semant_Visitor::visit(Starttime *node) {
    // 无需语义检查
}

void AST_Semant_Visitor::visit(Stoptime *node) {
    // 无需语义检查
}

// ============= 表达式 Visitor =============

void AST_Semant_Visitor::visit(BinaryOp *node) {
    if (node == nullptr) return;
    node->left->accept(*this);
    node->right->accept(*this);

    AST_Semant *left_sem = semant_map->getSemant(node->left);
    AST_Semant *right_sem = semant_map->getSemant(node->right);

    string op = node->op->op;

    // 所有二元运算符都要求 INT 操作数
    if (left_sem == nullptr) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: BinaryOp node has no semantic information for its left expression" << endl;
        exit(1);
    }
    if (right_sem == nullptr) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: BinaryOp node has no semantic information for its right expression" << endl;
        exit(1);
    }
    if (left_sem->get_type() != TypeKind::INT) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: Left operand of '" << op << "' must be int" << endl;
        exit(1);
    }
    if (right_sem->get_type() != TypeKind::INT) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: Right operand of '" << op << "' must be int" << endl;
        exit(1);
    }

    // 结果类型始终是 INT，不是 lvalue
    semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value, TypeKind::INT, monostate{}, false));
}

void AST_Semant_Visitor::visit(UnaryOp *node) {
    if (node == nullptr) return;
    node->exp->accept(*this);

    AST_Semant *exp_sem = semant_map->getSemant(node->exp);
    string op = node->op->op;

    // 一元运算符 - 和 ! 都要求 INT 操作数
    if (exp_sem == nullptr) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: UnaryOp node has no semantic information for its expression" << endl;
        exit(1);
    }
    if (exp_sem->get_type() != TypeKind::INT) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: Operand of unary '" << op << "' must be int" << endl;
        exit(1);
    }

    // 结果类型是 INT，不是 lvalue
    semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value, TypeKind::INT, monostate{}, false));
}

void AST_Semant_Visitor::visit(ArrayExp *node) {
    if (node == nullptr) return;
    node->arr->accept(*this);
    node->index->accept(*this);

    AST_Semant *arr_sem = semant_map->getSemant(node->arr);
    AST_Semant *idx_sem = semant_map->getSemant(node->index);

    // 基址必须是 ARRAY 类型
    if (arr_sem == nullptr) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: ArrayExp node has no semantic information for its array expression" << endl;
        exit(1);
    }
    if (arr_sem->get_type() != TypeKind::ARRAY) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: ArrayExp node has a non-array value expression" << endl;
        exit(1);
    }
    // 索引必须是 INT 类型
    if (idx_sem == nullptr) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: ArrayExp node has no semantic information for its index expression" << endl;
        exit(1);
    }
    if (idx_sem->get_type() != TypeKind::INT) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: Array index must be int" << endl;
        exit(1);
    }

    // 数组元素是 INT 类型，是 lvalue
    semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value, TypeKind::INT, monostate{}, true));
}

void AST_Semant_Visitor::visit(CallExp *node) { // CallExp 代表方法调用表达式，CallStm 代表方法调用语句。两者的区别在于 CallExp 需要设置语义信息（返回类型），而 CallStm 不需要。eg. foo.bar() 是一个方法调用表达式，既可以作为语句（CallStm），也可以作为更大表达式的一部分（CallExp）。如果是 foo.bar(); 就是一个方法调用语句（CallStm），不需要设置返回类型的语义信息；如果是 int x = foo.bar(); 就是一个方法调用表达式（CallExp），需要设置返回类型的语义信息，以供赋值语句检查类型兼容性。
    if (node == nullptr) return;
    // 分析对象表达式
    if (node->obj != nullptr) node->obj->accept(*this);

    AST_Semant *obj_sem = (node->obj != nullptr) ? semant_map->getSemant(node->obj) : nullptr;

    // 对象必须是 CLASS 类型
    if (obj_sem == nullptr || obj_sem->get_type() != TypeKind::CLASS) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: Method call on non-class object" << endl;
        exit(1);
    }

    string obj_class = get<string>(obj_sem->get_type_par());
    string method_name = node->name->id;

    // 为方法名 IdExp 设置 MethodName 语义信息
    semant_map->setSemant(node->name, new AST_Semant(AST_Semant::Kind::MethodName, TypeKind::INT, monostate{}, false));

    // 查找方法（在当前类或父类中）
    string class_to_be_looked_up = obj_class;
    if (!name_maps->is_method(class_to_be_looked_up, method_name)) {
        string parent = name_maps->get_parent(class_to_be_looked_up);
        if (!parent.empty() && name_maps->is_method(parent, method_name)) class_to_be_looked_up = parent;
        else {
            cerr << "Error: at position " << node->get_pos()->to_str() << endl;
            cerr << "Error: Method " << method_name << " not found in class " << obj_class << endl;
            exit(1);
        }
    }

    // 分析参数
    if (node->par != nullptr) {
        for (auto p : *(node->par)) p->accept(*this);
    }

    // 检查参数类型
    vector<Formal *> *formals = name_maps->get_method_formal_list(class_to_be_looked_up, method_name);
    if (formals != nullptr) {
        size_t expected = formals->size() - 1; // 最后一个是 __return__
        size_t actual = (node->par != nullptr) ? node->par->size() : 0;
        if (expected != actual) {
            cerr << "Error: at position " << node->get_pos()->to_str() << endl;
            cerr << "Error: Method " << method_name << " expects " << expected
                 << " arguments but got " << actual << endl;
            exit(1);
        }
        if (node->par != nullptr) {
            for (size_t i = 0; i < actual; i++) {
                AST_Semant *arg_sem = semant_map->getSemant((*node->par)[i]);
                Type *formal_type = (*formals)[i]->type;
                if (arg_sem != nullptr) {
                    if (!type_compatible(name_maps, formal_type->typeKind, get_type_par_from_type(formal_type),
                                         arg_sem->get_type(), arg_sem->get_type_par())) {
                        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
                        cerr << "Error: Argument type mismatch for parameter " << i
                             << " in call to " << method_name << endl;
                        exit(1);
                    }
                }
            }
        }

        // 设置返回类型
        Formal *ret_formal = formals->back(); // __return__
        Type *ret_type = ret_formal->type;
        semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value,
                              ret_type->typeKind, get_type_par_from_type(ret_type), false));
    }
}

void AST_Semant_Visitor::visit(ClassVar *node) { // ClassVar 代表字段访问表达式，如 foo.bar 中的 bar。需要检查 foo 的类型是否是 CLASS，以及 bar 是否是该类或其父类的字段。同时设置语义信息：字段访问的结果是一个值（Value），类型是字段的类型，并且是 lvalue（因为可以赋值）。另外，为字段名 IdExp 也设置语义信息，表示它是一个类变量（ClassVar）。
    if (node == nullptr) return;
    node->obj->accept(*this);

    AST_Semant *obj_sem = semant_map->getSemant(node->obj);
    if (obj_sem == nullptr || obj_sem->get_type() != TypeKind::CLASS) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: Field access on non-class object" << endl;
        exit(1);
    }

    string obj_class = get<string>(obj_sem->get_type_par());
    string field_name = node->id->id;

    // 在当前类或父类中查找字段
    VarDecl *vd = name_maps->get_class_var(obj_class, field_name);
    if (vd == nullptr) {
        string parent = name_maps->get_parent(obj_class);
        if (!parent.empty()) vd = name_maps->get_class_var(parent, field_name);
    }
    
    // 如果还是找不到，报错
    if (vd == nullptr) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: Field " << field_name << " not found in class " << obj_class << endl;
        exit(1);
    }

    // 设置语义信息：类变量是 lvalue
    semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value,
                          vd->type->typeKind, get_type_par_from_type(vd->type), true));
    // 同时为字段名 IdExp 设置语义信息
    semant_map->setSemant(node->id, new AST_Semant(AST_Semant::Kind::Value,
                          vd->type->typeKind, get_type_par_from_type(vd->type), true));
}

void AST_Semant_Visitor::visit(This *node) { // This 就是 this 关键字，代表当前对象。需要检查 this 是否在类方法中使用，以及设置语义信息：this 的类型是当前类，是一个值（Value），但不是 lvalue（因为不能赋值）。另外，this 只能在类方法中使用，如果在 __main__ 或其他非类方法中使用，需要报错。
    if (node == nullptr) return;
    // this 只能在类方法中使用（不能在 __main__ 中）
    if (current_visiting_class == "__main__" || current_visiting_class.empty()) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: 'this' used outside of a class method" << endl;
        exit(1);
    }
    // this 的类型是当前类，不是 lvalue
    semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value,
                          TypeKind::CLASS, current_visiting_class, false));
}

void AST_Semant_Visitor::visit(Length *node) { // Length 代表数组长度访问表达式，如 foo.length。需要检查 foo 的类型是否是 ARRAY，并设置语义信息：长度访问的结果是一个值（Value），类型是 INT，并且不是 lvalue。
    if (node == nullptr) return;
    node->exp->accept(*this);
    AST_Semant *sem = semant_map->getSemant(node->exp);
    if (sem == nullptr) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: Length node has no semantic information for its expression" << endl;
        exit(1);
    }
    if (sem->get_type() != TypeKind::ARRAY) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: length() argument must be an array" << endl;
        exit(1);
    }
    // length 返回 INT，不是 lvalue
    semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value, TypeKind::INT, monostate{}, false));
}

void AST_Semant_Visitor::visit(NewArray *node) { // new int[exp] 代表数组创建表达式，需要检查 exp 的类型是否是 INT，并设置语义信息：数组创建的结果是一个值（Value），类型是 ARRAY，并且不是 lvalue。
    if (node == nullptr) return;
    node->size->accept(*this);
    AST_Semant *sem = semant_map->getSemant(node->size);
    if (sem == nullptr) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: NewArray node has no semantic information for its size expression" << endl;
        exit(1);
    }
    if (sem->get_type() != TypeKind::INT) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: new int[] size must be int" << endl;
        exit(1);
    }
    // 结果是 ARRAY 类型，arity=0，不是 lvalue
    semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value, TypeKind::ARRAY, 0, false));
}

void AST_Semant_Visitor::visit(NewObject *node) { // new ClassName() 代表对象创建表达式，需要检查 ClassName 是否是一个已声明的类，并设置语义信息：对象创建的结果是一个值（Value），类型是 CLASS，并且不是 lvalue。同时，为 ClassName 的 IdExp 设置语义信息，表示它是一个类类型（ClassType）。
    if (node == nullptr) return;
    string class_name = node->id->id;
    if (!name_maps->is_class(class_name)) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: Unknown class: " << class_name << endl;
        exit(1);
    }
    // 结果是 CLASS 类型，不是 lvalue
    semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value, TypeKind::CLASS, class_name, false));
}

void AST_Semant_Visitor::visit(GetInt *node) {
    if (node == nullptr) return;
    // getint() 返回 INT，不是 lvalue
    semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value, TypeKind::INT, monostate{}, false));
}

void AST_Semant_Visitor::visit(GetCh *node) {
    if (node == nullptr) return;
    // getch() 返回 INT，不是 lvalue
    semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value, TypeKind::INT, monostate{}, false));
}

void AST_Semant_Visitor::visit(GetArray *node) {
    if (node == nullptr) return;
    node->exp->accept(*this);
    AST_Semant *sem = semant_map->getSemant(node->exp);
    if (sem == nullptr) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: GetArray node has no semantic information for its expression" << endl;
        exit(1);
    }
    if (sem->get_type() != TypeKind::ARRAY) {
        cerr << "Error: at position " << node->get_pos()->to_str() << endl;
        cerr << "Error: getarray() argument must be an array" << endl;
        exit(1);
    }
    // getarray() 返回 INT（读取的元素个数），不是 lvalue
    semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value, TypeKind::INT, monostate{}, false));
}

void AST_Semant_Visitor::visit(IdExp *node) { // IdExp 代表标识符表达式，如 foo、bar 等。需要按照名称查找优先顺序（方法局部变量 -> 方法形参 -> 类变量）查找标识符，并设置语义信息：标识符的类型和是否是 lvalue。同时，如果标识符未找到，需要报错。
    if (node == nullptr) return;
    string id = node->id;

    // 名称查找优先顺序：方法局部变量 -> 方法形参 -> 类变量（含父类）
    // 1. 方法局部变量
    VarDecl *vd = name_maps->get_method_var(current_visiting_class, current_visiting_method, id);
    if (vd != nullptr) {
        semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value,
                              vd->type->typeKind, get_type_par_from_type(vd->type), true));
        return;
    }

    // 2. 方法形参
    Formal *f = name_maps->get_method_formal(current_visiting_class, current_visiting_method, id);
    if (f != nullptr) {
        semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value,
                              f->type->typeKind, get_type_par_from_type(f->type), true));
        return;
    }

    // 3. 类变量（当前类及其父类，排除 __main__ 中的情况）
    if (current_visiting_class != "__main__") {
        vd = name_maps->get_class_var(current_visiting_class, id);
        if (vd != nullptr) {
            semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value,
                                  vd->type->typeKind, get_type_par_from_type(vd->type), true));
            return;
        }
        // 检查父类
        string parent = name_maps->get_parent(current_visiting_class);
        if (!parent.empty()) {
            vd = name_maps->get_class_var(parent, id);
            if (vd != nullptr) {
                semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value,
                                      vd->type->typeKind, get_type_par_from_type(vd->type), true));
                return;
            }
        }
    }

    // 4. 标识符未找到 - 报错
    cerr << "Error: at position " << node->get_pos()->to_str() << endl;
    cerr << "Error: Undeclared identifier: " << id << endl;
    exit(1);
}

void AST_Semant_Visitor::visit(IntExp *node) {
    if (node == nullptr) return;
    // 整数字面量是 INT 类型，不是 lvalue
    semant_map->setSemant(node, new AST_Semant(AST_Semant::Kind::Value, TypeKind::INT, monostate{}, false));
}

void AST_Semant_Visitor::visit(OpExp *node) {
    // 运算符标记不需要语义分析
}
