//#define DEBUG
//#undef DEBUG

#include <iostream> // IWYU pragma: keep
#include <string>
#include <map> // IWYU pragma: keep
#include <vector>
#include <algorithm> // IWYU pragma: keep
#include <variant> // IWYU pragma: keep
#include "config.hh" // IWYU pragma: keep
#include "ASTheader.hh" // IWYU pragma: keep
#include "FDMJAST.hh" // IWYU pragma: keep
#include "treep.hh" // IWYU pragma: keep
#include "temp.hh" // IWYU pragma: keep
#include "ast2tree.hh" // IWYU pragma: keep

using namespace std;

// Helper: convert fdmj TypeKind to tree::Type
static tree::Type typeKind2TreeType(fdmj::TypeKind tk) {
    if (tk == fdmj::TypeKind::INT) return tree::Type::INT;
    return tree::Type::PTR; // ARRAY and CLASS are pointers
}

// Generate method var table for a given class and method
// Allocates temps for local vars first, then for formals (including return)
Method_var_table* generate_method_var_table(string class_name, string method_name, Name_Maps* nm, Temp_map* tm) {
    Method_var_table* mvt = new Method_var_table();

    // 1. Allocate temps for method local variables
    set<string>* var_list = nm->get_method_var_list(class_name, method_name);
    if (var_list != nullptr) {
        for (auto &var_name : *var_list) {
            tree::Temp* t = tm->newtemp();
            (*mvt->var_temp_map)[var_name] = t;
            fdmj::VarDecl* vd = nm->get_method_var(class_name, method_name, var_name);
            if (vd != nullptr) (*mvt->var_type_map)[var_name] = typeKind2TreeType(vd->type->typeKind);
            else (*mvt->var_type_map)[var_name] = tree::Type::INT;
        }
        delete var_list;
    }

    // 2. Allocate temps for formals (including _^return^_method_name)
    vector<string>* formal_list = nm->get_method_formal_list_string(class_name, method_name);
    if (formal_list != nullptr) {
        for (auto &formal_name : *formal_list) {
            if (mvt->var_temp_map->find(formal_name) != mvt->var_temp_map->end()) continue; // local overrides formal
            tree::Temp* t = tm->newtemp();
            (*mvt->var_temp_map)[formal_name] = t;
            fdmj::Formal* f = nm->get_method_formal(class_name, method_name, formal_name);
            if (f != nullptr) (*mvt->var_type_map)[formal_name] = typeKind2TreeType(f->type->typeKind);
            else (*mvt->var_type_map)[formal_name] = tree::Type::INT;
        }
        delete formal_list;
    }

    return mvt;
}

// Generate class table (empty for HW3 since no classes)
Class_table* generate_class_table(AST_Semant_Map* semant_map) {
    return new Class_table();
}

// Main entry point: convert AST to tree IR
tree::Program* ast2tree(fdmj::Program* prog, AST_Semant_Map* semant_map) {
    ASTToTreeVisitor visitor;
    visitor.semant_map = semant_map;
    visitor.class_table = generate_class_table(semant_map);
    prog->accept(visitor);
    return static_cast<tree::Program*>(visitor.getTree());
}

// ======================== Visitor Implementations ========================

// Program: visit main method (and class decls in HW4)
void ASTToTreeVisitor::visit(fdmj::Program* node) {
    vector<tree::FuncDecl*> *fdl = new vector<tree::FuncDecl*>();
    // Visit main method
    if (node->main != nullptr) {
        node->main->accept(*this);
        tree::FuncDecl* fd = static_cast<tree::FuncDecl*>(visit_tree_result);
        if (fd != nullptr) fdl->push_back(fd);
    }
    // Visit class declarations (HW4)
    if (node->cdl != nullptr) {
        for (auto cd : *node->cdl) {
            cd->accept(*this);
            // In HW4, this would produce FuncDecls for each method
        }
    }
    visit_tree_result = new tree::Program(fdl);
}

