#define DEBUG
#undef DEBUG

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <queue>
#include <algorithm>
#include <utility>
#include "quad.hh"
#include "flowinfo.hh"
#include "quadssa.hh"
#include "temp.hh"

using namespace std;
using namespace quad;

// Forward declarations for internal functions
static void placePhi(QuadFuncDecl* func, ControlFlowInfo* domInfo, DataFlowInfo* liveness);
static void renameVariables(QuadFuncDecl* func, ControlFlowInfo* domInfo);
static void cleanupUnusedPhi(QuadFuncDecl* func);

// ========== Helper: Get type of a defined variable from a statement ==========
static QuadType getDefType(QuadStm* stm) {
    switch (stm->kind) {
        case QuadKind::MOVE: return static_cast<QuadMove*>(stm)->dst->type;
        case QuadKind::LOAD: return static_cast<QuadLoad*>(stm)->dst->type;
        case QuadKind::MOVE_BINOP: return static_cast<QuadMoveBinop*>(stm)->dst->type;
        case QuadKind::MOVE_CALL: return static_cast<QuadMoveCall*>(stm)->dst->type;
        case QuadKind::MOVE_EXTCALL: return static_cast<QuadMoveExtCall*>(stm)->dst->type;
        case QuadKind::PTR_CALC: return static_cast<QuadPtrCalc*>(stm)->dst->get_temp()->type;
        default: return QuadType::INT;
    }
}

// ========== placePhi: Insert PHI functions at dominance frontier join points ==========
static void placePhi(QuadFuncDecl* func, ControlFlowInfo* domInfo, DataFlowInfo* liveness) {
#ifdef DEBUG
    cout << "Placing phi functions for function: " << func->funcname << endl;
#endif
    // Step 0: Compute definition locations and types from statements
    // (The XML parser does NOT populate defs/uses, only liveness & allVars)
    map<int, set<int>> defBlocksMap; // var -> set of block labels that define it
    map<int, QuadType> varTypes;     // var -> its QuadType
    for (auto* block : *func->quadblocklist) {
        int blabel = block->entry_label->num;
        for (auto* stm : *block->quadlist) {
            if (!stm->def) continue;
            for (auto* t : *(stm->def)) {
                int v = t->num;
                defBlocksMap[v].insert(blabel);
                if (!varTypes.count(v)) varTypes[v] = getDefType(stm);
            }
        }
    }

    // Step 1: For each variable in allVars with definitions, compute iterated DF
    for (int v : liveness->allVars) {
        if (!defBlocksMap.count(v)) continue; // Parameter or unused
        auto defBlocks = defBlocksMap[v];

        // Iterated dominance frontier algorithm (worklist)
        set<int> phiBlocks;   // Blocks where PHI for v will be placed
        set<int> processed;   // Blocks already added to worklist
        queue<int> worklist;
        for (int b : defBlocks) {
            worklist.push(b);
            processed.insert(b);
        }

        while (!worklist.empty()) {
            int X = worklist.front(); worklist.pop();
            if (!domInfo->dominanceFrontiers.count(X)) continue;
            for (int Y : domInfo->dominanceFrontiers[X]) {
                if (phiBlocks.count(Y)) continue;
                // Pruned SSA: only place PHI if v is live-in at block Y
                auto* yBlock = domInfo->labelToBlock[Y];
                auto* labelStm = yBlock->quadlist->front();
                if (liveness->livein->count(labelStm) && (*liveness->livein)[labelStm].count(v)) {
                    phiBlocks.insert(Y);
                    if (!processed.count(Y)) {
                        processed.insert(Y);
                        worklist.push(Y);
                    }
                }
            }
        }

        // Step 3: Insert PHI nodes at the computed blocks
        auto type = varTypes.count(v) ? varTypes[v] : QuadType::INT;
        for (int blockLabel : phiBlocks) {
            auto* block = domInfo->labelToBlock[blockLabel];

            // Create PHI args: one per predecessor, sorted by label number
            auto* args = new vector<pair<Temp*, Label*>>();
            for (int pred : domInfo->predecessors[blockLabel])
                args->push_back( {new Temp(v), new Label(pred)} );

            // Create destination temp and def/use sets
            auto* destTemp = new Temp(v);
            auto* defSet = new set<Temp*>();
            defSet->insert(destTemp);
            auto* useSet = new set<Temp*>();
            for (auto& [argTemp, argLabel] : *args) useSet->insert(argTemp);

            auto* phi = new QuadPhi(
                new QuadTemp(destTemp, type), args, defSet, useSet
            );

            // Insert PHI right after the LABEL statement
            auto it = block->quadlist->begin();
            if (it != block->quadlist->end() && (*it)->kind == QuadKind::LABEL) ++it;
            block->quadlist->insert(it, phi);
        }
    }
}

