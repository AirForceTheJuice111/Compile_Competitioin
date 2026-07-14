#include "function_dce.hh"
#include <map>
#include <set>
#include <string>
#include <vector>
#include "quad_metadata.hh"
namespace quad { namespace {
void noteTerm(QuadTerm *term, const std::map<std::string, QuadFuncDecl *> &functions,
              std::set<std::string> &references) {
    if (term != nullptr && term->kind == QuadTermKind::NAME &&
        functions.count(term->get_name()) != 0) references.insert(term->get_name());
}
void noteCall(QuadCall *call, const std::map<std::string, QuadFuncDecl *> &functions,
              std::set<std::string> &references) {
    if (call == nullptr) return;
    if (functions.count(call->name) != 0) references.insert(call->name);
    noteTerm(call->obj_term, functions, references);
    if (call->args != nullptr) for (auto *a : *call->args) noteTerm(a, functions, references);
}
void noteExtCall(QuadExtCall *call, const std::map<std::string, QuadFuncDecl *> &functions,
                 std::set<std::string> &references) {
    if (call == nullptr) return;
    if (functions.count(call->extfun) != 0) references.insert(call->extfun);
    if (call->args != nullptr) for (auto *a : *call->args) noteTerm(a, functions, references);
}
std::set<std::string> refs(QuadFuncDecl *f,
    const std::map<std::string, QuadFuncDecl *> &functions) {
    std::set<std::string> r;
    if (f == nullptr || f->quadblocklist == nullptr) return r;
    for (auto *b : *f->quadblocklist) {
        if (b == nullptr || b->quadlist == nullptr) continue;
        for (auto *st : *b->quadlist) {
            if (st == nullptr) continue;
            switch (st->kind) {
            case QuadKind::MOVE: noteTerm(static_cast<QuadMove*>(st)->src,functions,r); break;
            case QuadKind::LOAD: noteTerm(static_cast<QuadLoad*>(st)->src,functions,r); break;
            case QuadKind::STORE: { auto*x=static_cast<QuadStore*>(st); noteTerm(x->src,functions,r); noteTerm(x->dst,functions,r); break; }
            case QuadKind::MOVE_BINOP: { auto*x=static_cast<QuadMoveBinop*>(st); noteTerm(x->left,functions,r); noteTerm(x->right,functions,r); break; }
            case QuadKind::CALL: noteCall(static_cast<QuadCall*>(st),functions,r); break;
            case QuadKind::MOVE_CALL: noteCall(static_cast<QuadMoveCall*>(st)->call,functions,r); break;
            case QuadKind::EXTCALL: noteExtCall(static_cast<QuadExtCall*>(st),functions,r); break;
            case QuadKind::MOVE_EXTCALL: noteExtCall(static_cast<QuadMoveExtCall*>(st)->extcall,functions,r); break;
            case QuadKind::CJUMP: { auto*x=static_cast<QuadCJump*>(st); noteTerm(x->left,functions,r); noteTerm(x->right,functions,r); break; }
            case QuadKind::RETURN: noteTerm(static_cast<QuadReturn*>(st)->exp,functions,r); break;
            case QuadKind::PTR_CALC: { auto*x=static_cast<QuadPtrCalc*>(st); noteTerm(x->dst,functions,r); noteTerm(x->ptr,functions,r); noteTerm(x->offset,functions,r); break; }
            default: break;
            }
        }
    }
    return r;
}
}}
namespace quad {
QuadProgram *eliminateDeadFunctions(QuadProgram *program, int *removedOut) {
    if (removedOut) *removedOut=0;
    if (program==nullptr || program->quadFuncDeclList==nullptr) return program;
    std::map<std::string,QuadFuncDecl*> functions;
    for(auto*f:*program->quadFuncDeclList) if(f) functions[f->funcname]=f;
    if(functions.count("main")==0) return program;
    std::set<std::string> reachable{"main"}; std::vector<std::string> work{"main"};
    while(!work.empty()) { auto name=work.back(); work.pop_back(); auto it=functions.find(name); if(it==functions.end()) continue;
        for(const auto&r:refs(it->second,functions)) if(reachable.insert(r).second) work.push_back(r);
    }
    auto *kept=new std::vector<QuadFuncDecl*>(); int removed=0;
    for(auto*f:*program->quadFuncDeclList) { if(!f) continue; if(reachable.count(f->funcname)) kept->push_back(f); else ++removed; }
    auto *result=new QuadProgram(kept,program->last_label_num,program->last_temp_num);
    rebuildQuadProgramMetadata(result); if(removedOut)*removedOut=removed; return result;
}
}
