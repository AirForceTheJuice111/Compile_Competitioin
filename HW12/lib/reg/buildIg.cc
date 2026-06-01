// #define DEBUG
#undef DEBUG

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "temp.hh"
#include "ig.hh"
#include "asmdataflow.hh"
#include "asmprog.hh"

using namespace std;

static void addNode(map<int, set<int>> &graph, int node) {
    if (graph.find(node) == graph.end()) {
        graph[node] = set<int>();
    }
}

static void addEdge(map<int, set<int>> &graph, int left, int right) {
    if (left == right) return;
    addNode(graph, left);
    addNode(graph, right);
    graph[left].insert(right);
    graph[right].insert(left);
}

static pair<int, int> orderedPair(int left, int right) {
    if (left > right) swap(left, right);
    return {left, right};
}

InterferenceGraph *buildIg(instr::AsmFunction* asmFunc, instr::AsmDataFlowInfo* flowInfo) {
    map<int, set<int>> graph;
    set<pair<int, int>> movePairs;

    if (asmFunc == nullptr || flowInfo == nullptr) {
        return new InterferenceGraph(graph, movePairs);
    }

    for (size_t i = 0; i < asmFunc->instructions.size(); ++i) {
        auto &assemInstr = asmFunc->instructions[i];
        set<int> use = flowInfo->getUse(i);
        set<int> def = flowInfo->getDef(i);
        set<int> liveout = flowInfo->liveout[i];

        for (int temp : use) addNode(graph, temp);
        for (int temp : def) addNode(graph, temp);
        for (int temp : liveout) addNode(graph, temp);

        bool isMove = assemInstr.kind == instr::AssemInstr::I_MOVE &&
                      assemInstr.dst.size() == 1 && assemInstr.src.size() == 1 &&
                      assemInstr.dst[0] != nullptr && assemInstr.src[0] != nullptr;
        int moveDst = isMove ? assemInstr.dst[0]->num : -1;
        int moveSrc = isMove ? assemInstr.src[0]->num : -1;
        if (isMove && moveDst != moveSrc) {
            movePairs.insert(orderedPair(moveDst, moveSrc));
        }

        for (int d : def) {
            for (int l : liveout) {
                if (isMove && l == moveSrc) continue;
                addEdge(graph, d, l);
            }
        }
    }

    return new InterferenceGraph(graph, movePairs);
}

// Build interference graphs for all functions in AsmProg

vector<InterferenceGraph*> buildIgProg(instr::AsmProg* program) {
    vector<InterferenceGraph*> graphs;
    
    if (program == nullptr || program->functions.empty()) {
        return graphs;
    }

#ifdef DEBUG
    cout << "Building interference graphs for program with " << program->functions.size() << " functions" << endl;
#endif

    for (auto &func : program->functions) {
        auto *flowInfo = new instr::AsmDataFlowInfo(&func);
        flowInfo->computeLiveness();
        graphs.push_back(buildIg(&func, flowInfo));
        delete flowInfo;
    }

    return graphs;
}