// ========== Rename helpers ==========

// Create a new versioned Temp for a USE (read current version from stack)
static Temp* versionUse(Temp* temp, set<int>& paramSet, set<int>& vars,
                        map<int, stack<int>>& stacks) {
    if (!temp) return temp;
    int num = temp->num;
    if (paramSet.count(num) || !vars.count(num)) return temp;
    if (stacks[num].empty()) return temp;
    return new Temp(VersionedTemp::versionedTempNum(num, stacks[num].top()));
}

// Create a new versioned Temp for a DEF (assign next version, push onto stack)
static Temp* versionDef(Temp* temp, set<int>& paramSet, set<int>& vars,
                        map<int, int>& count, map<int, stack<int>>& stacks,
                        map<int, int>& pushCnt) {
    if (!temp) return temp;
    int num = temp->num;
    if (paramSet.count(num) || !vars.count(num)) return temp;
    int ver = count[num]++;
    stacks[num].push(ver);
    pushCnt[num]++;
    return new Temp(VersionedTemp::versionedTempNum(num, ver));
}

// Rename a QuadTerm's temp if it's a TEMP (for uses)
static void renameTermUse(QuadTerm* term, set<int>& paramSet, set<int>& vars,
                          map<int, stack<int>>& stacks) {
    if (!term || term->kind != QuadTermKind::TEMP) return;
    term->get_temp()->temp = versionUse(term->get_temp()->temp, paramSet, vars, stacks);
}

// Rename all uses in a non-PHI statement
static void renameUses(QuadStm* stm, set<int>& paramSet, set<int>& vars,
                       map<int, stack<int>>& stacks) {
    switch (stm->kind) {
        case QuadKind::MOVE: {
            auto* s = static_cast<QuadMove*>(stm);
            renameTermUse(s->src, paramSet, vars, stacks);
            break;
        }
        case QuadKind::LOAD: {
            auto* s = static_cast<QuadLoad*>(stm);
            renameTermUse(s->src, paramSet, vars, stacks);
            break;
        }
        case QuadKind::STORE: {
            auto* s = static_cast<QuadStore*>(stm);
            renameTermUse(s->src, paramSet, vars, stacks);
            renameTermUse(s->dst, paramSet, vars, stacks);
            break;
        }
        case QuadKind::MOVE_BINOP: {
            auto* s = static_cast<QuadMoveBinop*>(stm);
            renameTermUse(s->left, paramSet, vars, stacks);
            renameTermUse(s->right, paramSet, vars, stacks);
            break;
        }
        case QuadKind::CALL: {
            auto* s = static_cast<QuadCall*>(stm);
            renameTermUse(s->obj_term, paramSet, vars, stacks);
            if (s->args) for (auto* a : *s->args) renameTermUse(a, paramSet, vars, stacks);
            break;
        }
        case QuadKind::MOVE_CALL: {
            auto* s = static_cast<QuadMoveCall*>(stm);
            if (s->call) {
                renameTermUse(s->call->obj_term, paramSet, vars, stacks);
                if (s->call->args) for (auto* a : *s->call->args) renameTermUse(a, paramSet, vars, stacks);
            }
            break;
        }
        case QuadKind::EXTCALL: {
            auto* s = static_cast<QuadExtCall*>(stm);
            if (s->args) for (auto* a : *s->args) renameTermUse(a, paramSet, vars, stacks);
            break;
        }
        case QuadKind::MOVE_EXTCALL: {
            auto* s = static_cast<QuadMoveExtCall*>(stm);
            if (s->extcall && s->extcall->args)
                for (auto* a : *s->extcall->args) renameTermUse(a, paramSet, vars, stacks);
            break;
        }
        case QuadKind::CJUMP: {
            auto* s = static_cast<QuadCJump*>(stm);
            renameTermUse(s->left, paramSet, vars, stacks);
            renameTermUse(s->right, paramSet, vars, stacks);
            break;
        }
        case QuadKind::RETURN: {
            auto* s = static_cast<QuadReturn*>(stm);
            renameTermUse(s->exp, paramSet, vars, stacks);
            break;
        }
        case QuadKind::PTR_CALC: {
            auto* s = static_cast<QuadPtrCalc*>(stm);
            renameTermUse(s->ptr, paramSet, vars, stacks);
            renameTermUse(s->offset, paramSet, vars, stacks);
            break;
        }
        default: break; // LABEL, JUMP, PHI - no uses to rename here
    }
}