// MainMethod: create a FuncDecl named "__$main__^main"
void ASTToTreeVisitor::visit(fdmj::MainMethod* node) {
    current_class = "__$main__";
    current_method = "main";
    string func_name = current_class + "^" + current_method;

    // Create temp map for this method
    method_temp_map = new Temp_map();
    Name_Maps* nm = semant_map->getNameMaps();

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
    vector<tree::Stm*> *sl = new vector<tree::Stm*>();
    if (node->vdl != nullptr) {
        for (auto vd : *node->vdl) {
            vd->accept(*this);
            if (visit_tree_result != nullptr) {
                tree::Stm* init_stm = static_cast<tree::Stm*>(visit_tree_result);
                sl->push_back(init_stm);
            }
        }
    }

    // Process statements
    if (node->sl != nullptr) {
        for (auto stm : *node->sl) {
            stm->accept(*this);
            // visit_exp_result holds the Tr_Exp for the statement
            if (visit_exp_result != nullptr) {
                Tr_nx* nx = visit_exp_result->unNx(method_temp_map);
                if (nx != nullptr && nx->stm != nullptr) sl->push_back(nx->stm);
            }
        }
    }

    tree::Stm* body = new tree::Seq(sl);

    // Build FuncDecl with no args for main
    tree::FuncDecl* fd = new tree::FuncDecl(
        func_name, nullptr, body, ret_type,
        method_temp_map->next_temp - 1,
        method_temp_map->next_label - 1
    );
    visit_tree_result = fd;
}

// ClassDecl: HW4
void ASTToTreeVisitor::visit(fdmj::ClassDecl* node) {
    // HW4: iterate methods, create FuncDecl for each
    visit_tree_result = nullptr;
}

// Type: not directly translated
void ASTToTreeVisitor::visit(fdmj::Type* node) {
    visit_tree_result = nullptr;
    visit_exp_result = nullptr;
}

// VarDecl: handle variable initialization
void ASTToTreeVisitor::visit(fdmj::VarDecl* node) {
    string var_name = node->id->id;
    tree::Temp* temp = method_var_table->get_var_temp(var_name);
    if (temp == nullptr) {
        visit_tree_result = nullptr;
        return;
    }
    tree::Type var_type = method_var_table->get_var_type(var_name);

    // Handle initialization
    if (holds_alternative<fdmj::IntExp*>(node->init)) {
        // int x = val;
        fdmj::IntExp* init_val = get<fdmj::IntExp*>(node->init);
        if (init_val != nullptr) {
            visit_tree_result = new tree::Move(
                new tree::TempExp(var_type, new tree::Temp(temp->num)),
                new tree::Const(init_val->val)
            );
            return;
        }
    }
    // No initialization or array init (HW4 for arrays)
    visit_tree_result = nullptr;
}

// MethodDecl: HW4
void ASTToTreeVisitor::visit(fdmj::MethodDecl* node) {
    visit_tree_result = nullptr;
    visit_exp_result = nullptr;
}

// Formal: not directly translated
void ASTToTreeVisitor::visit(fdmj::Formal* node) {
    visit_tree_result = nullptr;
    visit_exp_result = nullptr;
}

// Nested: translate statement list
void ASTToTreeVisitor::visit(fdmj::Nested* node) {
    vector<tree::Stm*> *sl = new vector<tree::Stm*>();
    if (node->sl != nullptr) {
        for (auto stm : *node->sl) {
            stm->accept(*this);
            if (visit_exp_result != nullptr) {
                Tr_nx* nx = visit_exp_result->unNx(method_temp_map);
                if (nx != nullptr && nx->stm != nullptr) sl->push_back(nx->stm);
            }
        }
    }
    visit_exp_result = new Tr_nx(new tree::Seq(sl));
}

