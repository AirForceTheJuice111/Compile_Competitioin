// #define DEBUG
// #undef DEBUG

#include <iostream> // IWYU pragma: keep
#include <string>
#include <map> // IWYU pragma: keep
#include <vector>
#include <algorithm>    // IWYU pragma: keep
#include <variant>      // IWYU pragma: keep
#include "config.hh"    // IWYU pragma: keep
#include "ASTheader.hh" // IWYU pragma: keep
#include "FDMJAST.hh"   // IWYU pragma: keep
#include "treep.hh"     // IWYU pragma: keep
#include "temp.hh"      // IWYU pragma: keep
#include "ast2tree.hh"  // IWYU pragma: keep

using namespace std;

static tree::TempExp* new_temp_exp_of(tree::Temp* temp) {
    return new tree::TempExp(tree::Type::PTR, new tree::Temp(temp->num));
}

static tree::ExpStm* new_exit_minus_1() {
    return new tree::ExpStm(
        new tree::ExtCall(
            tree::Type::INT,
            "exit",
            new vector<tree::Exp *>({new tree::Const(-1)})
        )
    );
}

// Helper: convert fdmj TypeKind to tree::Type
static tree::Type typeKind2TreeType(fdmj::TypeKind tk) {
    if (tk == fdmj::TypeKind::INT) return tree::Type::INT;
    else return tree::Type::PTR; // ARRAY and CLASS are pointers
}

// Generate method var table for a given class and method
// Allocates temps for local vars first, then for formals (including return)
Method_var_table *generate_method_var_table(string class_name, string method_name, Name_Maps *nm, Temp_map *tm) {
    Method_var_table *mvt = new Method_var_table();

    // 1. Allocate temps for method local variables
    set<string> *var_list = nm->get_method_var_list(class_name, method_name);
    if (var_list != nullptr) {
        for (auto &var_name : *var_list) {
            tree::Temp *t = tm->newtemp();
            (*mvt->var_temp_map)[var_name] = t;
            fdmj::VarDecl *vd = nm->get_method_var(class_name, method_name, var_name);
            if (vd != nullptr) (*mvt->var_type_map)[var_name] = typeKind2TreeType(vd->type->typeKind);
            else (*mvt->var_type_map)[var_name] = tree::Type::INT;
        }
        delete var_list;
    }

    // 2. Allocate temps for formals (including _^return^_method_name)
    vector<string> *formal_list = nm->get_method_formal_list_string(class_name, method_name);
    if (formal_list != nullptr) {
        for (auto &formal_name : *formal_list) {
            if (mvt->var_temp_map->find(formal_name) != mvt->var_temp_map->end()) continue; // local overrides formal
            tree::Temp *t = tm->newtemp();
            (*mvt->var_temp_map)[formal_name] = t;
            fdmj::Formal *f = nm->get_method_formal(class_name, method_name, formal_name);
            if (f != nullptr) (*mvt->var_type_map)[formal_name] = typeKind2TreeType(f->type->typeKind);
            else (*mvt->var_type_map)[formal_name] = tree::Type::INT;
        }
        delete formal_list;
    }

    return mvt;
}

// Resolve which class in the hierarchy actually declares a variable
// Walk from class_name up through parents until we find one that owns var_name
static string resolve_var_class(string class_name, string var_name, Name_Maps *nm) {
    string cur = class_name;
    while (!cur.empty()) {
        if (nm->is_class_var(cur, var_name)) return cur;
        cur = nm->get_parent(cur);
    }
    return "";
}

// Resolve which class in the hierarchy implements a method
static string resolve_method_class(string class_name, string method_name, Name_Maps *nm) {
    string cur = class_name;
    while (!cur.empty()) {
        if (nm->is_method(cur, method_name)) return cur;
        cur = nm->get_parent(cur);
    }
    return "";
}

// Generate class table (UOR: Unified Object Record)
// All classes share the same layout: all class vars first (sorted by ClassName^VarName),
// then all unique method names (sorted alphabetically). Each entry occupies address_length bytes.
Class_table *generate_class_table(AST_Semant_Map *semant_map) {
    auto ct = new Class_table();
    auto nm = semant_map->getNameMaps();
    auto class_list = nm->get_class_list();
    if (class_list == nullptr || class_list->empty()) return ct;

    int addr_len = compiler_config.at("address_length");

    // Collect all ClassName^VarName keys (skip __$main__ pseudo-class)
    set<string> all_var_keys;
    set<string> all_method_names;
    for (auto &cls : *class_list) {
        if (cls == "__$main__") continue; // skip main pseudo-class
        auto vars = nm->get_class_var_list(cls);
        if (vars != nullptr) {
            for (auto &v : *vars) all_var_keys.insert(cls + "^" + v);
            delete vars;
        }
        auto methods = nm->get_method_list(cls);
        if (methods != nullptr) {
            for (auto &m : *methods) all_method_names.insert(m);
            delete methods;
        }
    }
    delete class_list;

    // Assign positions: vars first (sorted), then methods (sorted)
    int pos = 0;
    for (auto &key : all_var_keys) {
        ct->var_pos_map[key] = pos;
        pos += addr_len;
    }
    for (auto &m : all_method_names) {
        ct->method_pos_map[m] = pos;
        pos += addr_len;
    }

    ct->print_class_table();
    return ct;
}

