#include "inline.hh"

#include <map>
#include <set>
#include <string>
#include <vector>

#include "quad_metadata.hh"
#include "temp.hh"

namespace {

constexpr std::size_t kMaxInlineBodyStatements = 16;

struct InlineCandidate {
    quad::QuadFuncDecl *function = nullptr;
    std::vector<quad::QuadStm *> body;
    quad::QuadReturn *returnStatement = nullptr;
};

bool isIntTerm(quad::QuadTerm *term, const std::set<int> &available,
               const std::set<int> &parameters, std::set<int> *usedParameters) {
    if (term == nullptr) return false;
    if (term->kind == quad::QuadTermKind::CONST) return true;
    if (term->kind != quad::QuadTermKind::TEMP || term->get_temp() == nullptr ||
        term->get_temp()->temp == nullptr ||
        term->get_temp()->type != quad::QuadType::INT) {
        return false;
    }
    int number = term->get_temp()->temp->num;
    if (available.count(number) == 0) return false;
    if (usedParameters != nullptr && parameters.count(number) != 0)
        usedParameters->insert(number);
    return true;
}

bool isIntBinop(const std::string &op) {
    static const std::set<std::string> supported = {
        "+", "-", "*", "/", "%", "&&", "||", "&", "|", "^",
        "<<", ">>", "==", "!=", "<", "<=", ">", ">="};
    return supported.count(op) != 0;
}

bool addIntDefinition(quad::QuadTemp *destination, std::set<int> &available,
                      const std::set<int> &parameters) {
    if (destination == nullptr || destination->temp == nullptr ||
        destination->type != quad::QuadType::INT) {
        return false;
    }
    int number = destination->temp->num;
    if (parameters.count(number) != 0 || available.count(number) != 0)
        return false;
    available.insert(number);
    return true;
}

bool analyzeCandidate(quad::QuadFuncDecl *function, InlineCandidate &candidate) {
    if (function == nullptr || function->return_type != quad::QuadType::INT ||
        function->quadblocklist == nullptr ||
        function->quadblocklist->size() != 1 || function->params == nullptr) {
        return false;
    }

    quad::QuadBlock *block = function->quadblocklist->front();
    if (block == nullptr || block->entry_label == nullptr ||
        block->quadlist == nullptr || block->quadlist->size() < 2 ||
        block->quadlist->size() > kMaxInlineBodyStatements + 2 ||
        (block->exit_labels != nullptr && !block->exit_labels->empty())) {
        return false;
    }

    auto *label = block->quadlist->front();
    auto *last = block->quadlist->back();
    if (label == nullptr || label->kind != quad::QuadKind::LABEL ||
        static_cast<quad::QuadLabel *>(label)->label == nullptr ||
        static_cast<quad::QuadLabel *>(label)->label->num !=
            block->entry_label->num ||
        last == nullptr || last->kind != quad::QuadKind::RETURN) {
        return false;
    }

    std::set<int> parameters;
    std::set<int> available;
    bool hasTypedSignature = function->param_types != nullptr &&
                             function->param_types->size() ==
                                 function->params->size();
    if (hasTypedSignature) {
        for (quad::QuadType type : *function->param_types) {
            if (type != quad::QuadType::INT) return false;
        }
    }
    for (auto *parameter : *function->params) {
        if (parameter == nullptr || !parameters.insert(parameter->num).second)
            return false;
        available.insert(parameter->num);
    }

    std::set<int> usedParameters;
    std::vector<quad::QuadStm *> body;
    for (std::size_t index = 1; index + 1 < block->quadlist->size(); ++index) {
        quad::QuadStm *statement = block->quadlist->at(index);
        if (statement == nullptr) return false;
        if (statement->kind == quad::QuadKind::MOVE) {
            auto *move = static_cast<quad::QuadMove *>(statement);
            if (!isIntTerm(move->src, available, parameters, &usedParameters) ||
                !addIntDefinition(move->dst, available, parameters)) {
                return false;
            }
        } else if (statement->kind == quad::QuadKind::MOVE_BINOP) {
            auto *binop = static_cast<quad::QuadMoveBinop *>(statement);
            if (!isIntBinop(binop->binop) ||
                !isIntTerm(binop->left, available, parameters,
                           &usedParameters) ||
                !isIntTerm(binop->right, available, parameters,
                           &usedParameters) ||
                !addIntDefinition(binop->dst, available, parameters)) {
                return false;
            }
        } else {
            // This excludes calls, memory and pointer operations, PHIs, and
            // every form of control flow from an inline candidate.
            return false;
        }
        body.push_back(statement);
    }

    auto *returnStatement = static_cast<quad::QuadReturn *>(last);
    if (!isIntTerm(returnStatement->exp, available, parameters,
                   &usedParameters) ||
        (!hasTypedSignature && usedParameters != parameters)) {
        // Legacy hand-built Quad without signature metadata still requires
        // every parameter to occur in an INT-typed operand.  A typed signature
        // safely admits otherwise-unused integer parameters.
        return false;
    }

    candidate.function = function;
    candidate.body = std::move(body);
    candidate.returnStatement = returnStatement;
    return true;
}

bool isIntCallArgument(quad::QuadTerm *argument) {
    return argument != nullptr &&
           (argument->kind == quad::QuadTermKind::CONST ||
            (argument->kind == quad::QuadTermKind::TEMP &&
             argument->get_temp() != nullptr &&
             argument->get_temp()->temp != nullptr &&
             argument->get_temp()->type == quad::QuadType::INT));
}

quad::QuadTerm *cloneSubstitutedTerm(
    quad::QuadTerm *term,
    const std::map<int, quad::QuadTerm *> &parameterValues,
    const std::map<int, int> &localTemps) {
    if (term == nullptr) return nullptr;
    if (term->kind == quad::QuadTermKind::CONST)
        return new quad::QuadTerm(term->get_const());
    if (term->kind != quad::QuadTermKind::TEMP || term->get_temp() == nullptr ||
        term->get_temp()->temp == nullptr ||
        term->get_temp()->type != quad::QuadType::INT) {
        return nullptr;
    }

    int oldNumber = term->get_temp()->temp->num;
    auto parameter = parameterValues.find(oldNumber);
    if (parameter != parameterValues.end()) return parameter->second->clone();
    auto local = localTemps.find(oldNumber);
    if (local == localTemps.end()) return nullptr;
    return new quad::QuadTerm(new quad::QuadTemp(
        new Temp(local->second), quad::QuadType::INT));
}

quad::QuadStm *cloneBodyStatement(
    quad::QuadStm *statement,
    const std::map<int, quad::QuadTerm *> &parameterValues,
    std::map<int, int> &localTemps, int &lastTemp) {
    if (statement == nullptr) return nullptr;
    if (statement->kind == quad::QuadKind::MOVE) {
        auto *move = static_cast<quad::QuadMove *>(statement);
        quad::QuadTerm *source =
            cloneSubstitutedTerm(move->src, parameterValues, localTemps);
        if (source == nullptr || move->dst == nullptr ||
            move->dst->temp == nullptr) {
            return nullptr;
        }
        int fresh = ++lastTemp;
        localTemps[move->dst->temp->num] = fresh;
        return new quad::QuadMove(
            new quad::QuadTemp(new Temp(fresh), quad::QuadType::INT), source,
            new std::set<Temp *>(), new std::set<Temp *>());
    }
    if (statement->kind == quad::QuadKind::MOVE_BINOP) {
        auto *binop = static_cast<quad::QuadMoveBinop *>(statement);
        quad::QuadTerm *left =
            cloneSubstitutedTerm(binop->left, parameterValues, localTemps);
        quad::QuadTerm *right =
            cloneSubstitutedTerm(binop->right, parameterValues, localTemps);
        if (left == nullptr || right == nullptr || binop->dst == nullptr ||
            binop->dst->temp == nullptr) {
            return nullptr;
        }
        int fresh = ++lastTemp;
        localTemps[binop->dst->temp->num] = fresh;
        return new quad::QuadMoveBinop(
            new quad::QuadTemp(new Temp(fresh), quad::QuadType::INT), left,
            binop->binop, right, new std::set<Temp *>(),
            new std::set<Temp *>());
    }
    return nullptr;
}

bool inlineCall(quad::QuadTemp *callDestination,
                std::vector<quad::QuadTerm *> *callArguments,
                const InlineCandidate &candidate, int &lastTemp,
                std::vector<quad::QuadStm *> &replacement) {
    if (callDestination == nullptr || callDestination->temp == nullptr ||
        callDestination->type != quad::QuadType::INT ||
        callArguments == nullptr || candidate.function == nullptr ||
        candidate.function->params == nullptr || callArguments->size() !=
            candidate.function->params->size()) {
        return false;
    }

    std::map<int, quad::QuadTerm *> parameterValues;
    for (std::size_t index = 0;
         index < candidate.function->params->size(); ++index) {
        quad::QuadTerm *argument = callArguments->at(index);
        Temp *parameter = candidate.function->params->at(index);
        if (parameter == nullptr || !isIntCallArgument(argument)) return false;
        parameterValues[parameter->num] = argument;
    }

    int trialLastTemp = lastTemp;
    std::map<int, int> localTemps;
    std::vector<quad::QuadStm *> clonedBody;
    for (auto *statement : candidate.body) {
        quad::QuadStm *clone = cloneBodyStatement(
            statement, parameterValues, localTemps, trialLastTemp);
        if (clone == nullptr) return false;
        clonedBody.push_back(clone);
    }

    quad::QuadTerm *returnValue = cloneSubstitutedTerm(
        candidate.returnStatement->exp, parameterValues, localTemps);
    if (returnValue == nullptr) return false;
    replacement.insert(replacement.end(), clonedBody.begin(), clonedBody.end());
    replacement.push_back(new quad::QuadMove(
        callDestination->clone(), returnValue, new std::set<Temp *>(),
        new std::set<Temp *>()));
    lastTemp = trialLastTemp;
    return true;
}

bool getInlineCall(quad::QuadStm *statement, std::string &name,
                   quad::QuadTemp *&destination,
                   std::vector<quad::QuadTerm *> *&arguments) {
    if (statement == nullptr) return false;
    if (statement->kind == quad::QuadKind::MOVE_EXTCALL) {
        auto *call = static_cast<quad::QuadMoveExtCall *>(statement);
        if (call->extcall == nullptr) return false;
        name = call->extcall->extfun;
        destination = call->dst;
        arguments = call->extcall->args;
        return true;
    }
    if (statement->kind == quad::QuadKind::MOVE_CALL) {
        auto *call = static_cast<quad::QuadMoveCall *>(statement);
        if (call->call == nullptr) return false;
        name = call->call->name;
        destination = call->dst;
        arguments = call->call->args;
        return true;
    }
    return false;
}

} // namespace

