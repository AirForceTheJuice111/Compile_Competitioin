#include "inline.hh"
#include <map>
#include <set>
#include <string>
#include <vector>
#include "quad.hh"
#include "temp.hh"
using namespace std;

namespace {
static const int MAX_INLINE_INSTS = 30;

set<Temp*>* es() { return new set<Temp*>(); }
set<Temp*>* os(int n) { auto* s = new set<Temp*>(); s->insert(new Temp(n)); return s; }

string callName(quad::QuadStm* s) {
    if (!s) return "";
    if (s->kind == quad::QuadKind::CALL) return static_cast<quad::QuadCall*>(s)->name;
    if (s->kind == quad::QuadKind::MOVE_CALL) { auto* mc = static_cast<quad::QuadMoveCall*>(s); return mc->call ? mc->call->name : ""; }
    if (s->kind == quad::QuadKind::EXTCALL) return static_cast<quad::QuadExtCall*>(s)->extfun;
    if (s->kind == quad::QuadKind::MOVE_EXTCALL) { auto* me = static_cast<quad::QuadMoveExtCall*>(s); return me->extcall ? me->extcall->extfun : ""; }
    return "";
}

bool isLeaf(quad::QuadFuncDecl* f, const set<string>& uf) {
    if (!f || !f->quadblocklist) return true;
    for (auto* b : *f->quadblocklist) { if (!b || !b->quadlist) continue; for (auto* s : *b->quadlist) if (s && uf.count(callName(s))) return false; }
    return true;
}
int countInsts(quad::QuadFuncDecl* f) { if (!f || !f->quadblocklist) return 0; int n=0; for (auto* b:*f->quadblocklist) if(b&&b->quadlist) n+=b->quadlist->size(); return n; }
bool isSingleBlock(quad::QuadFuncDecl* f) { return f && f->quadblocklist && f->quadblocklist->size()==1; }
bool isRecursive(quad::QuadFuncDecl* f) {
    if (!f) return false; string n=f->funcname; if(!f->quadblocklist) return false;
    for (auto* b:*f->quadblocklist){if(!b||!b->quadlist)continue;for(auto* s:*b->quadlist)if(s&&callName(s)==n)return true;}
    return false;
}

quad::QuadTerm* cloneTerm(quad::QuadTerm* t, const map<int,int>& rm) {
    if (!t) return nullptr;
    if (t->kind == quad::QuadTermKind::CONST) return new quad::QuadTerm(t->get_const());
    if (t->kind == quad::QuadTermKind::TEMP) {
        int on=t->get_temp()->temp->num, nn=on;
        auto it=rm.find(on); if(it!=rm.end()) nn=it->second;
        return new quad::QuadTerm(new quad::QuadTemp(new Temp(nn),t->get_temp()->type));
    }
    return new quad::QuadTerm(t->get_name());
}

void remapInst(quad::QuadStm* cl, map<int,int>& rm, int& gt, map<int,int>& rm2) {
    auto fresh = [&](int on)->int{ if(rm2.count(on))return rm2[on]; int n=++gt; rm2[on]=n; rm[on]=n; return n; };
    if (cl->def) { auto* nd=new set<Temp*>(); for(auto* t:*cl->def)if(t)nd->insert(new Temp(fresh(t->num))); cl->def=nd; }
    if (cl->use) { auto* nu=new set<Temp*>(); for(auto* t:*cl->use)if(t)nu->insert(new Temp(rm.count(t->num)?rm.at(t->num):t->num)); cl->use=nu; }
    switch(cl->kind) {
    case quad::QuadKind::MOVE: { auto* mv=static_cast<quad::QuadMove*>(cl); mv->dst=new quad::QuadTemp(new Temp(fresh(mv->dst->temp->num)),mv->dst->type); if(mv->src)mv->src=cloneTerm(mv->src,rm); break; }
    case quad::QuadKind::MOVE_BINOP: { auto* bp=static_cast<quad::QuadMoveBinop*>(cl); bp->dst=new quad::QuadTemp(new Temp(fresh(bp->dst->temp->num)),bp->dst->type); if(bp->left)bp->left=cloneTerm(bp->left,rm); if(bp->right)bp->right=cloneTerm(bp->right,rm); break; }
    case quad::QuadKind::LOAD: { auto* ld=static_cast<quad::QuadLoad*>(cl); ld->dst=new quad::QuadTemp(new Temp(fresh(ld->dst->temp->num)),ld->dst->type); if(ld->src)ld->src=cloneTerm(ld->src,rm); break; }
    case quad::QuadKind::STORE: { auto* st=static_cast<quad::QuadStore*>(cl); if(st->src)st->src=cloneTerm(st->src,rm); if(st->dst)st->dst=cloneTerm(st->dst,rm); break; }
    case quad::QuadKind::RETURN: { auto* rt=static_cast<quad::QuadReturn*>(cl); if(rt->exp)rt->exp=cloneTerm(rt->exp,rm); break; }
    case quad::QuadKind::CJUMP: { auto* cj=static_cast<quad::QuadCJump*>(cl); if(cj->left)cj->left=cloneTerm(cj->left,rm); if(cj->right)cj->right=cloneTerm(cj->right,rm); break; }
    case quad::QuadKind::PTR_CALC: { auto* pc=static_cast<quad::QuadPtrCalc*>(cl); if(pc->dst&&pc->dst->kind==quad::QuadTermKind::TEMP)pc->dst=new quad::QuadTerm(new quad::QuadTemp(new Temp(fresh(pc->dst->get_temp()->temp->num)),pc->dst->get_temp()->type)); if(pc->ptr)pc->ptr=cloneTerm(pc->ptr,rm); if(pc->offset)pc->offset=cloneTerm(pc->offset,rm); break; }
    case quad::QuadKind::PHI: { auto* ph=static_cast<quad::QuadPhi*>(cl); ph->temp_exp=new quad::QuadTemp(new Temp(fresh(ph->temp_exp->temp->num)),ph->temp_exp->type); if(ph->args)for(auto& a:*ph->args)if(a.first)a.first=new Temp(rm.count(a.first->num)?rm.at(a.first->num):a.first->num); break; }
    default: break;
    }
}

void inlineProgImpl(quad::QuadProgram* prog, int& inlined) {
    if (!prog||!prog->quadFuncDeclList) return;
    auto& funcs=*prog->quadFuncDeclList;
    map<string,quad::QuadFuncDecl*> nm; set<string> uf;
    for(auto* f:funcs){if(f){nm[f->funcname]=f;uf.insert(f->funcname);}}

    set<string> eligible;
    for(auto& kv:nm){ auto* f=kv.second; if(!isSingleBlock(f)||isRecursive(f)||countInsts(f)>MAX_INLINE_INSTS||!isLeaf(f,uf))continue; eligible.insert(kv.first); }
    if(eligible.empty()) return;

    int gt=prog->last_temp_num;
    for(auto* caller:funcs){ if(!caller||!caller->quadblocklist)continue;
        for(auto* block:*caller->quadblocklist){ if(!block||!block->quadlist)continue;
            auto* nl=new vector<quad::QuadStm*>();
            for(auto* stm:*block->quadlist){ if(!stm)continue;
                string cn=callName(stm); quad::QuadTemp* dst=nullptr;
                if(stm->kind==quad::QuadKind::MOVE_CALL){ dst=static_cast<quad::QuadMoveCall*>(stm)->dst; }
                if(stm->kind==quad::QuadKind::MOVE_EXTCALL){ dst=static_cast<quad::QuadMoveExtCall*>(stm)->dst; }
                if(cn.empty()||!eligible.count(cn)){ nl->push_back(stm); continue; }

                auto* callee=nm[cn]; if(!callee||!callee->quadblocklist||callee->quadblocklist->empty()){nl->push_back(stm);continue;}
                auto* cb=callee->quadblocklist->at(0); if(!cb||!cb->quadlist){nl->push_back(stm);continue;}

                map<int,int> rm; map<int,int> rm2;
                auto fresh=[&](int on)->int{ if(rm2.count(on))return rm2[on]; int n=++gt; rm2[on]=n; rm[on]=n; return n; };

                // Param moves: extract args from the call
                vector<quad::QuadTerm*>* args=nullptr;
                if(stm->kind==quad::QuadKind::CALL) args=static_cast<quad::QuadCall*>(stm)->args;
                else if(stm->kind==quad::QuadKind::MOVE_CALL) args=static_cast<quad::QuadMoveCall*>(stm)->call->args;
                else if(stm->kind==quad::QuadKind::EXTCALL) args=static_cast<quad::QuadExtCall*>(stm)->args;
                else if(stm->kind==quad::QuadKind::MOVE_EXTCALL) args=static_cast<quad::QuadMoveExtCall*>(stm)->extcall->args;

                map<int,quad::QuadTerm*> pm;
                if(callee->params&&args){ int np=callee->params->size(), na=args->size(); for(int i=0;i<np&&i<na;i++) pm[(*callee->params)[i]->num]=(*args)[i]->clone(); }
                for(auto& kv2:pm){ int npn=fresh(kv2.first); auto* mv=new quad::QuadMove(new quad::QuadTemp(new Temp(npn),quad::QuadType::INT),kv2.second,os(npn),es()); if(kv2.second->kind==quad::QuadTermKind::TEMP)mv->use->insert(new Temp(kv2.second->get_temp()->temp->num)); nl->push_back(mv); }

                for(auto* cs:*cb->quadlist){ if(!cs)continue; if(cs->kind==quad::QuadKind::LABEL)continue;
                    if(cs->kind==quad::QuadKind::RETURN){ auto* ret=static_cast<quad::QuadReturn*>(cs);
                        if(dst&&ret->exp){ auto* src=cloneTerm(ret->exp,rm); auto* mv=new quad::QuadMove(dst->clone(),src,os(dst->temp->num),es()); if(src->kind==quad::QuadTermKind::TEMP)mv->use->insert(new Temp(src->get_temp()->temp->num)); nl->push_back(mv); } continue; }
                    auto* cl=static_cast<quad::QuadStm*>(cs->clone()); remapInst(cl,rm,gt,rm2); nl->push_back(cl); }
                inlined++;
            }
            block->quadlist=nl;
        }
        caller->last_temp_num=gt;
    }
    prog->last_temp_num=gt;
}
} // namespace

namespace quad {
QuadProgram* inlineProg(QuadProgram* prog, int* eo) { if(!prog)return prog; int e=0; inlineProgImpl(prog,e); if(eo)*eo=e; return prog; }
} // namespace quad