// Main entry point: convert AST to tree IR
tree::Program *ast2tree(fdmj::Program *prog, AST_Semant_Map *semant_map) {
    ASTToTreeVisitor visitor;
    visitor.semant_map = semant_map;
    visitor.class_table = generate_class_table(semant_map);
    prog->accept(visitor);
    return static_cast<tree::Program *>(visitor.getTree());
}

// ======================== Visitor Implementations ========================

// Program: visit main method and class decls, collect all FuncDecls
void ASTToTreeVisitor::visit(fdmj::Program *node) {
    auto fdl = new vector<tree::FuncDecl *>();
    // Visit main method
    if (node->main != nullptr) {
        node->main->accept(*this);
        auto fd = static_cast<tree::FuncDecl *>(visit_tree_result);
        if (fd != nullptr) fdl->push_back(fd);
    }
    // Visit class declarations: iterate each class's methods directly
    if (node->cdl != nullptr) {
        for (auto cd : *node->cdl) {
            current_class = cd->id->id;
            if (cd->mdl != nullptr) {
                for (auto md : *cd->mdl) {
                    md->accept(*this);
                    auto fd = static_cast<tree::FuncDecl *>(visit_tree_result);
                    if (fd != nullptr) fdl->push_back(fd);
                }
            }
        }
    }
    visit_tree_result = new tree::Program(fdl);
}

// MainMethod: create a FuncDecl named "__$main__^main"
void ASTToTreeVisitor::visit(fdmj::MainMethod *node) {
    current_class = "__$main__";
    current_method = "main";
    string func_name = current_class + "^" + current_method;

    // Create temp map for this method
    method_temp_map = new Temp_map();
    auto nm = semant_map->getNameMaps();

    // Generate method var table (allocates temps for locals, then formals)
    method_var_table = generate_method_var_table(current_class, current_method, nm, method_temp_map);

    // Determine return type
    tree::Type ret_type = tree::Type::INT;
    string return_var = "_^return^_" + current_method;
    if (method_var_table->var_type_map->find(return_var) != method_var_table->var_type_map->end())
        ret_type = method_var_table->get_var_type(return_var);

    // Initialize continue/break labels
    continue_label = nullptr;
    break_label = nullptr;

    // Process variable declarations (handle initializations)
    vector<tree::Stm *> *sl = new vector<tree::Stm *>();
    if (node->vdl != nullptr) {
        for (auto vd : *node->vdl) {
            vd->accept(*this);
            if (visit_tree_result != nullptr) {
                auto init_stm = static_cast<tree::Stm *>(visit_tree_result);
                // Flatten Seq from VarDecl into the main body
                if (init_stm->getTreeKind() == tree::Kind::SEQ) {
                    auto seq = static_cast<tree::Seq *>(init_stm);
                    if (seq->sl != nullptr) for (auto s : *seq->sl) sl->push_back(s);
                } else {
                    sl->push_back(init_stm);
                }
            }
        }
    }

    // Process statements
    if (node->sl != nullptr) {
        for (auto stm : *node->sl) {
            stm->accept(*this);
            // visit_exp_result holds the Tr_Exp for the statement
            if (visit_exp_result != nullptr) {
                auto nx = visit_exp_result->unNx(method_temp_map);
                if (nx != nullptr && nx->stm != nullptr) sl->push_back(nx->stm);
            }
        }
    }

    tree::Stm *body = new tree::Seq(sl);

    // Build FuncDecl with no args for main
    visit_tree_result = new tree::FuncDecl(
        func_name, nullptr, body, ret_type,
        method_temp_map->next_temp - 1,
        method_temp_map->next_label - 1
    );
}

// ClassDecl: HW4
void ASTToTreeVisitor::visit(fdmj::ClassDecl *node) {
    // HW4: iterate methods, create FuncDecl for each
    visit_tree_result = nullptr;
}

// Type: not directly translated
void ASTToTreeVisitor::visit(fdmj::Type *node) {
    visit_tree_result = nullptr;
    visit_exp_result = nullptr;
}

// VarDecl: handle variable initialization
// - Class type: init to Const(0) (null pointer)
// - Array type without init: init to Const(0) (null pointer)
// - Array literal init (int[] a = {1,2,3}): malloc + store length + store elements
// - Int type without init: no action
void ASTToTreeVisitor::visit(fdmj::VarDecl *node) {
    auto var_name = node->id->id;
    auto temp = method_var_table->get_var_temp(var_name);
    if (temp == nullptr) { visit_tree_result = nullptr; return; }
    auto type = method_var_table->get_var_type(var_name);
    int addr_len = compiler_config.at("address_length");

    // Array literal init: int[] a = {1,2,3}
    // Note: xml2ast sets init to vector<IntExp*>*(nullptr) for arrays without init
    if (holds_alternative<vector<fdmj::IntExp *> *>(node->init)) {
        auto init_arr = get<vector<fdmj::IntExp *> *>(node->init);
        if (init_arr == nullptr) { visit_tree_result = nullptr; return; } // no init
        int len = init_arr->size();
        auto sl = new vector<tree::Stm *>();

        // var = malloc((len+1) * addr_len)
        sl->push_back(new tree::Move(
            new tree::TempExp(tree::Type::PTR, new tree::Temp(temp->num)), // new TempExp is because we want to keep the type information for the temp, which is needed for memory access later. The temp itself is allocated in the method var table and is of type PTR, but we need to wrap it in a TempExp to use it in the IR tree with the correct type.
            new tree::ExtCall(tree::Type::PTR, "malloc",
                new vector<tree::Exp *>({new tree::Const((len + 1) * addr_len)}))));

        // Mem[var] = len
        sl->push_back(new tree::Move(
            new tree::Mem(tree::Type::INT, new tree::TempExp(tree::Type::PTR, new tree::Temp(temp->num))),
            new tree::Const(len)));

        // Mem[var + (i+1)*addr_len] = init_arr[i]
        for (int i = 0; i < len; i++) {
            sl->push_back(new tree::Move(
                new tree::Mem(tree::Type::INT,
                    new tree::Binop(tree::Type::PTR, "+",
                        new tree::TempExp(tree::Type::PTR, new tree::Temp(temp->num)),
                        new tree::Const((i + 1) * addr_len))),
                new tree::Const(init_arr->at(i)->val)));
        }
        visit_tree_result = new tree::Seq(sl);
        return;
    }

    // Class type without init: init to 0 (null pointer)
    if (node->type->typeKind == fdmj::TypeKind::CLASS) {
        visit_tree_result = new tree::Move(
            new tree::TempExp(type, new tree::Temp(temp->num)),
            new tree::Const(0));
        return;
    }

    visit_tree_result = nullptr;
}

