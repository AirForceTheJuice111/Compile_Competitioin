#define DEBUG
#undef DEBUG

#include <iostream>
#include <queue>
#include <algorithm>
#include <map>
#include "quad.hh"
#include "flowinfo.hh"

using namespace std;
using namespace quad;
// Find all variables used or defined in the function
void DataFlowInfo::findAllVars() {
#ifdef DEBUG
    cout << "Finding all variables in function: " << func->funcname << endl;
#endif
    allVars.clear();
    defs->clear();
    uses->clear();
    // Include function parameters as variables
    if (func->params) {
        for (auto t : *func->params) allVars.insert(t->num);
    }
    // Collect all variables from def/use sets of each statement
    for (auto block : *func->quadblocklist) {
        if (!block || !block->quadlist) continue;
        for (auto stmt : *block->quadlist) {
            if (!stmt) continue;
            if (stmt->def) {
                for (auto t : *stmt->def) {
                    allVars.insert(t->num);
                    (*defs)[t->num].insert({block, stmt});
                }
            }
            if (stmt->use) {
                for (auto t : *stmt->use) {
                    allVars.insert(t->num);
                    (*uses)[t->num].insert({block, stmt});
                }
            }
        }
    }
#ifdef DEBUG
    cout << "All variables: ";
    for (auto v : allVars) cout << "t" << v << " ";
    cout << endl;
#endif
}

// Calculate both live-in and live-out sets for all statements
void DataFlowInfo::computeLiveness() {
#ifdef DEBUG
    cout << "Computing liveness for function: " << func->funcname << endl;
#endif
    livein->clear();
    liveout->clear();

    if (func == nullptr || func->quadblocklist == nullptr) {
        return;
    }

    map<int, QuadBlock*> labelToBlock;
    map<QuadBlock*, set<int>> blockUse;
    map<QuadBlock*, set<int>> blockDef;
    map<QuadBlock*, set<int>> blockLiveIn;
    map<QuadBlock*, set<int>> blockLiveOut;

    for (auto block : *func->quadblocklist) {
        if (!block) continue;
        if (block->entry_label) labelToBlock[block->entry_label->num] = block;

        set<int> usesBeforeDef;
        set<int> defsInBlock;
        if (block->quadlist) {
            for (auto stmt : *block->quadlist) {
                if (!stmt) continue;
                (*livein)[stmt] = set<int>();
                (*liveout)[stmt] = set<int>();
                if (stmt->use) {
                    for (auto t : *stmt->use) {
                        if (t != nullptr && defsInBlock.count(t->num) == 0) {
                            usesBeforeDef.insert(t->num);
                        }
                    }
                }
                if (stmt->def) {
                    for (auto t : *stmt->def) {
                        if (t != nullptr) {
                            defsInBlock.insert(t->num);
                        }
                    }
                }
            }
        }
        blockUse[block] = usesBeforeDef;
        blockDef[block] = defsInBlock;
        blockLiveIn[block] = set<int>();
        blockLiveOut[block] = set<int>();
    }

    // First solve the standard block-level liveness equations, then expand the
    // result inside each basic block. This avoids repeatedly copying statement
    // live sets for long straight-line blocks.
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = (int)func->quadblocklist->size() - 1; i >= 0; i--) {
            auto block = func->quadblocklist->at(i);
            if (!block) continue;

            set<int> new_liveout;
            if (block->exit_labels) {
                for (auto label : *block->exit_labels) {
                    if (label == nullptr) continue;
                    auto it = labelToBlock.find(label->num);
                    if (it != labelToBlock.end()) {
                        for (auto v : blockLiveIn[it->second]) new_liveout.insert(v);
                    }
                }
            }

            set<int> new_livein = blockUse[block];
            for (auto v : new_liveout) {
                if (blockDef[block].count(v) == 0) {
                    new_livein.insert(v);
                }
            }

            if (new_liveout != blockLiveOut[block]) {
                blockLiveOut[block] = new_liveout;
                changed = true;
            }
            if (new_livein != blockLiveIn[block]) {
                blockLiveIn[block] = new_livein;
                changed = true;
            }
        }
    }

    for (auto block : *func->quadblocklist) {
        if (!block || !block->quadlist) continue;
        set<int> live = blockLiveOut[block];
        auto &stmts = *block->quadlist;
        for (int i = (int)stmts.size() - 1; i >= 0; --i) {
            auto stmt = stmts[i];
            if (stmt == nullptr) continue;

            (*liveout)[stmt] = live;

            set<int> in = live;
            if (stmt->def) {
                for (auto t : *stmt->def) {
                    if (t != nullptr) in.erase(t->num);
                }
            }
            if (stmt->use) {
                for (auto t : *stmt->use) {
                    if (t != nullptr) in.insert(t->num);
                }
            }
            (*livein)[stmt] = in;
            live = in;
        }
    }
}

set<DataFlowInfo*>* dataFLowProg(QuadProgram* prog) {
    //THIS ONE IS DONE FOR YOU!
    // For each function in the program, compute its data flow information and 
    // return a set of DataFlowInfo for all functions
    if (!prog || !prog->quadFuncDeclList) return nullptr;
    set<DataFlowInfo*>* allDataFlows = new set<DataFlowInfo*>();
    for (auto func : *prog->quadFuncDeclList) {
        if (!func || !func->quadblocklist) continue;
        
        DataFlowInfo* dfInfo = new DataFlowInfo(func);
        dfInfo->findAllVars();
        dfInfo->computeLiveness();
#ifdef DEBUG
        cout << "Liveness information for function: " << func->funcname << endl;
        cout << dfInfo->printLiveness() << endl;
#endif
        allDataFlows->insert(dfInfo);
    }
    return allDataFlows;
}