// If: translate condition + then + else
void ASTToTreeVisitor::visit(fdmj::If* node) {
    // Translate condition expression and get as Tr_cx first
    node->exp->accept(*this);
    Tr_Exp* cond = visit_exp_result;
    Tr_cx* cx = cond->unCx(method_temp_map);

    // Translate then-body
    tree::Stm* then_stm = nullptr;
    if (node->stm1 != nullptr) {
        node->stm1->accept(*this);
        if (visit_exp_result != nullptr) {
            Tr_nx* nx = visit_exp_result->unNx(method_temp_map);
            if (nx != nullptr) then_stm = nx->stm;
        }
    }

    // Translate else-body
    tree::Stm* else_stm = nullptr;
    if (node->stm2 != nullptr) {
        node->stm2->accept(*this);
        if (visit_exp_result != nullptr) {
            Tr_nx* nx = visit_exp_result->unNx(method_temp_map);
            if (nx != nullptr) else_stm = nx->stm;
        }
    }

    // Get condition as Tr_cx (already done above)
    // Allocate labels for if
    tree::Label* true_label = method_temp_map->newlabel();
    tree::Label* false_label = method_temp_map->newlabel();
    tree::Label* end_label = method_temp_map->newlabel();

    cx->true_list->patch(true_label);
    cx->false_list->patch(false_label);

    vector<tree::Stm*> *sl = new vector<tree::Stm*>();
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
void ASTToTreeVisitor::visit(fdmj::While* node) {
    // Save outer continue/break labels
    tree::Label* outer_continue = continue_label;
    tree::Label* outer_break = break_label;

    // Translate condition first (before allocating while labels)
    node->exp->accept(*this);
    Tr_Exp* cond = visit_exp_result;

    // Translate body (before allocating while labels to match label ordering)
    tree::Stm* body_stm = nullptr;
    // Set up continue/break labels - allocate them after body translation
    // But we need them during body translation for break/continue...
    // Actually, we need to allocate continue/break first, then translate body

    // Let me reconsider: from the expected outputs, the continue label is the
    // first while-specific label allocated. Let me check irtest2:
    // condition unCx: labels 100, 101; then continue=102, body=103, done=104

    // Get condition as Tr_cx
    Tr_cx* cx = cond->unCx(method_temp_map);

    // Allocate while labels
    tree::Label* start_label = method_temp_map->newlabel();
    tree::Label* body_label = method_temp_map->newlabel();
    tree::Label* done_label = method_temp_map->newlabel();

    continue_label = start_label;
    break_label = done_label;

    cx->true_list->patch(body_label);
    cx->false_list->patch(done_label);

    if (node->stm != nullptr) {
        node->stm->accept(*this);
        if (visit_exp_result != nullptr) {
            Tr_nx* nx = visit_exp_result->unNx(method_temp_map);
            if (nx != nullptr) body_stm = nx->stm;
        }
    }

    // Restore outer continue/break labels
    continue_label = outer_continue;
    break_label = outer_break;

    vector<tree::Stm*> *sl = new vector<tree::Stm*>();
    sl->push_back(new tree::LabelStm(start_label));
    sl->push_back(cx->stm);
    sl->push_back(new tree::LabelStm(body_label));
    if (body_stm != nullptr) sl->push_back(body_stm);
    sl->push_back(new tree::Jump(start_label));
    sl->push_back(new tree::LabelStm(done_label));

    visit_exp_result = new Tr_nx(new tree::Seq(sl));
}

// Assign: move src into dst
void ASTToTreeVisitor::visit(fdmj::Assign* node) {
    // Translate left (destination)
    node->left->accept(*this);
    Tr_Exp* left = visit_exp_result;

    // Translate right (source)
    node->exp->accept(*this);
    Tr_Exp* right = visit_exp_result;

    tree::Exp* dst = left->unEx(method_temp_map)->exp;
    tree::Exp* src = right->unEx(method_temp_map)->exp;

    visit_exp_result = new Tr_nx(new tree::Move(dst, src));
}

// CallStm: HW4 (class method calls)
void ASTToTreeVisitor::visit(fdmj::CallStm* node) {
    visit_exp_result = new Tr_nx(new tree::Seq());
}

// Continue: jump to continue label
void ASTToTreeVisitor::visit(fdmj::Continue* node) {
    if (continue_label != nullptr)
        visit_exp_result = new Tr_nx(new tree::Jump(continue_label));
    else
        visit_exp_result = new Tr_nx(new tree::Seq());
}

// Break: jump to break label
void ASTToTreeVisitor::visit(fdmj::Break* node) {
    if (break_label != nullptr)
        visit_exp_result = new Tr_nx(new tree::Jump(break_label));
    else
        visit_exp_result = new Tr_nx(new tree::Seq());
}

// Return: return expression
void ASTToTreeVisitor::visit(fdmj::Return* node) {
    if (node->exp != nullptr) {
        node->exp->accept(*this);
        Tr_Exp* exp = visit_exp_result;
        tree::Exp* ret_exp = exp->unEx(method_temp_map)->exp;
        visit_exp_result = new Tr_nx(new tree::Return(ret_exp));
    } else {
        visit_exp_result = new Tr_nx(new tree::Return(new tree::Const(0)));
    }
}

// PutInt: putint(exp) -> ExtCall("putint", {exp})
void ASTToTreeVisitor::visit(fdmj::PutInt* node) {
    node->exp->accept(*this);
    Tr_Exp* exp = visit_exp_result;
    tree::Exp* arg = exp->unEx(method_temp_map)->exp;
    vector<tree::Exp*> *args = new vector<tree::Exp*>();
    args->push_back(arg);
    tree::ExtCall* call = new tree::ExtCall(tree::Type::INT, "putint", args);
    visit_exp_result = new Tr_nx(new tree::ExpStm(call));
}

// PutCh: putch(exp) -> ExtCall("putch", {exp})
void ASTToTreeVisitor::visit(fdmj::PutCh* node) {
    node->exp->accept(*this);
    Tr_Exp* exp = visit_exp_result;
    tree::Exp* arg = exp->unEx(method_temp_map)->exp;
    vector<tree::Exp*> *args = new vector<tree::Exp*>();
    args->push_back(arg);
    tree::ExtCall* call = new tree::ExtCall(tree::Type::INT, "putch", args);
    visit_exp_result = new Tr_nx(new tree::ExpStm(call));
}

// PutArray: putarray(n, arr) -> ExtCall("putarray", {n, arr})
void ASTToTreeVisitor::visit(fdmj::PutArray* node) {
    node->n->accept(*this);
    tree::Exp* n_exp = visit_exp_result->unEx(method_temp_map)->exp;
    node->arr->accept(*this);
    tree::Exp* arr_exp = visit_exp_result->unEx(method_temp_map)->exp;
    vector<tree::Exp*> *args = new vector<tree::Exp*>();
    args->push_back(n_exp);
    args->push_back(arr_exp);
    tree::ExtCall* call = new tree::ExtCall(tree::Type::INT, "putarray", args);
    visit_exp_result = new Tr_nx(new tree::ExpStm(call));
}

// Starttime: ExtCall("starttime", {})
void ASTToTreeVisitor::visit(fdmj::Starttime* node) {
    vector<tree::Exp*> *args = new vector<tree::Exp*>();
    tree::ExtCall* call = new tree::ExtCall(tree::Type::INT, "starttime", args);
    visit_exp_result = new Tr_nx(new tree::ExpStm(call));
}

// Stoptime: ExtCall("stoptime", {})
void ASTToTreeVisitor::visit(fdmj::Stoptime* node) {
    vector<tree::Exp*> *args = new vector<tree::Exp*>();
    tree::ExtCall* call = new tree::ExtCall(tree::Type::INT, "stoptime", args);
    visit_exp_result = new Tr_nx(new tree::ExpStm(call));
}

// BinaryOp: translate binary operations
void ASTToTreeVisitor::visit(fdmj::BinaryOp* node) {
    string op = node->op->op;

    // Short-circuit logical operators
    if (op == "&&") {
        // left && right: left true -> check right; left false -> overall false
        node->left->accept(*this);
        Tr_Exp* left = visit_exp_result;
        Tr_cx* left_cx = left->unCx(method_temp_map);

        node->right->accept(*this);
        Tr_Exp* right = visit_exp_result;
        Tr_cx* right_cx = right->unCx(method_temp_map);

        // left true -> evaluate right
        tree::Label* middle = method_temp_map->newlabel();
        left_cx->true_list->patch(middle);

        // Build combined statement
        vector<tree::Stm*> *sl = new vector<tree::Stm*>();
        sl->push_back(left_cx->stm);
        sl->push_back(new tree::LabelStm(middle));
        sl->push_back(right_cx->stm);

        // Combined: true_list = right's true, false_list = left's false + right's false
        Patch_list* combined_false = left_cx->false_list;
        combined_false->add(right_cx->false_list);

        visit_exp_result = new Tr_cx(right_cx->true_list, combined_false, new tree::Seq(sl));
        return;
    }

    if (op == "||") {
        // left || right: left true -> overall true; left false -> check right
        node->left->accept(*this);
        Tr_Exp* left = visit_exp_result;
        Tr_cx* left_cx = left->unCx(method_temp_map);

        node->right->accept(*this);
        Tr_Exp* right = visit_exp_result;
        Tr_cx* right_cx = right->unCx(method_temp_map);

        // left false -> evaluate right
        tree::Label* middle = method_temp_map->newlabel();
        left_cx->false_list->patch(middle);

        // Build combined statement
        vector<tree::Stm*> *sl = new vector<tree::Stm*>();
        sl->push_back(left_cx->stm);
        sl->push_back(new tree::LabelStm(middle));
        sl->push_back(right_cx->stm);

        // Combined: true_list = left's true + right's true, false_list = right's false
        Patch_list* combined_true = left_cx->true_list;
        combined_true->add(right_cx->true_list);

        visit_exp_result = new Tr_cx(combined_true, right_cx->false_list, new tree::Seq(sl));
        return;
    }

    // Comparison operators: produce Tr_cx
    if (op == "<" || op == ">" || op == "<=" || op == ">=" || op == "==" || op == "!=") {
        node->left->accept(*this);
        Tr_Exp* left = visit_exp_result;
        tree::Exp* left_exp = left->unEx(method_temp_map)->exp;

        node->right->accept(*this);
        Tr_Exp* right = visit_exp_result;
        tree::Exp* right_exp = right->unEx(method_temp_map)->exp;

        tree::Label* tl = method_temp_map->newlabel();
        tree::Label* fl = method_temp_map->newlabel();
        Patch_list* true_list = new Patch_list();
        true_list->add_patch(tl);
        Patch_list* false_list = new Patch_list();
        false_list->add_patch(fl);

        tree::Cjump* cjump = new tree::Cjump(op, left_exp, right_exp, tl, fl);
        visit_exp_result = new Tr_cx(true_list, false_list, cjump);
        return;
    }

    // Arithmetic operators: +, -, *, /
    node->left->accept(*this);
    Tr_Exp* left = visit_exp_result;
    tree::Exp* left_exp = left->unEx(method_temp_map)->exp;

    node->right->accept(*this);
    Tr_Exp* right = visit_exp_result;
    tree::Exp* right_exp = right->unEx(method_temp_map)->exp;

    tree::Binop* binop = new tree::Binop(tree::Type::INT, op, left_exp, right_exp);
    visit_exp_result = new Tr_ex(binop);
}

// UnaryOp: translate unary operations
void ASTToTreeVisitor::visit(fdmj::UnaryOp* node) {
    string op = node->op->op;
    node->exp->accept(*this);
    Tr_Exp* operand = visit_exp_result;

    if (op == "-") {
        // -exp -> 0 - exp
        tree::Exp* exp = operand->unEx(method_temp_map)->exp;
        visit_exp_result = new Tr_ex(new tree::Binop(tree::Type::INT, "-", new tree::Const(0), exp));
    } else if (op == "!") {
        // !exp -> 1 xor exp (or flip the cx lists)
        Tr_cx* cx = operand->unCx(method_temp_map);
        // Flip true and false lists
        visit_exp_result = new Tr_cx(cx->false_list, cx->true_list, cx->stm);
    } else {
        visit_exp_result = operand;
    }
}

// ArrayExp: arr[index] -> HW4 (not needed in HW3)
void ASTToTreeVisitor::visit(fdmj::ArrayExp* node) {
    visit_exp_result = new Tr_ex(new tree::Const(0));
}

// CallExp: HW4
void ASTToTreeVisitor::visit(fdmj::CallExp* node) {
    visit_exp_result = new Tr_ex(new tree::Const(0));
}

// ClassVar: HW4
void ASTToTreeVisitor::visit(fdmj::ClassVar* node) {
    visit_exp_result = new Tr_ex(new tree::Const(0));
}

// This: HW4
void ASTToTreeVisitor::visit(fdmj::This* node) {
    visit_exp_result = new Tr_ex(new tree::Const(0));
}

// Length: HW4 (array length)
void ASTToTreeVisitor::visit(fdmj::Length* node) {
    visit_exp_result = new Tr_ex(new tree::Const(0));
}

// NewArray: HW4
void ASTToTreeVisitor::visit(fdmj::NewArray* node) {
    visit_exp_result = new Tr_ex(new tree::Const(0));
}

// NewObject: HW4
void ASTToTreeVisitor::visit(fdmj::NewObject* node) {
    visit_exp_result = new Tr_ex(new tree::Const(0));
}

// GetInt: getint() -> ExtCall("getint", {})
void ASTToTreeVisitor::visit(fdmj::GetInt* node) {
    vector<tree::Exp*> *args = new vector<tree::Exp*>();
    tree::ExtCall* call = new tree::ExtCall(tree::Type::INT, "getint", args);
    visit_exp_result = new Tr_ex(call);
}

// GetCh: getch() -> ExtCall("getch", {})
void ASTToTreeVisitor::visit(fdmj::GetCh* node) {
    vector<tree::Exp*> *args = new vector<tree::Exp*>();
    tree::ExtCall* call = new tree::ExtCall(tree::Type::INT, "getch", args);
    visit_exp_result = new Tr_ex(call);
}

// GetArray: getarray(exp) -> ExtCall("getarray", {exp})
void ASTToTreeVisitor::visit(fdmj::GetArray* node) {
    node->exp->accept(*this);
    tree::Exp* arg = visit_exp_result->unEx(method_temp_map)->exp;
    vector<tree::Exp*> *args = new vector<tree::Exp*>();
    args->push_back(arg);
    tree::ExtCall* call = new tree::ExtCall(tree::Type::PTR, "getarray", args);
    visit_exp_result = new Tr_ex(call);
}

// IdExp: look up variable in method var table
void ASTToTreeVisitor::visit(fdmj::IdExp* node) {
    string name = node->id;
    tree::Temp* temp = method_var_table->get_var_temp(name);
    if (temp != nullptr) {
        tree::Type t = method_var_table->get_var_type(name);
        visit_exp_result = new Tr_ex(new tree::TempExp(t, new tree::Temp(temp->num)));
    } else {
        // Variable not found - should not happen in correct programs
        visit_exp_result = new Tr_ex(new tree::Const(0));
    }
}

// OpExp: not directly visited (handled in BinaryOp/UnaryOp)
void ASTToTreeVisitor::visit(fdmj::OpExp* node) {
    visit_exp_result = nullptr;
}

// IntExp: constant integer
void ASTToTreeVisitor::visit(fdmj::IntExp* node) {
    visit_exp_result = new Tr_ex(new tree::Const(node->val));
}