// MethodDecl: create a FuncDecl for a class method (ClassName^MethodName)
void ASTToTreeVisitor::visit(fdmj::MethodDecl *node) {
    current_method = node->id->id;
    string func_name = current_class + "^" + current_method;
    string return_var = "_^return^_" + current_method;

    // Create temp map for this method
    method_temp_map = new Temp_map();
    auto nm = semant_map->getNameMaps();

    // Generate method var table (allocates temps for locals, then formals including _^return^_)
    method_var_table = generate_method_var_table(current_class, current_method, nm, method_temp_map);

    // Save return type BEFORE overriding _^return^_ type to PTR
    auto ret_type = typeKind2TreeType(node->type->typeKind);

    // Override _^return^_ type to PTR (it represents the 'this' pointer in class methods)
    (*method_var_table->var_type_map)[return_var] = tree::Type::PTR;

    // Allocate one extra scratch temp (class method convention), which can be used for various purposes (e.g., storing 'this' pointer, intermediate calculations, etc.)
    method_temp_map->newtemp();

    // Initialize continue/break labels
    continue_label = nullptr;
    break_label = nullptr;

    // Process local variable declarations (handle initializations)
    auto sl = new vector<tree::Stm *>();
    if (node->vdl != nullptr) {
        for (auto vd : *node->vdl) {
            vd->accept(*this);
            if (visit_tree_result != nullptr) {
                auto init_stm = static_cast<tree::Stm *>(visit_tree_result);
                // Flatten Seq from VarDecl into method body
                if (init_stm->getTreeKind() == tree::Kind::SEQ) {
                    auto seq = static_cast<tree::Seq *>(init_stm);
                    if (seq->sl != nullptr) for (auto s : *seq->sl) sl->push_back(s);
                } else {
                    sl->push_back(init_stm);
                }
            }
        }
    }

    // Process method body statements
    if (node->sl != nullptr) {
        for (auto stm : *node->sl) {
            stm->accept(*this);
            if (visit_exp_result != nullptr) {
                auto nx = visit_exp_result->unNx(method_temp_map);
                if (nx != nullptr && nx->stm != nullptr) sl->push_back(nx->stm);
            }
        }
    }

    tree::Stm *body = new tree::Seq(sl);

    // Build FuncDecl args: [Temp(this_temp)]
    auto this_temp = method_var_table->get_var_temp(return_var); // this_temp is the temp allocated for _^return^_, which we repurpose as the 'this' pointer in class methods
    auto args = new vector<tree::Temp *>({new tree::Temp(this_temp->num)}); // method convention: the first argument is always the 'this' pointer, which is passed in the temp allocated for _^return^_. the other args (formals) are accessed via their allocated temps in the method var table. we don't need to include them in the FuncDecl args list because they are accessed directly via their temps, not passed as arguments in the IR level.

    visit_tree_result = new tree::FuncDecl(
        func_name, args, body, ret_type,
        method_temp_map->next_temp - 1,
        method_temp_map->next_label - 1);
}

// Formal: not directly translated
void ASTToTreeVisitor::visit(fdmj::Formal *node) {
    visit_tree_result = nullptr;
    visit_exp_result = nullptr;
}

// Nested: translate statement list
void ASTToTreeVisitor::visit(fdmj::Nested *node) {
    auto sl = new vector<tree::Stm *>();
    if (node->sl != nullptr) {
        for (auto stm : *node->sl) {
            stm->accept(*this);
            if (visit_exp_result != nullptr) {
                auto nx = visit_exp_result->unNx(method_temp_map);
                if (nx != nullptr && nx->stm != nullptr) sl->push_back(nx->stm);
            }
        }
    }
    visit_exp_result = new Tr_nx(new tree::Seq(sl));
}