// Rename all defs in a statement (including PHI destinations)
static void renameDefs(QuadStm* stm, set<int>& paramSet, set<int>& vars,
                       map<int, int>& count, map<int, stack<int>>& stacks,
                       map<int, int>& pushCnt) {
    switch (stm->kind) {
        case QuadKind::MOVE: {
            auto* s = static_cast<QuadMove*>(stm);
            s->dst->temp = versionDef(s->dst->temp, paramSet, vars, count, stacks, pushCnt);
            break;
        }
        case QuadKind::LOAD: {
            auto* s = static_cast<QuadLoad*>(stm);
            s->dst->temp = versionDef(s->dst->temp, paramSet, vars, count, stacks, pushCnt);
            break;
        }
        case QuadKind::MOVE_BINOP: {
            auto* s = static_cast<QuadMoveBinop*>(stm);
            s->dst->temp = versionDef(s->dst->temp, paramSet, vars, count, stacks, pushCnt);
            break;
        }
        case QuadKind::MOVE_CALL: {
            auto* s = static_cast<QuadMoveCall*>(stm);
            s->dst->temp = versionDef(s->dst->temp, paramSet, vars, count, stacks, pushCnt);
            break;
        }
        case QuadKind::MOVE_EXTCALL: {
            auto* s = static_cast<QuadMoveExtCall*>(stm);
            s->dst->temp = versionDef(s->dst->temp, paramSet, vars, count, stacks, pushCnt);
            break;
        }
        case QuadKind::PHI: {
            auto* s = static_cast<QuadPhi*>(stm);
            s->temp_exp->temp = versionDef(s->temp_exp->temp, paramSet, vars, count, stacks, pushCnt);
            break;
        }
        case QuadKind::PTR_CALC: {
            auto* s = static_cast<QuadPtrCalc*>(stm);
            if (s->dst && s->dst->kind == QuadTermKind::TEMP)
                s->dst->get_temp()->temp = versionDef(s->dst->get_temp()->temp, paramSet, vars, count, stacks, pushCnt);
            break;
        }
        default: break;
    }
}

// ========== Helper: add temp number from a QuadTerm to a set ==========
static void addTermNum(QuadTerm* term, set<int>& nums) {
    if (term && term->kind == QuadTermKind::TEMP) nums.insert(term->get_temp()->temp->num);
}

