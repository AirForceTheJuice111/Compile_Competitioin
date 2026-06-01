#define DEBUG
#undef DEBUG

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "ig.hh"
#include "coloring.hh"

//return true if any node is removed
bool Coloring::simplify() {
#ifdef DEBUG
    cout << "Simplifying..." << endl;
#endif
    bool changed = false;
    
    vector<int> toRemove;
    for (const auto &entry : graph) {
        int node = entry.first;
        if (isMachineReg(node)) continue;
        if (static_cast<int>(entry.second.size()) < k) {
            toRemove.push_back(node);
        }
    }

    for (int node : toRemove) {
        simplifiedNodes.push(node);
        eraseNode(node);
        changed = true;
    }

#ifdef DEBUG
    cout << "Simplifying done. Changed=" << changed << endl;
#endif
    return changed;
}

//return true if changed anything, false otherwise
bool Coloring::coalesce() {
#ifdef DEBUG
    cout << "Coalescing..." << endl;
#endif
    if (ig == nullptr) return false;
    bool changed = false;
    
    // Conservative implementation: keep move pairs for now. The allocator is
    // still correct without coalescing; it may emit more moves or spills.

    return changed;
}

//freeze the moves that are not coalesced
//return true if changed anything, false otherwise
bool Coloring::freeze() {
#ifdef DEBUG
    cout << "Freezing..." << endl;
#endif
    bool changed = false;

    if (movePairs.empty()) return false;

    for (auto it = movePairs.begin(); it != movePairs.end();) {
        int left = it->first;
        int right = it->second;
        bool canFreezeLeft = graph.find(left) != graph.end() && !isMachineReg(left) &&
                             static_cast<int>(graph[left].size()) < k;
        bool canFreezeRight = graph.find(right) != graph.end() && !isMachineReg(right) &&
                              static_cast<int>(graph[right].size()) < k;
        if (canFreezeLeft || canFreezeRight) {
            it = movePairs.erase(it);
            changed = true;
        } else {
            ++it;
        }
    }

    return changed;
}

//This is a soft spill: we just remove the node from the graph and add it to the simplified nodes
//as if nothing happened. The actual spill happens when select&coloring
bool Coloring::spill() {
#ifdef DEBUG
    cout << "Spilling..." << endl;
#endif

    bool changed = false;
    
    int spillCandidate = -1;
    size_t maxDegree = 0;
    for (const auto &entry : graph) {
        int node = entry.first;
        if (isMachineReg(node)) continue;
        if (entry.second.size() >= maxDegree) {
            spillCandidate = node;
            maxDegree = entry.second.size();
        }
    }

    if (spillCandidate >= 0) {
        simplifiedNodes.push(spillCandidate);
        eraseNode(spillCandidate);
        changed = true;
    }

    return changed;
}

//now try to select the registers for the nodes
bool Coloring::select() {
#ifdef DEBUG
    cout << "Selecting..." << endl;
#endif

    bool all_covered = true;

    for (const auto &entry : ig->graph) {
        int node = entry.first;
        if (isMachineReg(node)) colors[node] = node;
    }

    while (!simplifiedNodes.empty()) {
        int node = simplifiedNodes.top();
        simplifiedNodes.pop();

        set<int> unavailable;
        auto neighbors = ig->graph.find(node);
        if (neighbors != ig->graph.end()) {
            for (int neighbor : neighbors->second) {
                auto found = colors.find(neighbor);
                if (found != colors.end() && found->second >= 0 && found->second < k) {
                    unavailable.insert(found->second);
                }
            }
        }

        int selectedColor = -1;
        for (int color = 0; color < k; ++color) {
            if (unavailable.find(color) == unavailable.end()) {
                selectedColor = color;
                break;
            }
        }

        if (selectedColor < 0) {
            spilled.insert(node);
            all_covered = false;
        } else {
            colors[node] = selectedColor;
        }
    }

    for (const auto &entry : ig->graph) {
        int node = entry.first;
        if (colors.find(node) == colors.end() && spilled.find(node) == spilled.end()) {
            if (isMachineReg(node)) {
                colors[node] = node;
            } else {
                spilled.insert(node);
                all_covered = false;
            }
        }
    }

    return all_covered; //return true if all nodes are colored, false otherwise
}