// If: translate condition + then + else
void ASTToTreeVisitor::visit(fdmj::If *node) {
    // Translate condition expression and get as Tr_cx first
    node->exp->accept(*this);
    auto cx = visit_exp_result->unCx(method_temp_map);

    // Translate then-body
    tree::Stm *then_stm = nullptr; // then_stm is the translated then-body statement
    if (node->stm1 != nullptr) {
        node->stm1->accept(*this);
        if (visit_exp_result != nullptr) {
            auto nx = visit_exp_result->unNx(method_temp_map);
            if (nx != nullptr) then_stm = nx->stm; 
        }
    }

    // Translate else-body
    tree::Stm *else_stm = nullptr;
    if (node->stm2 != nullptr) {
        node->stm2->accept(*this);
        if (visit_exp_result != nullptr) {
            auto nx = visit_exp_result->unNx(method_temp_map);
            if (nx != nullptr) else_stm = nx->stm;
        }
    }

    // Get condition as Tr_cx (already done above)
    // Allocate labels for if
    auto true_label = method_temp_map->newlabel();
    auto false_label = method_temp_map->newlabel();
    auto end_label = method_temp_map->newlabel();

    cx->true_list->patch(true_label);
    cx->false_list->patch(false_label);

    auto sl = new vector<tree::Stm *>();
    sl->push_back(cx->stm);
    sl->push_back(new tree::LabelStm(true_label));
    if (then_stm != nullptr) sl->push_back(then_stm);
    sl->push_back(new tree::Jump(end_label));
    sl->push_back(new tree::LabelStm(false_label));
    if (else_stm != nullptr) sl->push_back(else_stm);
    sl->push_back(new tree::LabelStm(end_label));

    visit_exp_result = new Tr_nx(new tree::Seq(sl));
}

// While: translate condition + body with continue/break support
void ASTToTreeVisitor::visit(fdmj::While *node) {
    // Save outer continue/break labels
    auto outer_continue = continue_label;
    auto outer_break = break_label;

    // Translate condition first (before allocating while labels)
    node->exp->accept(*this);
    auto cond = visit_exp_result;

    // Translate body (before allocating while labels to match label ordering)
    tree::Stm *body_stm = nullptr;

    // Get condition as Tr_cx
    auto cx = cond->unCx(method_temp_map);

    // Allocate while labels
    auto start_label = method_temp_map->newlabel();
    auto body_label = method_temp_map->newlabel();
    auto done_label = method_temp_map->newlabel();

    continue_label = start_label;
    break_label = done_label;

    cx->true_list->patch(body_label);
    cx->false_list->patch(done_label);

    if (node->stm != nullptr) {
        node->stm->accept(*this);
        if (visit_exp_result != nullptr) {
            auto nx = visit_exp_result->unNx(method_temp_map);
            if (nx != nullptr) body_stm = nx->stm;
        }
    }

    // Restore outer continue/break labels
    continue_label = outer_continue;
    break_label = outer_break;

    vector<tree::Stm *> *sl = new vector<tree::Stm *>();
    sl->push_back(new tree::LabelStm(start_label));
    sl->push_back(cx->stm);
    sl->push_back(new tree::LabelStm(body_label));
    if (body_stm != nullptr) sl->push_back(body_stm);
    sl->push_back(new tree::Jump(start_label));
    sl->push_back(new tree::LabelStm(done_label));

    visit_exp_result = new Tr_nx(new tree::Seq(sl));
}

// Assign: move src into dst
void ASTToTreeVisitor::visit(fdmj::Assign *node) {
    // Translate left (destination)
    node->left->accept(*this);
    Tr_Exp *left = visit_exp_result;

    // Translate right (source)
    node->exp->accept(*this);
    Tr_Exp *right = visit_exp_result;

    tree::Exp *dst = left->unEx(method_temp_map)->exp;
    tree::Exp *src = right->unEx(method_temp_map)->exp;

    visit_exp_result = new Tr_nx(new tree::Move(dst, src));
}

// CallStm: obj.method(args) as statement (result discarded)
// Call(Memory[obj + method_offset], [obj, args...])
void ASTToTreeVisitor::visit(fdmj::CallStm *node) {
    string method_name = node->name->id;
    int method_pos = class_table->get_method_pos(method_name);

    // Visit obj (the class whose method is being called). obj is the first argument in the method call convention (a.k.a this), and also needed to compute the function pointer for the call
    node->obj->accept(*this);
    auto obj_exp = visit_exp_result->unEx(method_temp_map)->exp;

    // Build args: [obj, explicit_params...]
    auto call_args = new vector<tree::Exp *>({obj_exp});
    if (node->par != nullptr) {
        for (auto p : *node->par) {
            p->accept(*this);
            call_args->push_back(visit_exp_result->unEx(method_temp_map)->exp);
        }
    }

    // Function pointer: Memory[obj + method_offset]
    auto func_ptr = new tree::Mem(tree::Type::PTR,
        new tree::Binop(tree::Type::PTR, "+", obj_exp, new tree::Const(method_pos)));

    // Determine return type from semant map
    auto sem = semant_map->getSemant(node);
    tree::Type ret_type = tree::Type::INT;
    if (sem != nullptr) ret_type = typeKind2TreeType(sem->get_type());

    visit_exp_result = new Tr_nx(new tree::ExpStm(
        new tree::Call(ret_type, method_name, func_ptr, call_args)));
}

// Continue: jump to continue label
void ASTToTreeVisitor::visit(fdmj::Continue *node) {
    if (continue_label != nullptr)
        visit_exp_result = new Tr_nx(new tree::Jump(continue_label));
    else
        visit_exp_result = new Tr_nx(new tree::Seq());
}

