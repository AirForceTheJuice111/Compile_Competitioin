#include "loopunroll.hh"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <map>
#include <set>
#include <vector>

#include "cfg_transform.hh"
#include "quad_metadata.hh"

namespace quad {
namespace {

std::set<Temp *> *emptyTemps() {
    return new std::set<Temp *>();
}

QuadTemp *definedTemp(QuadStm *stm) {
    if (stm == nullptr) return nullptr;
    switch (stm->kind) {
    case QuadKind::MOVE: return static_cast<QuadMove *>(stm)->dst;
    case QuadKind::LOAD: return static_cast<QuadLoad *>(stm)->dst;
    case QuadKind::MOVE_BINOP: return static_cast<QuadMoveBinop *>(stm)->dst;
    case QuadKind::MOVE_CALL: return static_cast<QuadMoveCall *>(stm)->dst;
    case QuadKind::MOVE_EXTCALL: return static_cast<QuadMoveExtCall *>(stm)->dst;
    case QuadKind::PHI: return static_cast<QuadPhi *>(stm)->temp_exp;
    case QuadKind::PTR_CALC: {
        auto *term = static_cast<QuadPtrCalc *>(stm)->dst;
        return term != nullptr && term->kind == QuadTermKind::TEMP
            ? term->get_temp() : nullptr;
    }
    default: return nullptr;
    }
}

bool replaceDefinition(QuadStm *stm, int fresh) {
    QuadTemp *def = definedTemp(stm);
    if (def == nullptr || def->temp == nullptr) return false;
    QuadType type = def->type;
    switch (stm->kind) {
    case QuadKind::MOVE:
        static_cast<QuadMove *>(stm)->dst = new QuadTemp(new Temp(fresh), type);
        return true;
    case QuadKind::LOAD:
        static_cast<QuadLoad *>(stm)->dst = new QuadTemp(new Temp(fresh), type);
        return true;
    case QuadKind::MOVE_BINOP:
        static_cast<QuadMoveBinop *>(stm)->dst = new QuadTemp(new Temp(fresh), type);
        return true;
    case QuadKind::PTR_CALC:
        static_cast<QuadPtrCalc *>(stm)->dst =
            new QuadTerm(new QuadTemp(new Temp(fresh), type));
        return true;
    default:
        return false;
    }
}

std::map<int, QuadStm *> collectDefinitions(QuadFuncDecl *func) {
    std::map<int, QuadStm *> result;
    if (func == nullptr || func->quadblocklist == nullptr) return result;
    for (auto *block : *func->quadblocklist) {
        if (block == nullptr || block->quadlist == nullptr) continue;
        for (auto *stm : *block->quadlist) {
            QuadTemp *def = definedTemp(stm);
            if (def != nullptr && def->temp != nullptr) result[def->temp->num] = stm;
        }
    }
    return result;
}

bool resolveConstant(QuadTerm *term, const std::map<int, QuadStm *> &defs,
                     int64_t &value) {
    std::set<int> visited;
    for (int depth = 0; depth < 16 && term != nullptr; ++depth) {
        if (term->kind == QuadTermKind::CONST) {
            value = term->get_const();
            return true;
        }
        if (term->kind != QuadTermKind::TEMP || term->get_temp() == nullptr ||
            term->get_temp()->temp == nullptr) {
            return false;
        }
        int number = term->get_temp()->temp->num;
        if (!visited.insert(number).second) return false;
        auto found = defs.find(number);
        if (found == defs.end() || found->second == nullptr ||
            found->second->kind != QuadKind::MOVE) {
            return false;
        }
        term = static_cast<QuadMove *>(found->second)->src;
    }
    return false;
}

bool termIsTemp(QuadTerm *term, int number) {
    return term != nullptr && term->kind == QuadTermKind::TEMP &&
           term->get_temp() != nullptr && term->get_temp()->temp != nullptr &&
           term->get_temp()->temp->num == number;
}

bool evalRelation(const std::string &relop, int64_t left, int64_t right,
                  bool &result) {
    if (relop == "<") result = left < right;
    else if (relop == "<=") result = left <= right;
    else if (relop == ">") result = left > right;
    else if (relop == ">=") result = left >= right;
    else if (relop == "==") result = left == right;
    else if (relop == "!=") result = left != right;
    else return false;
    return true;
}

struct HeaderPhi {
    QuadPhi *phi = nullptr;
    int initial = -1;
    int backedge = -1;
};

struct UnrollCandidate {
    int header = -1;
    int body = -1;
    int preheader = -1;
    int exit = -1;
    int tripCount = 0;
    std::vector<HeaderPhi> phis;
    std::vector<QuadStm *> bodyStatements;
};

bool phiInputs(QuadPhi *phi, int preheader, int body, HeaderPhi &result) {
    if (phi == nullptr || phi->temp_exp == nullptr || phi->temp_exp->temp == nullptr ||
        phi->args == nullptr || phi->args->size() != 2) return false;
    result.phi = phi;
    for (const auto &arg : *phi->args) {
        if (arg.first == nullptr || arg.second == nullptr) return false;
        if (arg.second->num == preheader) result.initial = arg.first->num;
        else if (arg.second->num == body) result.backedge = arg.first->num;
        else return false;
    }
    return result.initial >= 0 && result.backedge >= 0;
}

bool findStep(const HeaderPhi &iv, const std::map<int, QuadStm *> &defs,
              int64_t &step) {
    auto found = defs.find(iv.backedge);
    if (found == defs.end() || found->second == nullptr ||
        found->second->kind != QuadKind::MOVE_BINOP) return false;
    auto *binop = static_cast<QuadMoveBinop *>(found->second);
    int phiNumber = iv.phi->temp_exp->temp->num;
    int64_t constant = 0;
    if (binop->binop == "+") {
        if (termIsTemp(binop->left, phiNumber) &&
            resolveConstant(binop->right, defs, constant)) {
            step = constant;
            return step != 0;
        }
        if (termIsTemp(binop->right, phiNumber) &&
            resolveConstant(binop->left, defs, constant)) {
            step = constant;
            return step != 0;
        }
    }
    if (binop->binop == "-" && termIsTemp(binop->left, phiNumber) &&
        resolveConstant(binop->right, defs, constant)) {
        step = -constant;
        return step != 0;
    }
    return false;
}

bool analyzeLoop(QuadFuncDecl *func, ControlFlowInfo *cfi,
                 const NaturalLoopInfo &loop, const LoopUnrollOptions &options,
                 UnrollCandidate &candidate) {
    if (loop.blocks.size() != 2 || loop.outsidePredecessors.size() != 1 ||
        loop.backedgePredecessors.size() != 1 || loop.exitEdges.size() != 1) {
        return false;
    }
    int body = *loop.backedgePredecessors.begin();
    if (body == loop.header || loop.blocks.count(body) == 0 ||
        cfi->successors[body] != std::set<int>{loop.header}) return false;
    int preheader = *loop.outsidePredecessors.begin();
    if (cfi->successors[preheader] != std::set<int>{loop.header}) return false;

    QuadBlock *headerBlock = findQuadBlock(func, loop.header);
    QuadBlock *bodyBlock = findQuadBlock(func, body);
    if (headerBlock == nullptr || bodyBlock == nullptr ||
        headerBlock->quadlist == nullptr || bodyBlock->quadlist == nullptr ||
        headerBlock->quadlist->empty() || bodyBlock->quadlist->empty()) return false;
    QuadStm *headerLast = headerBlock->quadlist->back();
    QuadStm *bodyLast = bodyBlock->quadlist->back();
    if (headerLast == nullptr || headerLast->kind != QuadKind::CJUMP ||
        bodyLast == nullptr || bodyLast->kind != QuadKind::JUMP) return false;
    auto *condition = static_cast<QuadCJump *>(headerLast);
    int trueTarget = condition->t == nullptr ? -1 : condition->t->num;
    int falseTarget = condition->f == nullptr ? -1 : condition->f->num;
    bool bodyWhenTrue;
    int exit;
    if (trueTarget == body && loop.blocks.count(falseTarget) == 0) {
        bodyWhenTrue = true;
        exit = falseTarget;
    } else if (falseTarget == body && loop.blocks.count(trueTarget) == 0) {
        bodyWhenTrue = false;
        exit = trueTarget;
    } else {
        return false;
    }

    std::vector<HeaderPhi> phis;
    for (std::size_t i = 1; i + 1 < headerBlock->quadlist->size(); ++i) {
        QuadStm *stm = headerBlock->quadlist->at(i);
        if (stm == nullptr || stm->kind != QuadKind::PHI) return false;
        HeaderPhi phi;
        if (!phiInputs(static_cast<QuadPhi *>(stm), preheader, body, phi)) return false;
        phis.push_back(phi);
    }
    if (phis.empty()) return false;

    std::vector<QuadStm *> bodyStatements;
    int definitions = 0;
    for (std::size_t i = 1; i + 1 < bodyBlock->quadlist->size(); ++i) {
        QuadStm *stm = bodyBlock->quadlist->at(i);
        if (stm == nullptr || stm->kind == QuadKind::LABEL ||
            stm->kind == QuadKind::PHI || stm->kind == QuadKind::JUMP ||
            stm->kind == QuadKind::CJUMP || stm->kind == QuadKind::RETURN ||
            stm->kind == QuadKind::CALL || stm->kind == QuadKind::MOVE_CALL ||
            stm->kind == QuadKind::EXTCALL || stm->kind == QuadKind::MOVE_EXTCALL) {
            return false;
        }
        if (definedTemp(stm) != nullptr) ++definitions;
        bodyStatements.push_back(stm);
    }
    if (bodyStatements.empty() ||
        static_cast<int>(bodyStatements.size()) > options.maxBodyInstructions) return false;

    const auto defs = collectDefinitions(func);
    HeaderPhi *induction = nullptr;
    bool inductionOnLeft = false;
    int64_t bound = 0;
    for (auto &phi : phis) {
        int number = phi.phi->temp_exp->temp->num;
        if (termIsTemp(condition->left, number) &&
            resolveConstant(condition->right, defs, bound)) {
            induction = &phi;
            inductionOnLeft = true;
            break;
        }
        if (termIsTemp(condition->right, number) &&
            resolveConstant(condition->left, defs, bound)) {
            induction = &phi;
            inductionOnLeft = false;
            break;
        }
    }
    if (induction == nullptr || induction->phi->temp_exp->type != QuadType::INT) return false;
    int64_t initial = 0;
    QuadTerm initialTerm(new QuadTemp(new Temp(induction->initial), QuadType::INT));
    if (!resolveConstant(&initialTerm, defs, initial)) return false;
    int64_t step = 0;
    if (!findStep(*induction, defs, step)) return false;

    int trips = 0;
    int64_t value = initial;
    for (;;) {
        bool relation = false;
        if (!evalRelation(condition->relop,
                          inductionOnLeft ? value : bound,
                          inductionOnLeft ? bound : value,
                          relation)) return false;
        bool entersBody = bodyWhenTrue ? relation : !relation;
        if (!entersBody) break;
        if (++trips > options.maxTripCount) return false;
        int64_t next = value + step;
        if (next < std::numeric_limits<int32_t>::min() ||
            next > std::numeric_limits<int32_t>::max()) return false;
        value = next;
    }
    if (trips < 2 ||
        static_cast<int>(bodyStatements.size()) * trips > options.maxExpandedInstructions ||
        definitions * trips > options.maxAdditionalTemps) return false;

    candidate.header = loop.header;
    candidate.body = body;
    candidate.preheader = preheader;
    candidate.exit = exit;
    candidate.tripCount = trips;
    candidate.phis = phis;
    candidate.bodyStatements = bodyStatements;
    return true;
}

bool applyUnroll(QuadFuncDecl *func, const UnrollCandidate &candidate,
                 LoopUnrollStats &stats) {
    QuadBlock *exitBlock = findQuadBlock(func, candidate.exit);
    if (exitBlock == nullptr) return false;
    int newLabel = ++func->last_label_num;
    if (!retargetQuadEdge(func, candidate.preheader, candidate.header, newLabel)) {
        return false;
    }

    std::map<int, int> currentPhiValues;
    for (const auto &phi : candidate.phis) {
        currentPhiValues[phi.phi->temp_exp->temp->num] = phi.initial;
    }

    auto *newStatements = new std::vector<QuadStm *>();
    newStatements->push_back(
        new QuadLabel(new Label(newLabel), emptyTemps(), emptyTemps()));
    for (int iteration = 0; iteration < candidate.tripCount; ++iteration) {
        std::map<int, int> iterationDefs;
        for (auto *stm : candidate.bodyStatements) {
            QuadTemp *def = definedTemp(stm);
            if (def != nullptr && def->temp != nullptr) {
                iterationDefs[def->temp->num] = ++func->last_temp_num;
            }
        }
        std::map<int, int> uses = currentPhiValues;
        uses.insert(iterationDefs.begin(), iterationDefs.end());
        for (auto *stm : candidate.bodyStatements) {
            auto *clone = static_cast<QuadStm *>(stm->clone());
            QuadTemp *def = definedTemp(stm);
            if (def != nullptr && def->temp != nullptr &&
                !replaceDefinition(clone, iterationDefs[def->temp->num])) {
                return false;
            }
            rewriteQuadStatementUses(clone, uses);
            newStatements->push_back(clone);
            ++stats.instructionsCloned;
        }
        std::map<int, int> nextValues;
        for (const auto &phi : candidate.phis) {
            int dest = phi.phi->temp_exp->temp->num;
            auto bodyDef = iterationDefs.find(phi.backedge);
            if (bodyDef != iterationDefs.end()) nextValues[dest] = bodyDef->second;
            else {
                auto mapped = uses.find(phi.backedge);
                nextValues[dest] = mapped == uses.end() ? phi.backedge : mapped->second;
            }
        }
        currentPhiValues = nextValues;
    }
    newStatements->push_back(
        new QuadJump(new Label(candidate.exit), emptyTemps(), emptyTemps()));
    auto *exits = new std::vector<Label *>();
    exits->push_back(new Label(candidate.exit));
    auto *unrolledBlock = new QuadBlock(newStatements, new Label(newLabel), exits);

    replacePhiPredecessor(exitBlock, candidate.header, newLabel);
    rewriteQuadUses(func, currentPhiValues);
    auto &blocks = *func->quadblocklist;
    blocks.erase(std::remove_if(blocks.begin(), blocks.end(), [&](QuadBlock *block) {
        if (block == nullptr || block->entry_label == nullptr) return false;
        int label = block->entry_label->num;
        return label == candidate.header || label == candidate.body;
    }), blocks.end());
    auto exitPosition = std::find_if(blocks.begin(), blocks.end(), [&](QuadBlock *block) {
        return block != nullptr && block->entry_label != nullptr &&
               block->entry_label->num == candidate.exit;
    });
    blocks.insert(exitPosition, unrolledBlock);
    rebuildQuadFunctionMetadata(func);
    ++stats.loopsUnrolled;
    return true;
}

} // namespace

QuadProgram *loopUnrollProg(QuadProgram *program,
                            const LoopUnrollOptions &options,
                            LoopUnrollStats *statsOut) {
    if (program == nullptr || program->quadFuncDeclList == nullptr) return program;
    LoopUnrollStats stats;
    auto *functions = new std::vector<QuadFuncDecl *>();
    int lastLabel = program->last_label_num;
    int lastTemp = program->last_temp_num;
    for (auto *original : *program->quadFuncDeclList) {
        ++stats.functionsVisited;
        if (original == nullptr) {
            functions->push_back(original);
            ++stats.functionsSkipped;
            continue;
        }
        auto *candidateFunc = static_cast<QuadFuncDecl *>(original->clone());
        candidateFunc->last_label_num = std::max(candidateFunc->last_label_num, lastLabel);
        candidateFunc->last_temp_num = std::max(candidateFunc->last_temp_num, lastTemp);
        rebuildQuadFunctionMetadata(candidateFunc);
        std::string reason;
        if (!verifyQuadSsaCfg(candidateFunc, &reason)) {
            functions->push_back(original);
            ++stats.functionsSkipped;
            continue;
        }

        bool changed = false;
        LoopUnrollStats candidateStats;
        for (int round = 0; round < 16; ++round) {
            ControlFlowInfo *cfi = buildControlFlowInfo(candidateFunc);
            if (cfi == nullptr) break;
            bool transformed = false;
            for (const auto &loop : findNaturalLoopInfo(candidateFunc, cfi)) {
                UnrollCandidate unroll;
                if (!analyzeLoop(candidateFunc, cfi, loop, options, unroll)) continue;
                if (applyUnroll(candidateFunc, unroll, candidateStats)) {
                    transformed = true;
                    changed = true;
                }
                break;
            }
            if (!transformed) break;
        }
        rebuildQuadFunctionMetadata(candidateFunc);
        if (!changed || !verifyQuadSsaCfg(candidateFunc, &reason)) {
            functions->push_back(original);
            if (changed) ++stats.functionsSkipped;
            continue;
        }
        eliminateDeadPureQuadDefs(candidateFunc);
        rebuildQuadFunctionMetadata(candidateFunc);
        if (!verifyQuadSsaCfg(candidateFunc, &reason)) {
            functions->push_back(original);
            ++stats.functionsSkipped;
            continue;
        }
        ++stats.functionsChanged;
        stats.loopsUnrolled += candidateStats.loopsUnrolled;
        stats.instructionsCloned += candidateStats.instructionsCloned;
        lastLabel = std::max(lastLabel, candidateFunc->last_label_num);
        lastTemp = std::max(lastTemp, candidateFunc->last_temp_num);
        functions->push_back(candidateFunc);
    }
    auto *result = new QuadProgram(functions, lastLabel, lastTemp);
    rebuildQuadProgramMetadata(result);
    if (statsOut != nullptr) *statsOut = stats;
    return result;
}

} // namespace quad
