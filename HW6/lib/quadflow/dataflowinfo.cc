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

    // Build label -> block map for successor lookup
    map<int, QuadBlock*> labelToBlock;
    for (auto block : *func->quadblocklist) {
        if (block && block->entry_label) labelToBlock[block->entry_label->num] = block;
    }

    // Initialize live-in and live-out to empty for all statements
    for (auto block : *func->quadblocklist) {
        if (!block || !block->quadlist) continue;
        for (auto stmt : *block->quadlist) {
            (*livein)[stmt] = set<int>();
            (*liveout)[stmt] = set<int>();
        }
    }

    // Iterative backward dataflow analysis until convergence
    bool changed = true;
    while (changed) {
        changed = false;
        // Process blocks in reverse order for faster convergence
        for (int i = (int)func->quadblocklist->size() - 1; i >= 0; i--) {
            auto block = func->quadblocklist->at(i);
            if (!block || !block->quadlist || block->quadlist->empty()) continue;

            auto &stmts = *block->quadlist;

            // live_out of last statement = union of live_in of first stmts of successor blocks (successor means immediate successors)
            auto lastStmt = stmts.back();
            set<int> new_liveout;
            if (block->exit_labels) {
                for (auto label : *block->exit_labels) {
                    auto it = labelToBlock.find(label->num);
                    if (it != labelToBlock.end() && it->second->quadlist && !it->second->quadlist->empty()) {
                        auto firstStmt = it->second->quadlist->front();
                        for (auto v : (*livein)[firstStmt]) new_liveout.insert(v);
                    }
                }
            }
            if (new_liveout != (*liveout)[lastStmt]) {
                (*liveout)[lastStmt] = new_liveout;
                changed = true;
            }

            // Process statements from last to first
            for (int j = (int)stmts.size() - 1; j >= 0; j--) {
                auto stmt = stmts[j];
                // live_in = use ∪ (live_out - def)
                set<int> new_livein = (*liveout)[stmt];
                if (stmt->def) {
                    for (auto t : *stmt->def) new_livein.erase(t->num);
                }
                if (stmt->use) {
                    for (auto t : *stmt->use) new_livein.insert(t->num);
                }
                if (new_livein != (*livein)[stmt]) {
                    (*livein)[stmt] = new_livein;
                    changed = true;
                }
                // live_out of previous statement = live_in of current statement
                if (j > 0) {
                    if ((*livein)[stmt] != (*liveout)[stmts[j - 1]]) {
                        (*liveout)[stmts[j - 1]] = (*livein)[stmt];
                        changed = true;
                    }
                }
            }
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