// Break: jump to break label
void ASTToTreeVisitor::visit(fdmj::Break *node) {
    if (break_label != nullptr)
        visit_exp_result = new Tr_nx(new tree::Jump(break_label));
    else
        visit_exp_result = new Tr_nx(new tree::Seq());
}

// Return: return expression
void ASTToTreeVisitor::visit(fdmj::Return *node) {
    if (node->exp != nullptr) {
        node->exp->accept(*this);
        auto exp = visit_exp_result;
        auto ret_exp = exp->unEx(method_temp_map)->exp; // Tr_Exp base class -> Tr_ex wrapper -> tree::Exp node
        visit_exp_result = new Tr_nx(new tree::Return(ret_exp)); // return <SOMETHING>;
    } else {
        visit_exp_result = new Tr_nx(new tree::Return(new tree::Const(0))); // return;
    }
}

// PutInt: putint(exp) -> ExtCall("putint", {exp})
void ASTToTreeVisitor::visit(fdmj::PutInt *node) {
    node->exp->accept(*this);
    auto arg = visit_exp_result->unEx(method_temp_map)->exp;
    visit_exp_result = new Tr_nx(new tree::ExpStm(
        new tree::ExtCall(tree::Type::INT, "putint", new vector<tree::Exp *>({arg}))));
}

// PutCh: putch(exp) -> ExtCall("putch", {exp})
void ASTToTreeVisitor::visit(fdmj::PutCh *node) {
    node->exp->accept(*this);
    auto arg = visit_exp_result->unEx(method_temp_map)->exp;
    visit_exp_result = new Tr_nx(new tree::ExpStm(
        new tree::ExtCall(tree::Type::INT, "putch", new vector<tree::Exp *>({arg}))));
}

// PutArray: putarray(n, arr) -> ExtCall("putarray", {n, arr})
void ASTToTreeVisitor::visit(fdmj::PutArray *node) {
    node->n->accept(*this);
    auto n_exp = visit_exp_result->unEx(method_temp_map)->exp;
    node->arr->accept(*this);
    auto arr_exp = visit_exp_result->unEx(method_temp_map)->exp;
    visit_exp_result = new Tr_nx(new tree::ExpStm(
        new tree::ExtCall(tree::Type::INT, "putarray", new vector<tree::Exp *>({n_exp, arr_exp}))));
}

// Starttime: ExtCall("starttime", {})
void ASTToTreeVisitor::visit(fdmj::Starttime *node) {
    visit_exp_result = new Tr_nx(new tree::ExpStm(
        new tree::ExtCall(tree::Type::INT, "starttime", new vector<tree::Exp *>())));
}

// Stoptime: ExtCall("stoptime", {})
void ASTToTreeVisitor::visit(fdmj::Stoptime *node) {
    visit_exp_result = new Tr_nx(new tree::ExpStm(
        new tree::ExtCall(tree::Type::INT, "stoptime", new vector<tree::Exp *>())));
}

// BinaryOp: translate binary operations
void ASTToTreeVisitor::visit(fdmj::BinaryOp *node) {
    string op = node->op->op; // BinaryOp -> OpExp -> string

    // Short-circuit logical operators
    if (op == "&&") {
        // left && right: left true -> check right; left false -> overall false
        node->left->accept(*this);
        auto left = visit_exp_result;
        auto left_cx = left->unCx(method_temp_map);

        node->right->accept(*this);
        auto right = visit_exp_result;
        auto right_cx = right->unCx(method_temp_map);

        // left true -> evaluate right
        auto middle = method_temp_map->newlabel();
        left_cx->true_list->patch(middle);

        // Combined: true_list = right's true, false_list = left's false + right's false
        left_cx->false_list->add(right_cx->false_list);
        visit_exp_result = new Tr_cx(right_cx->true_list, left_cx->false_list,
            new tree::Seq(new vector<tree::Stm *>({left_cx->stm, new tree::LabelStm(middle), right_cx->stm})));
        return;
    }

    if (op == "||") {
        // left || right: left true -> overall true; left false -> check right
        node->left->accept(*this);
        auto left = visit_exp_result;
        auto left_cx = left->unCx(method_temp_map);

        node->right->accept(*this);
        auto right = visit_exp_result;
        auto right_cx = right->unCx(method_temp_map);

        // left false -> evaluate right
        auto middle = method_temp_map->newlabel();
        left_cx->false_list->patch(middle);

        // Combined: true_list = left's true + right's true, false_list = right's false
        left_cx->true_list->add(right_cx->true_list);
        visit_exp_result = new Tr_cx(left_cx->true_list, right_cx->false_list,
            new tree::Seq(new vector<tree::Stm *>({left_cx->stm, new tree::LabelStm(middle), right_cx->stm})));
        return;
    }

    // Comparison operators: produce Tr_cx
    if (op == "<" || op == ">" || op == "<=" || op == ">=" || op == "==" || op == "!=") {
        node->left->accept(*this);
        auto left = visit_exp_result;
        tree::Exp *left_exp = left->unEx(method_temp_map)->exp;

        node->right->accept(*this);
        auto right = visit_exp_result;
        tree::Exp *right_exp = right->unEx(method_temp_map)->exp;

        auto tl = method_temp_map->newlabel();
        auto fl = method_temp_map->newlabel();
        auto true_list = new Patch_list();
        true_list->add_patch(tl);
        auto false_list = new Patch_list();
        false_list->add_patch(fl);

        auto cjump = new tree::Cjump(op, left_exp, right_exp, tl, fl); // this is the only place where a Cjump is new-ed, and the labels in this Cjump will be patched later using the patch lists when we generate code for the then/else branches of the if statement (for example)
        visit_exp_result = new Tr_cx(true_list, false_list, cjump);
        return;
    }

    // Arithmetic operators: +, -, *, /
    node->left->accept(*this);
    auto left = visit_exp_result;
    auto left_exp = left->unEx(method_temp_map)->exp;

    node->right->accept(*this);
    auto right = visit_exp_result;
    auto right_exp = right->unEx(method_temp_map)->exp;

    auto binop = new tree::Binop(tree::Type::INT, op, left_exp, right_exp);
    visit_exp_result = new Tr_ex(binop);
}

