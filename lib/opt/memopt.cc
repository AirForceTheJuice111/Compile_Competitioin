#include "memopt.hh"

#include <map>
#include <set>
#include <string>
#include <vector>

#include "quad.hh"
#include "temp.hh"

using namespace std;

namespace {

set<Temp*>* es() { return new set<Temp*>(); }
set<Temp*>* os(int n) { auto* s = new set<Temp*>(); s->insert(new Temp(n)); return s; }

// Memory address key: PTR_CALC result temp number.
// If a STORE and a LOAD use the same PTR_CALC temp as address,
// they access the same location (within a block, no aliasing).
using MemKey = int;  // PTR_CALC result temp number
const MemKey INVALID_KEY = -1;

MemKey extractKey(quad::QuadTerm* addr) {
    if (!addr || addr->kind != quad::QuadTermKind::TEMP) return INVALID_KEY;
    return addr->get_temp()->temp->num;
}

// Check if a statement is a call that might have side effects.
bool isSideEffectCall(quad::QuadStm* stm) {
    if (!stm) return false;
    switch (stm->kind) {
    case quad::QuadKind::CALL:
    case quad::QuadKind::EXTCALL:
    case quad::QuadKind::MOVE_CALL:
    case quad::QuadKind::MOVE_EXTCALL:
        return true;
    default:
        return false;
    }
}

void memOptFunction(quad::QuadFuncDecl* func, int& eliminated) {
    if (!func || !func->quadblocklist) return;

    // Optimize each block independently
    for (auto* block : *func->quadblocklist) {
        if (!block || !block->quadlist) continue;

// Track last store to each address (PTR_CALC temp)
    map<MemKey, int> lastStoreVal;   // MemKey -> stored value temp (-1 if const)
    map<MemKey, int> lastStorePos;   // MemKey -> position in newList
    set<int> deadPositions;

        auto* newList = new vector<quad::QuadStm*>();

        for (auto* stm : *block->quadlist) {
            if (!stm) continue;

            // Calls invalidate all memory state
            if (isSideEffectCall(stm)) {
                lastStoreVal.clear();
                lastStorePos.clear();
                newList->push_back(stm);
                continue;
            }

            // STORE: mem[addr] = value
            if (stm->kind == quad::QuadKind::STORE) {
                auto* store = static_cast<quad::QuadStore*>(stm);
                MemKey key = extractKey(store->dst);
                if (key == INVALID_KEY) {
                    newList->push_back(stm);
                    // Invalidate stores to unknown addresses
                    lastStoreVal.clear();
                    lastStorePos.clear();
                    continue;
                }

                // Get stored value temp
                int valTemp = -1;
                if (store->src && store->src->kind == quad::QuadTermKind::TEMP)
                    valTemp = store->src->get_temp()->temp->num;

                // If there's a previous store to same address, mark it dead
                auto it = lastStorePos.find(key);
                if (it != lastStorePos.end()) {
                    deadPositions.insert(it->second);
                    eliminated++;
                }

                // Record this store
                lastStoreVal[key] = valTemp;
                lastStorePos[key] = (int)newList->size();
                newList->push_back(store);
                continue;
            }

            // LOAD: temp = mem[addr]
            if (stm->kind == quad::QuadKind::LOAD) {
                auto* load = static_cast<quad::QuadLoad*>(stm);
                MemKey key = extractKey(load->src);
                if (key == INVALID_KEY) {
                    newList->push_back(stm);
                    continue;
                }

                // Check if we have a stored value for this address
                auto it = lastStoreVal.find(key);
                if (it != lastStoreVal.end() && it->second >= 0) {
                    // Forward: replace LOAD with MOVE from stored value
                    int dstNum = load->dst->temp->num;
                    int srcNum = it->second;
                    auto* src = new quad::QuadTerm(
                        new quad::QuadTemp(new Temp(srcNum),
                                           load->dst->type));
                    auto* mv = new quad::QuadMove(
                        load->dst->clone(), src, os(dstNum), os(srcNum));
                    newList->push_back(mv);
                    eliminated++;
                    continue;
                }

                newList->push_back(stm);
                continue;
            }

            newList->push_back(stm);
        }

        // Filter out dead stores
        if (!deadPositions.empty()) {
            auto* filtered = new vector<quad::QuadStm*>();
            for (int i = 0; i < (int)newList->size(); i++)
                if (!deadPositions.count(i))
                    filtered->push_back((*newList)[i]);
            newList = filtered;
        }

        block->quadlist = newList;
    }
}

} // namespace

namespace quad {
QuadProgram* memOptProg(QuadProgram* prog, int* eliminatedOut) {
    if (!prog) return prog;
    int eliminated = 0;
    for (auto* fd : *prog->quadFuncDeclList)
        if (fd) memOptFunction(fd, eliminated);
    if (eliminatedOut) *eliminatedOut = eliminated;
    return prog;
}
} // namespace quad
