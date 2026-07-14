#include "memopt.hh"

#include <map>
#include <set>
#include <string>
#include <vector>

#include "flowinfo.hh"
#include "quad.hh"
#include "quad_metadata.hh"
#include "temp.hh"

using namespace std;

namespace {

using MemoryLocation = int;
constexpr MemoryLocation kUnknownLocation = -1;

struct MemoryCell {
    int version = -1;
    quad::QuadTerm *storedValue = nullptr;
};
using MemoryState = map<MemoryLocation, MemoryCell>;

set<Temp *> *defs(int number) {
    auto *result = new set<Temp *>();
    result->insert(new Temp(number));
    return result;
}

set<Temp *> *uses(quad::QuadTerm *term) {
    auto *result = new set<Temp *>();
    if (term != nullptr && term->kind == quad::QuadTermKind::TEMP &&
        term->get_temp() != nullptr && term->get_temp()->temp != nullptr) {
        result->insert(new Temp(term->get_temp()->temp->num));
    }
    return result;
}

MemoryLocation locationOf(quad::QuadTerm *term) {
    if (term == nullptr || term->kind != quad::QuadTermKind::TEMP ||
        term->get_temp() == nullptr || term->get_temp()->temp == nullptr ||
        term->get_temp()->type != quad::QuadType::PTR) {
        return kUnknownLocation;
    }
    return term->get_temp()->temp->num;
}

bool isUnknownMemoryEffect(quad::QuadStm *statement) {
    if (statement == nullptr) return false;
    switch (statement->kind) {
        case quad::QuadKind::CALL:
        case quad::QuadKind::EXTCALL:
        case quad::QuadKind::MOVE_CALL:
        case quad::QuadKind::MOVE_EXTCALL:
            return true;
        default:
            return false;
    }
}

bool sameState(const MemoryState &left, const MemoryState &right) {
    if (left.size() != right.size()) return false;
    for (const auto &[location, cell] : left) {
        auto it = right.find(location);
        if (it == right.end() || it->second.version != cell.version) return false;
    }
    return true;
}

MemoryState mergePredecessors(int label, const ControlFlowInfo *flow,
                              const map<int, MemoryState> &outStates) {
    MemoryState merged;
    auto predIt = flow->predecessors.find(label);
    if (predIt == flow->predecessors.end() || predIt->second.empty()) return merged;

    bool first = true;
    for (int predecessor : predIt->second) {
        auto stateIt = outStates.find(predecessor);
        if (stateIt == outStates.end()) return {};
        if (first) {
            merged = stateIt->second;
            first = false;
            continue;
        }
        for (auto it = merged.begin(); it != merged.end();) {
            auto other = stateIt->second.find(it->first);
            if (other == stateIt->second.end() ||
                other->second.version != it->second.version) {
                it = merged.erase(it);
            } else {
                ++it;
            }
        }
    }
    return merged;
}

MemoryState transfer(const quad::QuadBlock *block, MemoryState state,
                     int &nextVersion) {
    if (block == nullptr || block->quadlist == nullptr) return state;
    for (auto *statement : *block->quadlist) {
        if (statement == nullptr) continue;
        if (isUnknownMemoryEffect(statement)) {
            state.clear();
            continue;
        }
        if (statement->kind == quad::QuadKind::STORE) {
            auto *store = static_cast<quad::QuadStore *>(statement);
            MemoryLocation location = locationOf(store->dst);
            if (location == kUnknownLocation) {
                state.clear();
                continue;
            }
            state[location] = {nextVersion++, store->src};
        }
    }
    return state;
}

void collectMemoryStates(quad::QuadFuncDecl *function, ControlFlowInfo *flow,
                         map<int, MemoryState> &inStates,
                         map<int, MemoryState> &outStates) {
    if (function == nullptr || flow == nullptr) return;
    int nextVersion = 0;
    for (int iteration = 0; iteration < 2 * max(1, (int)flow->allBlocks.size());
         ++iteration) {
        bool changed = false;
        for (auto *block : *function->quadblocklist) {
            if (block == nullptr || block->entry_label == nullptr) continue;
            int label = block->entry_label->num;
            MemoryState incoming;
            if (label != flow->entryBlock) {
                incoming = mergePredecessors(label, flow, outStates);
            }
            MemoryState outgoing = transfer(block, incoming, nextVersion);
            if (!sameState(inStates[label], incoming) ||
                !sameState(outStates[label], outgoing)) {
                changed = true;
                inStates[label] = incoming;
                outStates[label] = outgoing;
            }
        }
        if (!changed) break;
    }
}

void rewriteLoads(quad::QuadFuncDecl *function, ControlFlowInfo *flow,
                  const map<int, MemoryState> &inStates, int &eliminated) {
    if (function == nullptr || flow == nullptr) return;
    for (auto *block : *function->quadblocklist) {
        if (block == nullptr || block->entry_label == nullptr ||
            block->quadlist == nullptr) continue;
        MemoryState state;
        int label = block->entry_label->num;
        auto it = inStates.find(label);
        if (it != inStates.end()) state = it->second;

        auto *rewritten = new vector<quad::QuadStm *>();
        for (auto *statement : *block->quadlist) {
            if (statement == nullptr) continue;
            if (isUnknownMemoryEffect(statement)) {
                state.clear();
                rewritten->push_back(statement);
                continue;
            }
            if (statement->kind == quad::QuadKind::STORE) {
                auto *store = static_cast<quad::QuadStore *>(statement);
                MemoryLocation location = locationOf(store->dst);
                if (location == kUnknownLocation) state.clear();
                else state[location] = {static_cast<int>(state.size()) + 1,
                                        store->src};
                rewritten->push_back(statement);
                continue;
            }
            if (statement->kind == quad::QuadKind::LOAD) {
                auto *load = static_cast<quad::QuadLoad *>(statement);
                MemoryLocation location = locationOf(load->src);
                auto value = state.find(location);
                if (location != kUnknownLocation && value != state.end() &&
                    value->second.storedValue != nullptr) {
                    auto *source = value->second.storedValue->clone();
                    rewritten->push_back(new quad::QuadMove(
                        load->dst->clone(), source, defs(load->dst->temp->num),
                        uses(source)));
                    ++eliminated;
                    continue;
                }
            }
            rewritten->push_back(statement);
        }
        block->quadlist = rewritten;
    }
}

} // namespace

namespace quad {

QuadProgram *memOptProg(QuadProgram *program, set<FuncFlowInfo *> *flows,
                        int *eliminatedOut) {
    if (eliminatedOut != nullptr) *eliminatedOut = 0;
    if (program == nullptr || flows == nullptr) return program;
    int eliminated = 0;
    for (auto *flow : *flows) {
        if (flow == nullptr || flow->cfi == nullptr || flow->cfi->func == nullptr)
            continue;
        map<int, MemoryState> inStates;
        map<int, MemoryState> outStates;
        collectMemoryStates(flow->cfi->func, flow->cfi, inStates, outStates);
        rewriteLoads(flow->cfi->func, flow->cfi, inStates, eliminated);
    }
    rebuildQuadProgramMetadata(program);
    if (eliminatedOut != nullptr) *eliminatedOut = eliminated;
    return program;
}

} // namespace quad