// UnaryOp: translate unary operations
void ASTToTreeVisitor::visit(fdmj::UnaryOp *node) {
    string op = node->op->op;
    node->exp->accept(*this);
    auto operand = visit_exp_result;

    if (op == "-") {
        // -exp -> 0 - exp
        auto exp = operand->unEx(method_temp_map)->exp;
        visit_exp_result = new Tr_ex(new tree::Binop(tree::Type::INT, "-", new tree::Const(0), exp));
    } else if (op == "!") {
        // !exp -> 1 xor exp (or flip the cx lists)
        auto cx = operand->unCx(method_temp_map);
        // Flip true and false lists
        visit_exp_result = new Tr_cx(cx->false_list, cx->true_list, cx->stm);
    } else {
        visit_exp_result = operand;
    }
}

// ArrayExp: arr[index] -> bounds-checked Memory access
// If arr or index is complex (not Temp/Const), materialize to a temp first.
// Bounds check: CJump(index >= 0, ok, exit), CJump(index >= len, exit, done), exit(-1)
// Result: Memory[arr + (index + 1) * addr_len], because arr[0] stores the length, so actual data starts from arr + addr_len
void ASTToTreeVisitor::visit(fdmj::ArrayExp *node) {
    int addr_len = compiler_config.at("address_length");

    // Visit array expression
    node->arr->accept(*this);
    auto arr_raw = visit_exp_result->unEx(method_temp_map)->exp;

    // Visit index expression
    node->index->accept(*this);
    auto idx_raw = visit_exp_result->unEx(method_temp_map)->exp;

    // Materialize arr if complex
    tree::Exp *arr_exp = arr_raw;
    tree::Stm *arr_pre_stm = nullptr;
    if (arr_raw->getTreeKind() != tree::Kind::TEMPEXP && arr_raw->getTreeKind() != tree::Kind::CONST) {
        auto arr_temp = method_temp_map->newtemp();
        arr_pre_stm = new tree::Seq(new vector<tree::Stm *>({
            new tree::Move(
                new tree::TempExp(tree::Type::PTR, new tree::Temp(arr_temp->num)),
                arr_raw)}));
        arr_exp = new tree::TempExp(tree::Type::PTR, new tree::Temp(arr_temp->num));
    }

    // Materialize index if complex
    tree::Exp *idx_exp = idx_raw;
    tree::Stm *idx_pre_stm = nullptr;
    if (idx_raw->getTreeKind() != tree::Kind::TEMPEXP && idx_raw->getTreeKind() != tree::Kind::CONST) {
        auto idx_temp = method_temp_map->newtemp();
        idx_pre_stm = new tree::Seq(new vector<tree::Stm *>({
            new tree::Move(
                new tree::TempExp(tree::Type::INT, new tree::Temp(idx_temp->num)),
                idx_raw)}));
        idx_exp = new tree::TempExp(tree::Type::INT, new tree::Temp(idx_temp->num));
    }

    // Bounds check
    auto len_temp = method_temp_map->newtemp();
    auto exit_label = method_temp_map->newlabel();
    auto ok_label = method_temp_map->newlabel();
    auto done_label = method_temp_map->newlabel();

    auto bounds_sl = new vector<tree::Stm *>();
    // len = Mem[arr]
    bounds_sl->push_back(new tree::Move(
        new tree::TempExp(tree::Type::INT, new tree::Temp(len_temp->num)),
        new tree::Mem(tree::Type::INT, arr_exp)));
    // CJump(index >= 0, ok, exit)
    bounds_sl->push_back(new tree::Cjump(">=", idx_exp, new tree::Const(0), ok_label, exit_label));
    bounds_sl->push_back(new tree::LabelStm(ok_label));
    // CJump(index >= len, exit, done)
    bounds_sl->push_back(new tree::Cjump(">=", idx_exp,
        new tree::TempExp(tree::Type::INT, new tree::Temp(len_temp->num)), exit_label, done_label));
    bounds_sl->push_back(new tree::LabelStm(exit_label));
    // exit(-1)
    bounds_sl->push_back(new tree::ExpStm(
        new tree::ExtCall(tree::Type::INT, "exit", new vector<tree::Exp *>({new tree::Const(-1)}))));
    bounds_sl->push_back(new tree::LabelStm(done_label));

    // ESeq: bounds check, returns index
    auto bounds_eseq = new tree::Eseq(tree::Type::INT, new tree::Seq(bounds_sl), idx_exp);

    // Memory[arr + (bounds_checked_index + 1) * addr_len]
    auto mem = new tree::Mem(tree::Type::INT,
        new tree::Binop(tree::Type::PTR, "+", arr_exp,
            new tree::Binop(tree::Type::INT, "*",
                new tree::Binop(tree::Type::INT, "+", bounds_eseq, new tree::Const(1)),
                new tree::Const(addr_len))));

    // Wrap with materialization pre-statements if needed
    tree::Exp *result = mem;
    if (idx_pre_stm != nullptr)
        result = new tree::Eseq(tree::Type::INT, idx_pre_stm, result); // idx_pre_stm; return result;
    if (arr_pre_stm != nullptr)
        result = new tree::Eseq(tree::Type::INT, arr_pre_stm, result); // arr_pre_stm; return result;

    visit_exp_result = new Tr_ex(result);
}