// Collect def and use temp numbers from a statement's structure
static void collectDefUseNums(QuadStm* stm, set<int>& defN, set<int>& useN) {
    switch (stm->kind) {
        case QuadKind::MOVE: {
            auto* s = static_cast<QuadMove*>(stm);
            defN.insert(s->dst->temp->num);
            addTermNum(s->src, useN);
            break;
        }
        case QuadKind::LOAD: {
            auto* s = static_cast<QuadLoad*>(stm);
            defN.insert(s->dst->temp->num);
            addTermNum(s->src, useN);
            break;
        }
        case QuadKind::STORE: {
            auto* s = static_cast<QuadStore*>(stm);
            addTermNum(s->src, useN);
            addTermNum(s->dst, useN);
            break;
        }
        case QuadKind::MOVE_BINOP: {
            auto* s = static_cast<QuadMoveBinop*>(stm);
            defN.insert(s->dst->temp->num);
            addTermNum(s->left, useN);
            addTermNum(s->right, useN);
            break;
        }
        case QuadKind::CALL: {
            auto* s = static_cast<QuadCall*>(stm);
            addTermNum(s->obj_term, useN);
            if (s->args) for (auto* a : *s->args) addTermNum(a, useN);
            break;
        }
        case QuadKind::MOVE_CALL: {
            auto* s = static_cast<QuadMoveCall*>(stm);
            defN.insert(s->dst->temp->num);
            if (s->call) {
                addTermNum(s->call->obj_term, useN);
                if (s->call->args) for (auto* a : *s->call->args) addTermNum(a, useN);
            }
            break;
        }
        case QuadKind::EXTCALL: {
            auto* s = static_cast<QuadExtCall*>(stm);
            if (s->args) for (auto* a : *s->args) addTermNum(a, useN);
            break;
        }
        case QuadKind::MOVE_EXTCALL: {
            auto* s = static_cast<QuadMoveExtCall*>(stm);
            defN.insert(s->dst->temp->num);
            if (s->extcall && s->extcall->args)
                for (auto* a : *s->extcall->args) addTermNum(a, useN);
            break;
        }
        case QuadKind::CJUMP: {
            auto* s = static_cast<QuadCJump*>(stm);
            addTermNum(s->left, useN);
            addTermNum(s->right, useN);
            break;
        }
        case QuadKind::RETURN: {
            auto* s = static_cast<QuadReturn*>(stm);
            addTermNum(s->exp, useN);
            break;
        }
        case QuadKind::PHI: {
            auto* s = static_cast<QuadPhi*>(stm);
            defN.insert(s->temp_exp->temp->num);
            for (auto& [temp, label] : *s->args) useN.insert(temp->num);
            break;
        }
        case QuadKind::PTR_CALC: {
            auto* s = static_cast<QuadPtrCalc*>(stm);
            if (s->dst && s->dst->kind == QuadTermKind::TEMP)
                defN.insert(s->dst->get_temp()->temp->num);
            addTermNum(s->ptr, useN);
            addTermNum(s->offset, useN);
            break;
        }
        default: break;
    }
}

// Rebuild def/use sets for all statements
static void rebuildDefUseSets(QuadFuncDecl* func) {
    for (auto* block : *func->quadblocklist) {
        for (auto* stm : *block->quadlist) {
            set<int> defN, useN;
            collectDefUseNums(stm, defN, useN);
            // Allocate Temps in sorted number order
            auto* newDef = new set<Temp*>();
            for (int n : defN) newDef->insert(new Temp(n));
            auto* newUse = new set<Temp*>();
            for (int n : useN) newUse->insert(new Temp(n));
            stm->def = newDef;
            stm->use = newUse;
        }
    }
}

// Recursive dominator-tree walk for renaming
static void renameBlock(int blockLabel, ControlFlowInfo* domInfo,
                        set<int>& paramSet, set<int>& vars,
                        map<int, int>& count, map<int, stack<int>>& stacks) {
    auto* block = domInfo->labelToBlock[blockLabel];
    map<int, int> pushCnt; // Track pushes per variable for rollback

    // Process each statement in the block
    for (auto* stm : *block->quadlist) {
        if (stm->kind != QuadKind::PHI) renameUses(stm, paramSet, vars, stacks);
        renameDefs(stm, paramSet, vars, count, stacks, pushCnt);
    }

    // Update PHI args in CFG successor blocks
    if (domInfo->successors.count(blockLabel)) {
        for (int succ : domInfo->successors[blockLabel]) {
            auto* sb = domInfo->labelToBlock[succ];
            for (auto* stm : *sb->quadlist) {
                if (stm->kind == QuadKind::PHI) {
                    auto* phi = static_cast<QuadPhi*>(stm);
                    for (auto& arg : *phi->args) {
                        if (arg.second->num != blockLabel) continue;
                        int origNum = arg.first->num;
                        if (vars.count(origNum) && !stacks[origNum].empty())
                            arg.first = new Temp(VersionedTemp::versionedTempNum(origNum, stacks[origNum].top()));
                    }
                } else if (stm->kind != QuadKind::LABEL) break;
            }
        }
    }

    // Recurse into dominator tree children
    if (domInfo->domTree.count(blockLabel)) {
        for (int child : domInfo->domTree[blockLabel])
            renameBlock(child, domInfo, paramSet, vars, count, stacks);
    }

    // Pop stacks back to previous state
    for (auto& [v, cnt] : pushCnt)
        for (int i = 0; i < cnt; i++) stacks[v].pop();
}