namespace quad {

QuadProgram *inlineProg(QuadProgram *program, int *eliminatedOut) {
    if (eliminatedOut != nullptr) *eliminatedOut = 0;
    if (program == nullptr || program->quadFuncDeclList == nullptr)
        return program;

    rebuildQuadProgramMetadata(program);
    std::map<std::string, InlineCandidate> candidates;
    for (auto *function : *program->quadFuncDeclList) {
        InlineCandidate candidate;
        if (analyzeCandidate(function, candidate))
            candidates[function->funcname] = std::move(candidate);
    }
    if (candidates.empty()) return program;

    int lastTemp = program->last_temp_num;
    int inlined = 0;
    for (auto *function : *program->quadFuncDeclList) {
        if (function == nullptr || function->quadblocklist == nullptr) continue;
        for (auto *block : *function->quadblocklist) {
            if (block == nullptr || block->quadlist == nullptr) continue;
            auto *rewritten = new std::vector<QuadStm *>();
            for (auto *statement : *block->quadlist) {
                std::string name;
                QuadTemp *destination = nullptr;
                std::vector<QuadTerm *> *arguments = nullptr;
                if (getInlineCall(statement, name, destination, arguments)) {
                    auto candidate = candidates.find(name);
                    if (candidate != candidates.end() && inlineCall(
                            destination, arguments, candidate->second,
                            lastTemp, *rewritten)) {
                        ++inlined;
                        continue;
                    }
                }
                rewritten->push_back(statement);
            }
            block->quadlist = rewritten;
        }
    }

    program->last_temp_num = lastTemp;
    rebuildQuadProgramMetadata(program);
    if (eliminatedOut != nullptr) *eliminatedOut = inlined;
    return program;
}

} // namespace quad