// CallExp: obj.method(args) as expression (result used)
// Call(Memory[obj + method_offset], [obj, args...])
void ASTToTreeVisitor::visit(fdmj::CallExp *node) {
    string method_name = node->name->id;
    int method_pos = class_table->get_method_pos(method_name);

    // Visit obj
    node->obj->accept(*this);
    auto obj_exp = visit_exp_result->unEx(method_temp_map)->exp;

    // Build args: [obj, explicit_params...]
    auto call_args = new vector<tree::Exp *>({obj_exp});
    if (node->par != nullptr) {
        for (auto p : *node->par) {
            p->accept(*this);
            call_args->push_back(visit_exp_result->unEx(method_temp_map)->exp);
        }
    }

    // Function pointer: Memory[obj + method_offset]
    auto func_ptr = new tree::Mem(tree::Type::PTR,
        new tree::Binop(tree::Type::PTR, "+", obj_exp, new tree::Const(method_pos)));

    // Determine return type from semant map
    auto sem = semant_map->getSemant(node);
    tree::Type ret_type = tree::Type::INT;
    if (sem != nullptr) ret_type = typeKind2TreeType(sem->get_type());

    visit_exp_result = new Tr_ex(
        new tree::Call(ret_type, method_name, func_ptr, call_args));
}

// ClassVar: obj.field -> Memory[obj + class_table.get_var_pos(declaring_class, field)]
// Sets class_var_class_name for chained access (e.g. this.c.j)
void ASTToTreeVisitor::visit(fdmj::ClassVar *node) {
    auto nm = semant_map->getNameMaps();

    // Visit obj to get its translation and determine its class type
    node->obj->accept(*this);
    auto obj_exp = visit_exp_result->unEx(method_temp_map)->exp;

    // Determine the class type of the obj expression
    string obj_class;
    if (node->obj->getASTKind() == fdmj::ASTKind::This) {
        obj_class = current_class;
    } else {
        // Use class_var_class_name if set by a previous ClassVar visit (chained access)
        // or look up from semant_map
        auto sem = semant_map->getSemant(node->obj);
        if (sem != nullptr && sem->get_type() == fdmj::TypeKind::CLASS)
            obj_class = get<string>(sem->get_type_par());
        else // didn't get class type info from semant map because of chained access, eg. this.c.j, where semant map only has type info for 'this' but not for 'this.c', so we rely on class_var_class_name to carry the class type info across chained ClassVar visits
            obj_class = class_var_class_name;
    }

    string field_name = node->id->id;

    // Walk up class hierarchy to find which class declares this field
    string declaring_class = resolve_var_class(obj_class, field_name, nm);
    int offset = class_table->get_var_pos(declaring_class, field_name);

    // Determine field type from class var declaration
    auto field_vd = nm->get_class_var(declaring_class, field_name);
    tree::Type field_type = (field_vd != nullptr) ? typeKind2TreeType(field_vd->type->typeKind) : tree::Type::INT;

    // Set class_var_class_name for chained access
    if (field_vd != nullptr && field_vd->type->typeKind == fdmj::TypeKind::CLASS)
        class_var_class_name = field_vd->type->cid->id;
    else
        class_var_class_name = "";

    // Memory[obj + offset]
    auto mem = new tree::Mem(field_type,
        new tree::Binop(tree::Type::PTR, "+", obj_exp, new tree::Const(offset)));
    visit_exp_result = new Tr_ex(mem);
}

// This: return TempExp of the 'this' pointer (_^return^_method's temp)
void ASTToTreeVisitor::visit(fdmj::This *node) {
    string return_var = "_^return^_" + current_method;
    auto temp = method_var_table->get_var_temp(return_var);
    class_var_class_name = current_class; // for ClassVar chained access
    visit_exp_result = new Tr_ex(new tree::TempExp(tree::Type::PTR, new tree::Temp(temp->num)));
}

// Length: return Memory[arr], which is the length of the array (stored at arr[0])
void ASTToTreeVisitor::visit(fdmj::Length *node) {
    node->exp->accept(*this);
    auto arr_exp = visit_exp_result->unEx(method_temp_map)->exp;
    visit_exp_result = new Tr_ex(new tree::Mem(tree::Type::INT, arr_exp));
}

