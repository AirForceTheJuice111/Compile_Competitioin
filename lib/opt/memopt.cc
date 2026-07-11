#include "memopt.hh"
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "quad.hh"
#include "flowinfo.hh"
#include "temp.hh"
using namespace std;

namespace {
set<Temp*>* es() { return new set<Temp*>(); }
set<Temp*>* os(int n) { auto* s = new set<Temp*>(); s->insert(new Temp(n)); return s; }
using MemKey = int; const MemKey BAD = -1;
MemKey key(quad::QuadTerm* a) { return (a&&a->kind==quad::QuadTermKind::TEMP)?a->get_temp()->temp->num:BAD; }
bool call(quad::QuadStm* s) { if(!s)return false; auto k=s->kind; return k==quad::QuadKind::CALL||k==quad::QuadKind::EXTCALL||k==quad::QuadKind::MOVE_CALL||k==quad::QuadKind::MOVE_EXTCALL; }

struct Mem { map<MemKey,int> val; map<MemKey,int> pos; set<int> dead; };

void memFn(quad::QuadFuncDecl* f, ControlFlowInfo* cfi, int& elim) {
    if(!f||!f->quadblocklist||!cfi) return;
    map<int,Mem> bs;
    std::function<void(int)> walk=[&](int L){
        auto it=cfi->labelToBlock.find(L); if(it==cfi->labelToBlock.end())return;
        auto* b=it->second; if(!b||!b->quadlist)return;
        Mem m; int id=-1; auto ii=cfi->immediateDominator.find(L); if(ii!=cfi->immediateDominator.end())id=ii->second;
        if(id>=0&&bs.count(id)) m=bs[id];
        auto* nl=new vector<quad::QuadStm*>();
        for(auto* s:*b->quadlist){if(!s)continue;
            if(call(s)){m.val.clear();m.pos.clear();nl->push_back(s);continue;}
            if(s->kind==quad::QuadKind::STORE){auto* st=static_cast<quad::QuadStore*>(s); MemKey k=key(st->dst);
                if(k==BAD){m.val.clear();m.pos.clear();nl->push_back(s);continue;}
                int vt=-1; if(st->src&&st->src->kind==quad::QuadTermKind::TEMP)vt=st->src->get_temp()->temp->num;
                if(m.pos.count(k)){m.dead.insert(m.pos[k]);elim++;}
                m.val[k]=vt; m.pos[k]=(int)nl->size(); nl->push_back(st); continue;}
            if(s->kind==quad::QuadKind::LOAD){auto* ld=static_cast<quad::QuadLoad*>(s); MemKey k=key(ld->src);
                if(k==BAD){nl->push_back(s);continue;}
                if(m.val.count(k)&&m.val[k]>=0){int dn=ld->dst->temp->num,sn=m.val[k];
                    auto* src=new quad::QuadTerm(new quad::QuadTemp(new Temp(sn),ld->dst->type));
                    nl->push_back(new quad::QuadMove(ld->dst->clone(),src,os(dn),os(sn))); elim++; continue;}
                nl->push_back(s); continue;}
            nl->push_back(s);}
        if(!m.dead.empty()){auto* fl=new vector<quad::QuadStm*>(); for(int i=0;i<(int)nl->size();i++)if(!m.dead.count(i))fl->push_back((*nl)[i]); nl=fl;}
        b->quadlist=nl; bs[L]=m;
        auto dc=cfi->domTree.find(L); if(dc!=cfi->domTree.end())for(int c:dc->second)walk(c);
    };
    if(cfi->entryBlock>=0) walk(cfi->entryBlock);
}
} // namespace

namespace quad {
QuadProgram* memOptProg(QuadProgram* p, set<FuncFlowInfo*>* fl, int* eo) {
    if(!p||!fl)return p; int e=0;
    for(auto* ff:*fl)if(ff&&ff->cfi&&ff->cfi->func)memFn(ff->cfi->func,ff->cfi,e);
    if(eo)*eo=e; return p;
}
} // namespace quad