// ========== renameVariables ==========
static void renameVariables(QuadFuncDecl* func, ControlFlowInfo* domInfo) {
#ifdef DEBUG
    cout << "Entering renaming variables for function: " << func->funcname << endl;
#endif
    // Collect parameter temp numbers (these are NOT versioned)
    set<int> paramSet;
    if (func->params) for (auto* p : *func->params) paramSet.insert(p->num);

    // Collect all non-parameter variables that have definitions
    set<int> vars;
    for (auto* block : *func->quadblocklist) {
        for (auto* stm : *block->quadlist) {
            if (!stm->def) continue;
            for (auto* t : *(stm->def))
                if (!paramSet.count(t->num)) vars.insert(t->num);
        }
    }

    // Initialize version counters and stacks
    map<int, int> count;
    map<int, stack<int>> stacks;
    for (int v : vars) count[v] = 0;

    // Walk dominator tree starting from the entry block
    if (domInfo->entryBlock != -1)
        renameBlock(domInfo->entryBlock, domInfo, paramSet, vars, count, stacks);

    // Rebuild all def/use sets to reflect versioned temp numbers
    rebuildDefUseSets(func);
}

// ========== cleanupUnusedPhi ==========
static void cleanupUnusedPhi(QuadFuncDecl* func) {
#ifdef DEBUG
    cout << "Cleaning up unused phi functions for function: " << func->funcname << endl;
#endif
    bool changed = true;
    while (changed) {
        changed = false;
        // Collect all used temp numbers (exclude PHI self-references)
        set<int> usedTemps;
        for (auto* block : *func->quadblocklist) {
            for (auto* stm : *block->quadlist) {
                if (stm->kind == QuadKind::PHI) {
                    auto* phi = static_cast<QuadPhi*>(stm);
                    int defNum = phi->temp_exp->temp->num;
                    if (stm->use) for (auto* t : *(stm->use))
                        if (t->num != defNum) usedTemps.insert(t->num);
                } else {
                    if (stm->use) for (auto* t : *(stm->use)) usedTemps.insert(t->num);
                }
            }
        }
        // Remove PHI nodes whose defined temp is never used
        for (auto* block : *func->quadblocklist) {
            auto it = block->quadlist->begin();
            while (it != block->quadlist->end()) {
                if ((*it)->kind == QuadKind::PHI) {
                    auto* phi = static_cast<QuadPhi*>(*it);
                    if (!usedTemps.count(phi->temp_exp->temp->num)) {
                        it = block->quadlist->erase(it);
                        changed = true;
                        continue;
                    }
                }
                ++it;
            }
        }
    }
}

// Convert blocked Quad with precomputed flow info to SSA form
quad::QuadProgram *quad2ssa(set<FuncFlowInfo*>* allFuncFlow) {
    if (!allFuncFlow || allFuncFlow->empty()) {
        return nullptr; // Invalid program
    }

    // Sort functions by QuadFuncDecl* address to preserve original declaration order
    vector<FuncFlowInfo*> sorted(allFuncFlow->begin(), allFuncFlow->end());
    sort(sorted.begin(), sorted.end(), [](auto* a, auto* b) {
        return a->cfi->func < b->cfi->func;
    });

    auto* funcs = new vector<QuadFuncDecl*>();
    funcs->reserve(sorted.size());
    int prog_last_label_num = -1;
    int prog_last_temp_num = -1;

    for (auto* ffi : sorted) {
        if (!ffi || !ffi->cfi || !ffi->cfi->func) continue;
        auto* funcdecl = ffi->cfi->func;

        auto* domInfo = ffi->cfi;
        auto* liveness = ffi->dfi;

        // Place PHI functions at dominance frontier join points
        placePhi(funcdecl, domInfo, liveness);
        // Rename variables to ensure SSA property
        renameVariables(funcdecl, domInfo);
        // Clean up unnecessary PHI nodes
        cleanupUnusedPhi(funcdecl);

        funcs->push_back(funcdecl);
        if (prog_last_label_num < funcdecl->last_label_num) prog_last_label_num = funcdecl->last_label_num;
        if (prog_last_temp_num < funcdecl->last_temp_num) prog_last_temp_num = funcdecl->last_temp_num;
    }
    return new QuadProgram(funcs, prog_last_label_num, prog_last_temp_num);
}