// NewArray: new int[size] -> ESeq(Seq(malloc, store_length), ptr_temp)
void ASTToTreeVisitor::visit(fdmj::NewArray *node) {
    int addr_len = compiler_config.at("address_length");
    node->size->accept(*this);
    auto size_exp = visit_exp_result->unEx(method_temp_map)->exp;

    auto ptr_temp = method_temp_map->newtemp();
    auto sl = new vector<tree::Stm *>();

    // ptr = malloc((size + 1) * addr_len)
    sl->push_back(new tree::Move(
        new tree::TempExp(tree::Type::PTR, new tree::Temp(ptr_temp->num)),
        new tree::ExtCall(tree::Type::PTR, "malloc",
            new vector<tree::Exp *>({
                new tree::Binop(tree::Type::INT, "*",
                    new tree::Binop(tree::Type::INT, "+", size_exp, new tree::Const(1)),
                    new tree::Const(addr_len))}))));

    // Mem[ptr] = size
    sl->push_back(new tree::Move(
        new tree::Mem(tree::Type::INT, new tree::TempExp(tree::Type::PTR, new tree::Temp(ptr_temp->num))),
        size_exp));

    visit_exp_result = new Tr_ex(new tree::Eseq(tree::Type::PTR,
        new tree::Seq(sl),
        new tree::TempExp(tree::Type::PTR, new tree::Temp(ptr_temp->num))));
}

// NewObject: new ClassName() -> ESeq(Seq(malloc, store method ptrs), ptr_temp)
void ASTToTreeVisitor::visit(fdmj::NewObject *node) {
    auto nm = semant_map->getNameMaps();
    string class_name = node->id->id;

    // Calculate object size: total entries in UOR * addr_len (UOR is the layout of the object in memory, which includes all fields and method pointers, and is determined by the class hierarchy and the class_table we built during semantic analysis. the size of the UOR determines how much memory we need to allocate for each object of this class, UOR's full name is "Unified Object Representation").
    int total_size = (class_table->var_pos_map.size() + class_table->method_pos_map.size())
                     * compiler_config.at("address_length");

    auto ptr_temp = method_temp_map->newtemp();
    auto sl = new vector<tree::Stm *>();

    // ptr = malloc(total_size)
    sl->push_back(new tree::Move(
        new tree::TempExp(tree::Type::PTR, new tree::Temp(ptr_temp->num)),
        new tree::ExtCall(tree::Type::PTR, "malloc",
            new vector<tree::Exp *>({new tree::Const(total_size)}))));

    // Store method function pointers for this class, which are needed for dynamic dispatch. We get the method positions from the class_table, and resolve which class implements each method for this class using the resolve_method_class function (which walks up the class hierarchy to find the class that implements the method). Then we store a string literal "ImplClass^method" at the corresponding method offset in the object layout, which will be used by the runtime to identify which method to call during dynamic dispatch.
    // For each method in the UOR, resolve which class implements it for class_name
    for (auto &[method_name, method_pos] : class_table->method_pos_map) {
        auto impl_class = resolve_method_class(class_name, method_name, nm);
        if (impl_class.empty()) continue; // this class doesn't have this method
        // Store Name("ImplClass^method") at Mem[ptr + method_pos]
        auto sname = method_temp_map->newstringlabel(impl_class + "^" + method_name);
        sl->push_back(new tree::Move(
            new tree::Mem(tree::Type::PTR,
                new tree::Binop(tree::Type::PTR, "+",
                    new tree::TempExp(tree::Type::PTR, new tree::Temp(ptr_temp->num)),
                    new tree::Const(method_pos))),
            new tree::Name(sname)));
    }

    visit_exp_result = new Tr_ex(new tree::Eseq(tree::Type::PTR,
        new tree::Seq(sl),
        new tree::TempExp(tree::Type::PTR, new tree::Temp(ptr_temp->num))));
}

// GetInt: getint() -> ExtCall("getint", {})
void ASTToTreeVisitor::visit(fdmj::GetInt *node) {
    visit_exp_result = new Tr_ex(
        new tree::ExtCall(tree::Type::INT, "getint", new vector<tree::Exp *>()));
}

// GetCh: getch() -> ExtCall("getch", {})
void ASTToTreeVisitor::visit(fdmj::GetCh *node) {
    visit_exp_result = new Tr_ex(
        new tree::ExtCall(tree::Type::INT, "getch", new vector<tree::Exp *>()));
}

// GetArray: getarray(exp) -> ExtCall("getarray", {exp})
void ASTToTreeVisitor::visit(fdmj::GetArray *node) {
    node->exp->accept(*this);
    auto arg = visit_exp_result->unEx(method_temp_map)->exp;
    visit_exp_result = new Tr_ex(
        new tree::ExtCall(tree::Type::PTR, "getarray", new vector<tree::Exp *>({arg})));
}

// IdExp: look up variable in method var table, convert to TempExp
void ASTToTreeVisitor::visit(fdmj::IdExp *node) {
    string name = node->id;
    auto temp = method_var_table->get_var_temp(name);
    if (temp != nullptr) {
        auto t = method_var_table->get_var_type(name);
        visit_exp_result = new Tr_ex(new tree::TempExp(t, new tree::Temp(temp->num))); // same Temp register number, but wrapped in a TempExp with type info
    } else {
        // Variable not found - should not happen in correct programs
        visit_exp_result = new Tr_ex(new tree::Const(0));
    }
}

// OpExp: not directly visited (handled in BinaryOp/UnaryOp)
void ASTToTreeVisitor::visit(fdmj::OpExp *node) {
    visit_exp_result = nullptr;
}

// IntExp: constant integer
void ASTToTreeVisitor::visit(fdmj::IntExp *node) {
    visit_exp_result = new Tr_ex(new tree::Const(node->val));
}
