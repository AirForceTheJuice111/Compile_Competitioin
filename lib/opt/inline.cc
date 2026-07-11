#include "inline.hh"
#include <map>
#include <set>
#include <string>
#include <vector>
#include "quad.hh"
#include "temp.hh"
using namespace std;

namespace {
static const int MAX_INLINE_INSTS = 60;

set<Temp*>* es() { return new set<Temp*>(); }

string callName(quad::QuadStm* s) {
    if(!s)return"";
    switch(s->kind){
    case quad::QuadKind::CALL: return static_cast<quad::QuadCall*>(s)->name;
    case quad::QuadKind::EXTCALL: return static_cast<quad::QuadExtCall*>(s)->extfun;
    case quad::QuadKind::MOVE_CALL:{auto*m=static_cast<quad::QuadMoveCall*>(s);return m->call?m->call->name:"";}
    case quad::QuadKind::MOVE_EXTCALL:{auto*m=static_cast<quad::QuadMoveExtCall*>(s);return m->extcall?m->extcall->extfun:"";}
    default:return"";
    }
}
bool isRecursive(quad::QuadFuncDecl* f) {
    if(!f||!f->quadblocklist)return false; string n=f->funcname;
    for(auto*b:*f->quadblocklist){if(!b||!b->quadlist)continue;for(auto*s:*b->quadlist)if(s&&callName(s)==n)return true;}
    return false;
}
int countInsts(quad::QuadFuncDecl* f) { if(!f||!f->quadblocklist)return 0;int n=0;for(auto*b:*f->quadblocklist)if(b&&b->quadlist)n+=b->quadlist->size();return n; }

quad::QuadTerm* cloneTerm(quad::QuadTerm* t, const map<int,int>& rm) {
    if(!t)return nullptr;
    if(t->kind==quad::QuadTermKind::CONST)return new quad::QuadTerm(t->get_const());
    if(t->kind==quad::QuadTermKind::TEMP){int on=t->get_temp()->temp->num,nn=on;auto it=rm.find(on);if(it!=rm.end())nn=it->second;return new quad::QuadTerm(new quad::QuadTemp(new Temp(nn),t->get_temp()->type));}
    return new quad::QuadTerm(t->get_name());
}

void remapStm(quad::QuadStm* cl, map<int,int>& rm, int& gt, map<int,int>& rm2, map<int,int>& lm) {
    auto fresh=[&](int on)->int{if(rm2.count(on))return rm2[on];int n=++gt;rm2[on]=n;rm[on]=n;return n;};
    if(cl->def){auto* nd=new set<Temp*>();for(auto*t:*cl->def)if(t)nd->insert(new Temp(fresh(t->num)));cl->def=nd;}
    if(cl->use){auto* nu=new set<Temp*>();for(auto*t:*cl->use)if(t)nu->insert(new Temp(rm.count(t->num)?rm.at(t->num):t->num));cl->use=nu;}
    switch(cl->kind){
    case quad::QuadKind::JUMP:{auto*j=static_cast<quad::QuadJump*>(cl);j->label=new Label(lm.count(j->label->num)?lm.at(j->label->num):j->label->num);break;}
    case quad::QuadKind::CJUMP:{auto*c=static_cast<quad::QuadCJump*>(cl);if(c->left)c->left=cloneTerm(c->left,rm);if(c->right)c->right=cloneTerm(c->right,rm);if(c->t)c->t=new Label(lm.count(c->t->num)?lm.at(c->t->num):c->t->num);if(c->f)c->f=new Label(lm.count(c->f->num)?lm.at(c->f->num):c->f->num);break;}
    case quad::QuadKind::MOVE:{auto*mv=static_cast<quad::QuadMove*>(cl);mv->dst=new quad::QuadTemp(new Temp(fresh(mv->dst->temp->num)),mv->dst->type);if(mv->src)mv->src=cloneTerm(mv->src,rm);break;}
    case quad::QuadKind::MOVE_BINOP:{auto*bp=static_cast<quad::QuadMoveBinop*>(cl);bp->dst=new quad::QuadTemp(new Temp(fresh(bp->dst->temp->num)),bp->dst->type);if(bp->left)bp->left=cloneTerm(bp->left,rm);if(bp->right)bp->right=cloneTerm(bp->right,rm);break;}
    case quad::QuadKind::LOAD:{auto*ld=static_cast<quad::QuadLoad*>(cl);ld->dst=new quad::QuadTemp(new Temp(fresh(ld->dst->temp->num)),ld->dst->type);if(ld->src)ld->src=cloneTerm(ld->src,rm);break;}
    case quad::QuadKind::STORE:{auto*st=static_cast<quad::QuadStore*>(cl);if(st->src)st->src=cloneTerm(st->src,rm);if(st->dst)st->dst=cloneTerm(st->dst,rm);break;}
    case quad::QuadKind::PTR_CALC:{auto*pc=static_cast<quad::QuadPtrCalc*>(cl);if(pc->dst&&pc->dst->kind==quad::QuadTermKind::TEMP)pc->dst=new quad::QuadTerm(new quad::QuadTemp(new Temp(fresh(pc->dst->get_temp()->temp->num)),pc->dst->get_temp()->type));if(pc->ptr)pc->ptr=cloneTerm(pc->ptr,rm);if(pc->offset)pc->offset=cloneTerm(pc->offset,rm);break;}
    case quad::QuadKind::PHI:{auto*ph=static_cast<quad::QuadPhi*>(cl);ph->temp_exp=new quad::QuadTemp(new Temp(fresh(ph->temp_exp->temp->num)),ph->temp_exp->type);if(ph->args)for(auto&a:*ph->args){if(a.first)a.first=new Temp(rm.count(a.first->num)?rm.at(a.first->num):a.first->num);if(a.second)a.second=new Label(lm.count(a.second->num)?lm.at(a.second->num):a.second->num);}break;}
    default:break;
    }
}

void inlineProgImpl(quad::QuadProgram* prog, int& inlined) {
    if(!prog||!prog->quadFuncDeclList)return;
    auto&funcs=*prog->quadFuncDeclList;
    map<string,quad::QuadFuncDecl*>nm;set<string>uf;
    for(auto*f:funcs){if(f){nm[f->funcname]=f;uf.insert(f->funcname);}}
    set<string>eligible;
    for(auto&kv:nm){auto*f=kv.second;if(!isRecursive(f)&&countInsts(f)<=MAX_INLINE_INSTS)eligible.insert(kv.first);}
    if(eligible.empty())return;

    int gl=prog->last_label_num, gt=prog->last_temp_num;
    auto*newFuncs=new vector<quad::QuadFuncDecl*>();

    for(auto*caller:funcs){
        if(!caller||!caller->quadblocklist){newFuncs->push_back(caller);continue;}
        auto*newBlocks=new vector<quad::QuadBlock*>();
        bool modified=false;

        for(auto*block:*caller->quadblocklist){
            if(!block||!block->quadlist){newBlocks->push_back(block);continue;}

            // Find call sites in this block
            auto&stmts=*block->quadlist;
            for(size_t i=0;i<stmts.size();i++){
                auto*stm=stmts[i];
                if(!stm)continue;
                string cn=callName(stm);
                quad::QuadTemp*dst=nullptr;
                if(stm->kind==quad::QuadKind::MOVE_CALL)dst=static_cast<quad::QuadMoveCall*>(stm)->dst;
                if(stm->kind==quad::QuadKind::MOVE_EXTCALL)dst=static_cast<quad::QuadMoveExtCall*>(stm)->dst;
                if(cn.empty()||!eligible.count(cn))continue;
                auto*callee=nm[cn];
                if(!callee||!callee->quadblocklist||callee->quadblocklist->empty())continue;

                // --- Split block and inline ---
                // Prefix: instructions 0..i-1
                auto*prefix=new vector<quad::QuadStm*>();
                for(size_t j=0;j<i;j++)prefix->push_back(stmts[j]);

                // Suffix (continuation): instructions i+1..end
                int contLabel=++gl;
                auto*suffix=new vector<quad::QuadStm*>();
                for(size_t j=i+1;j<stmts.size();j++)suffix->push_back(stmts[j]);

                // Build label map for callee
                map<int,int>lm;
                for(auto*cb:*callee->quadblocklist)if(cb&&cb->entry_label){int nl=++gl;lm[cb->entry_label->num]=nl;}

                // Temp remap
                map<int,int>rm,rm2;
                auto fresh=[&](int on)->int{if(rm2.count(on))return rm2[on];int n=++gt;rm2[on]=n;rm[on]=n;return n;};

                // Extract args
                vector<quad::QuadTerm*>*args=nullptr;
                switch(stm->kind){
                case quad::QuadKind::CALL:args=static_cast<quad::QuadCall*>(stm)->args;break;
                case quad::QuadKind::EXTCALL:args=static_cast<quad::QuadExtCall*>(stm)->args;break;
                case quad::QuadKind::MOVE_CALL:args=static_cast<quad::QuadMoveCall*>(stm)->call->args;break;
                case quad::QuadKind::MOVE_EXTCALL:args=static_cast<quad::QuadMoveExtCall*>(stm)->extcall->args;break;
                default:break;
                }
                map<int,quad::QuadTerm*>pm;
                if(callee->params&&args){int np=callee->params->size(),na=args->size();for(int j=0;j<np&&j<na;j++)pm[(*callee->params)[j]->num]=(*args)[j]->clone();}

                // Clone callee blocks
                int calleeEntryLabel=lm[callee->quadblocklist->at(0)->entry_label->num];
                vector<quad::QuadBlock*>clonedBlocks;

                for(auto*cb:*callee->quadblocklist){
                    if(!cb||!cb->quadlist)continue;
                    auto*cl=new vector<quad::QuadStm*>();

                    // Param moves at entry block
                    if(cb==callee->quadblocklist->at(0)){
                        for(auto&kv2:pm){
                            int npn=fresh(kv2.first);
                            auto*mv=new quad::QuadMove(new quad::QuadTemp(new Temp(npn),quad::QuadType::INT),kv2.second,new set<Temp*>(),new set<Temp*>());
                            mv->def->insert(new Temp(npn));
                            if(kv2.second->kind==quad::QuadTermKind::TEMP)mv->use->insert(new Temp(kv2.second->get_temp()->temp->num));
                            cl->push_back(mv);
                        }
                    }

                    for(auto*cs:*cb->quadlist){
                        if(!cs)continue;
                        if(cs->kind==quad::QuadKind::LABEL)continue;

                        if(cs->kind==quad::QuadKind::RETURN){
                            auto*ret=static_cast<quad::QuadReturn*>(cs);
                            if(dst&&ret->exp){
                                auto*src=cloneTerm(ret->exp,rm);
                                auto*mv=new quad::QuadMove(dst->clone(),src,new set<Temp*>(),new set<Temp*>());
                                mv->def->insert(new Temp(dst->temp->num));
                                if(src->kind==quad::QuadTermKind::TEMP)mv->use->insert(new Temp(src->get_temp()->temp->num));
                                cl->push_back(mv);
                            }
                            cl->push_back(new quad::QuadJump(new Label(contLabel),new set<Temp*>(),new set<Temp*>()));
                            continue;
                        }

                        auto*cc=static_cast<quad::QuadStm*>(cs->clone());
                        remapStm(cc,rm,gt,rm2,lm);
                        cl->push_back(cc);
                    }

                    int el=lm[cb->entry_label->num];
                    auto*exits=new vector<Label*>();
                    for(auto*s:*cl)if(s){
                        if(s->kind==quad::QuadKind::JUMP)exits->push_back(new Label(static_cast<quad::QuadJump*>(s)->label->num));
                        else if(s->kind==quad::QuadKind::CJUMP){auto*cj=static_cast<quad::QuadCJump*>(s);if(cj->t)exits->push_back(new Label(cj->t->num));if(cj->f)exits->push_back(new Label(cj->f->num));}
                    }
                    clonedBlocks.push_back(new quad::QuadBlock(cl,new Label(el),exits));
                }

                // Build result blocks
                // Prefix block (may be empty)
                if(!prefix->empty()){
                    prefix->push_back(new quad::QuadJump(new Label(calleeEntryLabel),new set<Temp*>(),new set<Temp*>()));
                    auto*pe=new vector<Label*>();pe->push_back(new Label(calleeEntryLabel));
                    if(block->entry_label)newBlocks->push_back(new quad::QuadBlock(prefix,new Label(block->entry_label->num),pe));
                }else{
                    // No prefix - first block is callee entry, so update entry
                    // We need a block for this. Create empty prefix block.
                    auto*ep=new vector<quad::QuadStm*>();
                    ep->push_back(new quad::QuadJump(new Label(calleeEntryLabel),new set<Temp*>(),new set<Temp*>()));
                    auto*pe2=new vector<Label*>();pe2->push_back(new Label(calleeEntryLabel));
                    newBlocks->push_back(new quad::QuadBlock(ep,new Label(block->entry_label->num),pe2));
                }

                // Cloned callee blocks
                for(auto*cb2:clonedBlocks)newBlocks->push_back(cb2);

                // Continuation block
                if(!suffix->empty()){
                    auto*se=new vector<Label*>();
                    // Inherit original block's exit labels
                    if(block->exit_labels)for(auto*l:*block->exit_labels)if(l)se->push_back(new Label(l->num));
                    newBlocks->push_back(new quad::QuadBlock(suffix,new Label(contLabel),se));
                }else{
                    auto*se2=new vector<Label*>();
                    if(block->exit_labels)for(auto*l:*block->exit_labels)if(l)se2->push_back(new Label(l->num));
                    newBlocks->push_back(new quad::QuadBlock(suffix,new Label(contLabel),se2));
                }

                inlined++;modified=true;
                goto next_block; // break out of stmt loop
            }

            // No call site found, keep block as-is
            newBlocks->push_back(block);
            next_block:;
        }

        if(modified){caller->quadblocklist=newBlocks;caller->last_label_num=gl;caller->last_temp_num=gt;}
        newFuncs->push_back(caller);
    }

    prog->quadFuncDeclList=newFuncs;
    prog->last_label_num=gl;
    prog->last_temp_num=gt;
}
} // namespace

namespace quad {
QuadProgram* inlineProg(QuadProgram* p, int* eo) { if(!p)return p; int e=0; inlineProgImpl(p,e); if(eo)*eo=e; return p; }
} // namespace quad
