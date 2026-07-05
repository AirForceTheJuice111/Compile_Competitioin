#include "advDFG.hh"

#include <unordered_map>
#include <vector>

namespace instr {

static int firstDefTemp(const quad::QuadStm *stm) {
    if (stm == nullptr || stm->def == nullptr || stm->def->empty()) {
        return -1;
    }
    return (*stm->def->begin())->num;
}

static std::set<int> usedTemps(const quad::QuadStm *stm) {
    std::set<int> out;
    if (stm == nullptr || stm->use == nullptr) {
        return out;
    }
    for (auto *temp : *stm->use) {
        if (temp != nullptr) {
            out.insert(temp->num);
        }
    }
    return out;
}

static bool touchesMemoryOrCall(const quad::QuadStm *stm) {
    if (stm == nullptr) {
        return false;
    }
    switch (stm->kind) {
        case quad::QuadKind::LOAD:
        case quad::QuadKind::STORE:
        case quad::QuadKind::CALL:
        case quad::QuadKind::MOVE_CALL:
        case quad::QuadKind::EXTCALL:
        case quad::QuadKind::MOVE_EXTCALL:
            return true;
        default:
            return false;
    }
}

static bool isExitStatement(const quad::QuadStm *stm) {
    if (stm == nullptr) {
        return false;
    }
    return stm->kind == quad::QuadKind::JUMP ||
           stm->kind == quad::QuadKind::CJUMP ||
           stm->kind == quad::QuadKind::RETURN;
}

advDFGprog *buildAdvDFGprog(const quad::QuadProgram *program) {
    auto *out = new advDFGprog(program);
    if (program == nullptr || program->quadFuncDeclList == nullptr) {
        return out;
    }

    for (auto *func : *program->quadFuncDeclList) {
        auto *funcGraph = new advDFGfunc(func);
        out->addFunc(funcGraph);
        if (func == nullptr || func->quadblocklist == nullptr) {
            continue;
        }

        for (auto *block : *func->quadblocklist) {
            auto *blockGraph = new advDFGblock(block);
            funcGraph->addBlock(blockGraph);
            if (block == nullptr || block->quadlist == nullptr) {
                continue;
            }

            auto *entryNode = new advDFGNode(NodeType::EntryLabel);
            blockGraph->graph.addNode(entryNode);

            std::unordered_map<int, advDFGNode*> lastTempDef;
            advDFGNode *lastChainDef = nullptr;
            int nextChain = 0;
            std::vector<advDFGNode*> statementNodes;

            for (auto *stm : *block->quadlist) {
                if (stm == nullptr || stm->kind == quad::QuadKind::PHI ||
                    stm->kind == quad::QuadKind::LABEL) {
                    continue;
                }

                auto type = isExitStatement(stm) ? NodeType::ExitStatement : NodeType::Statement;
                auto *node = new advDFGNode(type, stm);
                node->tempDefined = firstDefTemp(stm);
                node->tempsUsed = usedTemps(stm);

                blockGraph->graph.addEdge(entryNode, node); // entry to statement edges

                if (touchesMemoryOrCall(stm)) { // memory / call chain edges
                    node->chainDefined = nextChain++;
                    if (lastChainDef != nullptr) {
                        node->chainUsed = lastChainDef->chainDefined;
                        blockGraph->graph.addEdge(lastChainDef, node);
                    }
                    lastChainDef = node;
                }

                for (int used : node->tempsUsed) { // def-use edges
                    auto found = lastTempDef.find(used);
                    if (found != lastTempDef.end()) {
                        blockGraph->graph.addEdge(found->second, node);
                    } else {
                        blockGraph->graph.addEdge(entryNode, node);
                    }
                }

                if (node->tempDefined >= 0) { // tempDefined >= 0 means this node defines a temporary, so update lastTempDef
                    lastTempDef[node->tempDefined] = node;
                }

                blockGraph->graph.addNode(node);
                statementNodes.push_back(node);
            }

            advDFGNode *lastStatement = nullptr;
            for (auto it = statementNodes.rbegin(); it != statementNodes.rend(); ++it) {
                if ((*it)->type == NodeType::ExitStatement) {
                    lastStatement = *it;
                    break;
                }
            }
            if (lastStatement == nullptr && !statementNodes.empty()) {
                lastStatement = statementNodes.back();
            }

            if (lastStatement != nullptr) {
                for (auto *node : statementNodes) {
                    if (node != lastStatement) {
                        blockGraph->graph.addEdge(node, lastStatement); // node-to-exit edges
                    }
                }
            }
        }
    }

    return out;

}

} // namespace instr
