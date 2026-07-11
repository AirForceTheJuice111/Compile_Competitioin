#include "backend_driver.hh"

#include "algebrasimp.hh"
#include "blocking.hh"
#include "canon.hh"
#include "copyprop.hh"
#include "flowinfo.hh"
#include "funcspec.hh"
#include "gvn.hh"
#include "inline.hh"
#include "loopheader.hh"
#include "loopinductionopt.hh"
#include "looplicm.hh"
#include "memopt.hh"
#include "opt.hh"
#include "quad.hh"
#include "quadssa.hh"
#include "tree2quad.hh"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace backend {

namespace {

bool writeText(const std::string &path, const std::string &text) {
    std::ofstream out(path);
    if (!out) {
        return false;
    }
    out << text;
    return true;
}

void noteTemp(tree::Temp *temp, int &maxTemp) {
    if (temp != nullptr) {
        maxTemp = std::max(maxTemp, temp->num);
    }
}

void noteLabel(tree::Label *label, int &maxLabel) {
    if (label != nullptr) {
        maxLabel = std::max(maxLabel, label->num);
    }
}

void noteQuadTemp(quad::QuadTemp *temp, int &maxTemp) {
    if (temp != nullptr) {
        noteTemp(temp->temp, maxTemp);
    }
}

void noteQuadTerm(quad::QuadTerm *term, int &maxTemp) {
    if (term != nullptr && term->kind == quad::QuadTermKind::TEMP) {
        noteQuadTemp(term->get_temp(), maxTemp);
    }
}

void noteTempSet(std::set<tree::Temp *> *temps, int &maxTemp) {
    if (temps == nullptr) {
        return;
    }
    for (auto *temp : *temps) {
        noteTemp(temp, maxTemp);
    }
}

void noteTermList(std::vector<quad::QuadTerm *> *terms, int &maxTemp) {
    if (terms == nullptr) {
        return;
    }
    for (auto *term : *terms) {
        noteQuadTerm(term, maxTemp);
    }
}

void noteCall(quad::QuadCall *call, int &maxTemp) {
    if (call == nullptr) {
        return;
    }
    noteQuadTerm(call->obj_term, maxTemp);
    noteTermList(call->args, maxTemp);
}

void noteExtCall(quad::QuadExtCall *call, int &maxTemp) {
    if (call == nullptr) {
        return;
    }
    noteTermList(call->args, maxTemp);
}

void noteStatementExtents(quad::QuadStm *stm, int &maxLabel, int &maxTemp) {
    if (stm == nullptr) {
        return;
    }
    noteTempSet(stm->def, maxTemp);
    noteTempSet(stm->use, maxTemp);

    switch (stm->kind) {
    case quad::QuadKind::MOVE: {
        auto *s = static_cast<quad::QuadMove *>(stm);
        noteQuadTemp(s->dst, maxTemp);
        noteQuadTerm(s->src, maxTemp);
        break;
    }
    case quad::QuadKind::LOAD: {
        auto *s = static_cast<quad::QuadLoad *>(stm);
        noteQuadTemp(s->dst, maxTemp);
        noteQuadTerm(s->src, maxTemp);
        break;
    }
    case quad::QuadKind::STORE: {
        auto *s = static_cast<quad::QuadStore *>(stm);
        noteQuadTerm(s->src, maxTemp);
        noteQuadTerm(s->dst, maxTemp);
        break;
    }
    case quad::QuadKind::MOVE_BINOP: {
        auto *s = static_cast<quad::QuadMoveBinop *>(stm);
        noteQuadTemp(s->dst, maxTemp);
        noteQuadTerm(s->left, maxTemp);
        noteQuadTerm(s->right, maxTemp);
        break;
    }
    case quad::QuadKind::CALL:
        noteCall(static_cast<quad::QuadCall *>(stm), maxTemp);
        break;
    case quad::QuadKind::MOVE_CALL: {
        auto *s = static_cast<quad::QuadMoveCall *>(stm);
        noteQuadTemp(s->dst, maxTemp);
        noteCall(s->call, maxTemp);
        break;
    }
    case quad::QuadKind::EXTCALL:
        noteExtCall(static_cast<quad::QuadExtCall *>(stm), maxTemp);
        break;
    case quad::QuadKind::MOVE_EXTCALL: {
        auto *s = static_cast<quad::QuadMoveExtCall *>(stm);
        noteQuadTemp(s->dst, maxTemp);
        noteExtCall(s->extcall, maxTemp);
        break;
    }
    case quad::QuadKind::LABEL:
        noteLabel(static_cast<quad::QuadLabel *>(stm)->label, maxLabel);
        break;
    case quad::QuadKind::JUMP:
        noteLabel(static_cast<quad::QuadJump *>(stm)->label, maxLabel);
        break;
    case quad::QuadKind::CJUMP: {
        auto *s = static_cast<quad::QuadCJump *>(stm);
        noteQuadTerm(s->left, maxTemp);
        noteQuadTerm(s->right, maxTemp);
        noteLabel(s->t, maxLabel);
        noteLabel(s->f, maxLabel);
        break;
    }
    case quad::QuadKind::PHI: {
        auto *s = static_cast<quad::QuadPhi *>(stm);
        noteQuadTemp(s->temp_exp, maxTemp);
        if (s->args != nullptr) {
            for (auto &arg : *s->args) {
                noteTemp(arg.first, maxTemp);
                noteLabel(arg.second, maxLabel);
            }
        }
        break;
    }
    case quad::QuadKind::RETURN:
        noteQuadTerm(static_cast<quad::QuadReturn *>(stm)->exp, maxTemp);
        break;
    case quad::QuadKind::PTR_CALC: {
        auto *s = static_cast<quad::QuadPtrCalc *>(stm);
        noteQuadTerm(s->dst, maxTemp);
        noteQuadTerm(s->ptr, maxTemp);
        noteQuadTerm(s->offset, maxTemp);
        break;
    }
    default:
        break;
    }
}

void refreshQuadExtents(quad::QuadProgram *program) {
    if (program == nullptr || program->quadFuncDeclList == nullptr) {
        return;
    }

    int programMaxLabel = program->last_label_num;
    int programMaxTemp = program->last_temp_num;
    for (auto *func : *program->quadFuncDeclList) {
        if (func == nullptr) {
            continue;
        }

        int funcMaxLabel = func->last_label_num;
        int funcMaxTemp = func->last_temp_num;
        if (func->params != nullptr) {
            for (auto *param : *func->params) {
                noteTemp(param, funcMaxTemp);
            }
        }
        if (func->quadblocklist != nullptr) {
            for (auto *block : *func->quadblocklist) {
                if (block == nullptr) {
                    continue;
                }
                noteLabel(block->entry_label, funcMaxLabel);
                if (block->exit_labels != nullptr) {
                    for (auto *label : *block->exit_labels) {
                        noteLabel(label, funcMaxLabel);
                    }
                }
                if (block->quadlist != nullptr) {
                    for (auto *stm : *block->quadlist) {
                        noteStatementExtents(stm, funcMaxLabel, funcMaxTemp);
                    }
                }
            }
        }
        func->last_label_num = funcMaxLabel;
        func->last_temp_num = funcMaxTemp;
        programMaxLabel = std::max(programMaxLabel, funcMaxLabel);
        programMaxTemp = std::max(programMaxTemp, funcMaxTemp);
    }
    program->last_label_num = programMaxLabel;
    program->last_temp_num = programMaxTemp;
}

std::set<FuncFlowInfo *> *computeFlow(quad::QuadProgram *program) {
    const char *detailEnv = std::getenv("BACKEND_PROFILE_DETAIL");
    bool detailProfile = detailEnv != nullptr && detailEnv[0] != '\0' && std::string(detailEnv) != "0";
    auto last = std::chrono::steady_clock::now();
    auto mark = [&](const std::string &name) {
        if (!detailProfile) {
            return;
        }
        auto now = std::chrono::steady_clock::now();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count();
        std::cerr << "BACKEND_PROFILE_DETAIL " << name << " " << millis << "ms\n";
        std::cerr.flush();
        last = now;
    };

    auto *dataFlows = dataFLowProg(program);
    mark("quad-dataflow");
    if (dataFlows == nullptr) {
        return nullptr;
    }

    auto *flows = new std::set<FuncFlowInfo *>();
    for (auto *dfi : *dataFlows) {
        if (dfi == nullptr || dfi->func == nullptr) {
            continue;
        }
        auto *cfi = new ControlFlowInfo(dfi->func);
        cfi->computeEverything();
        mark("quad-controlflow-" + dfi->func->funcname);
        flows->insert(new FuncFlowInfo(cfi, dfi, program->last_label_num, program->last_temp_num));
    }
    return flows;
}

bool optModeUsesSccp(OptMode mode) {
    return mode == OptMode::Const || mode == OptMode::AllOpt;
}

bool optModeUsesGvn(OptMode mode) {
    return mode == OptMode::Const || mode == OptMode::AllOpt;
}

bool optModeUsesLicm(OptMode mode) {
    return mode == OptMode::Loop1 || mode == OptMode::AllLoop || mode == OptMode::AllOpt;
}

bool optModeUsesIv(OptMode mode) {
    return mode == OptMode::Loop2 || mode == OptMode::AllLoop || mode == OptMode::AllOpt;
}

bool passListContains(const char *environment, const std::string &name) {
    const char *env = std::getenv(environment);
    if (env == nullptr || env[0] == '\0') {
        return false;
    }
    std::string enabled(env);
    std::size_t begin = 0;
    while (begin < enabled.size()) {
        begin = enabled.find_first_not_of(" ,;:", begin);
        if (begin == std::string::npos) {
            break;
        }
        std::size_t end = enabled.find_first_of(" ,;:", begin);
        if (enabled.substr(begin, end - begin) == name) {
            return true;
        }
        begin = end == std::string::npos ? enabled.size() : end + 1;
    }
    return false;
}

bool optimizationPassEnabled(const std::string &name) {
    if (passListContains("SYSY_DISABLE_PASSES", name)) {
        return false;
    }
    if (name == "algebrasimp" || name == "gvn" || name == "copyprop" ||
        name == "inline") {
        return true;
    }
    return passListContains("SYSY_EXPERIMENTAL_PASSES", name);
}

quad::QuadProgram *runGvnPass(quad::QuadProgram *program) {
    auto *flow = computeFlow(program);
    if (flow == nullptr) {
        return nullptr;
    }
    int eliminated = 0;
    auto *result = quad::gvnProg(program, flow, &eliminated);
    if (result == nullptr) {
        return program;
    }
    refreshQuadExtents(result);
    if (std::getenv("BACKEND_PROFILE") != nullptr) {
        std::cerr << "BACKEND_PROFILE gvn eliminated " << eliminated << " instructions\n";
    }
    return result;
}

quad::QuadProgram *runInlinePass(quad::QuadProgram *program) {
    int inlined = 0;
    auto *result = quad::inlineProg(program, &inlined);
    if (result == nullptr) {
        return program;
    }
    refreshQuadExtents(result);
    if (std::getenv("BACKEND_PROFILE") != nullptr) {
        std::cerr << "BACKEND_PROFILE inline inlined " << inlined << " calls\n";
    }
    return result;
}

quad::QuadProgram *runAlgebraSimpPass(quad::QuadProgram *program) {
    int eliminated = 0;
    auto *result = quad::algebraSimpProg(program, &eliminated);
    if (result == nullptr) {
        return program;
    }
    refreshQuadExtents(result);
    if (std::getenv("BACKEND_PROFILE") != nullptr) {
        std::cerr << "BACKEND_PROFILE algebrasimp eliminated " << eliminated
                  << " instructions\n";
    }
    return result;
}

quad::QuadProgram *runCopyPropPass(quad::QuadProgram *program) {
    int eliminated = 0;
    auto *result = quad::copyPropProg(program, &eliminated);
    if (result == nullptr) {
        return program;
    }
    refreshQuadExtents(result);
    if (std::getenv("BACKEND_PROFILE") != nullptr) {
        std::cerr << "BACKEND_PROFILE copyprop eliminated " << eliminated
                  << " instructions\n";
    }
    return result;
}

quad::QuadProgram *runMemOptPass(quad::QuadProgram *program) {
    auto *flow = computeFlow(program);
    if (flow == nullptr) {
        return program;
    }
    int eliminated = 0;
    auto *result = quad::memOptProg(program, flow, &eliminated);
    if (result == nullptr) {
        return program;
    }
    refreshQuadExtents(result);
    if (std::getenv("BACKEND_PROFILE") != nullptr) {
        std::cerr << "BACKEND_PROFILE memopt eliminated " << eliminated
                  << " instructions\n";
    }
    return result;
}

quad::QuadProgram *runFuncSpecPass(quad::QuadProgram *program) {
    int specialized = 0;
    auto *result = quad::funcSpecProg(program, &specialized);
    if (result == nullptr) {
        return program;
    }
    refreshQuadExtents(result);
    if (std::getenv("BACKEND_PROFILE") != nullptr) {
        std::cerr << "BACKEND_PROFILE funcspec created " << specialized
                  << " specializations\n";
    }
    return result;
}

quad::QuadProgram *runLicmPass(quad::QuadProgram *program) {
    auto *flow = computeFlow(program);
    if (flow == nullptr) {
        return nullptr;
    }

    auto *licmFuncs = new std::vector<quad::QuadFuncDecl *>();
    int licmLastLabel = program->last_label_num;
    int licmLastTemp = program->last_temp_num;
    for (auto *ffi : *flow) {
        if (ffi == nullptr || ffi->cfi == nullptr || ffi->cfi->func == nullptr) {
            continue;
        }
        auto *func = ffi->cfi->func;
        auto *loopHeaders = findLoopHeadersWithFlow(func, ffi->cfi);
        licmFuncs->push_back(loopHoistFunc(func, loopHeaders));
        if (ffi->programLastLabelNum >= 0) {
            licmLastLabel = ffi->programLastLabelNum;
        }
        if (ffi->programLastTempNum >= 0) {
            licmLastTemp = ffi->programLastTempNum;
        }
    }

    auto *licm = new quad::QuadProgram(licmFuncs, licmLastLabel, licmLastTemp);
    refreshQuadExtents(licm);
    return licm;
}

quad::QuadProgram *runIvPass(quad::QuadProgram *program) {
    auto *flow = computeFlow(program);
    if (flow == nullptr) {
        return nullptr;
    }

    auto *ivFuncs = new std::vector<quad::QuadFuncDecl *>();
    int ivLastLabel = program->last_label_num;
    int ivLastTemp = program->last_temp_num;
    for (auto *ffi : *flow) {
        if (ffi == nullptr || ffi->cfi == nullptr || ffi->cfi->func == nullptr) {
            continue;
        }
        auto *funcList = new std::vector<quad::QuadFuncDecl *>();
        funcList->push_back(ffi->cfi->func);
        auto *funcProg = new quad::QuadProgram(funcList, program->last_label_num, program->last_temp_num);

        auto *strengthReduced = loopInductionStrengthReductionPass(funcProg, ffi->cfi);
        refreshQuadExtents(strengthReduced);
        auto *cleaned = loopInductionCleanupPass(strengthReduced);
        refreshQuadExtents(cleaned);

        if (cleaned != nullptr && cleaned->quadFuncDeclList != nullptr && !cleaned->quadFuncDeclList->empty()) {
            ivFuncs->push_back(cleaned->quadFuncDeclList->at(0));
        }
        if (ffi->programLastLabelNum >= 0) {
            ivLastLabel = ffi->programLastLabelNum;
        }
        if (ffi->programLastTempNum >= 0) {
            ivLastTemp = ffi->programLastTempNum;
        }
    }

    auto *iv = new quad::QuadProgram(ivFuncs, ivLastLabel, ivLastTemp);
    refreshQuadExtents(iv);
    return iv;
}

void maybeWriteQuad(const BackendOptions &options, const std::string &suffix, quad::QuadProgram *program) {
    if (!options.emitDebugFiles || options.debugBase.empty() || program == nullptr) {
        return;
    }
    std::string text;
    program->print(text, 0, true);
    writeText(options.debugBase + suffix, text);
}

class ProfileTimer {
public:
    ProfileTimer() : enabled_(false) {
        const char *env = std::getenv("BACKEND_PROFILE");
        enabled_ = env != nullptr && env[0] != '\0' && std::string(env) != "0";
        last_ = Clock::now();
    }

    void mark(const std::string &name) {
        if (!enabled_) {
            return;
        }
        auto now = Clock::now();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_).count();
        std::cerr << "BACKEND_PROFILE " << name << " " << millis << "ms\n";
        std::cerr.flush();
        last_ = now;
    }

private:
    using Clock = std::chrono::steady_clock;
    bool enabled_;
    Clock::time_point last_;
};

std::string cleanAsmFunctionName(std::string name) {
    for (char &ch : name) {
        if (ch == '^') {
            ch = '$';
        }
    }
    if (name.rfind("__$", 0) == 0) {
        name = name.substr(3);
        size_t marker = name.find("__$");
        if (marker != std::string::npos) {
            name = name.substr(0, marker);
        }
    }
    return name;
}

std::string aarch64BranchMnemonic(const std::string &relop) {
    if (relop == ">") return "b.gt";
    if (relop == ">=") return "b.ge";
    if (relop == "<") return "b.lt";
    if (relop == "<=") return "b.le";
    if (relop == "==" || relop == "=") return "b.eq";
    if (relop == "!=") return "b.ne";
    return "b";
}

int alignUpInt(int value, int align) {
    if (align <= 1) return value;
    int rem = value % align;
    return rem == 0 ? value : value + align - rem;
}

class Aarch64StackEmitter {
public:
    std::string emit(quad::QuadProgram *program) {
        out_.str("");
        out_.clear();
        needsParallelRuntime_ = false;
        if (program == nullptr || program->quadFuncDeclList == nullptr) {
            return {};
        }
        collectFunctionSignatures(program);
        out_ << ".text\n";
        for (auto *func : *program->quadFuncDeclList) {
            if (func != nullptr) {
                emitFunction(func);
            }
        }
        if (needsParallelRuntime_) {
            emitParallelRuntime();
        }
        return out_.str();
    }

private:
    struct StackArg {
        quad::QuadTerm *term = nullptr;
        quad::QuadType type = quad::QuadType::INT;
        bool variadicDouble = false;
    };

    std::ostringstream out_;
    quad::QuadFuncDecl *func_ = nullptr;
    std::string funcName_;
    std::unordered_map<int, quad::QuadType> tempTypes_;
    std::unordered_map<std::string, std::vector<quad::QuadType>> functionParamTypes_;
    std::map<int, int> slots_;
    std::unordered_map<int, int> residentRegs_;
    std::vector<std::pair<int, int>> calleeSaveSlots_;
    std::vector<int> phiScratchSlots_;
    int frameSize_ = 0;
    int edgeLabelId_ = 0;
    bool needsParallelRuntime_ = false;

    static bool isRuntimeParallelSymbol(const std::string &name) {
        return name == "__sysy_parallel_for_range" ||
               name == "__sysy_parallel_reduce_int_range";
    }

    static bool isFloatHelper(const std::string &name) {
        return name == "__sysy_i2f_bits" || name == "__sysy_f2i_bits" ||
               name == "__sysy_fadd_bits" || name == "__sysy_fsub_bits" ||
               name == "__sysy_fmul_bits" || name == "__sysy_fdiv_bits" ||
               name == "__sysy_fneg_bits" || name == "__sysy_fcmpeq_bits" ||
               name == "__sysy_fcmpne_bits" || name == "__sysy_fcmplt_bits" ||
               name == "__sysy_fcmple_bits" || name == "__sysy_fcmpgt_bits" ||
               name == "__sysy_fcmpge_bits";
    }

    static bool isEncodedPutf(const std::string &name) {
        return name.rfind("putf$", 0) == 0;
    }

    static std::vector<char> encodedPutfSpecs(const std::string &name) {
        if (!isEncodedPutf(name)) {
            return {};
        }
        return std::vector<char>(name.begin() + 5, name.end());
    }

    void emitLine(const std::string &line) {
        if (!line.empty() && line.back() == ':') {
            out_ << line << "\n";
        } else {
            out_ << "\t" << line << "\n";
        }
    }

    std::string labelName(tree::Label *label) const {
        if (label == nullptr) {
            return funcName_ + "$L0";
        }
        return funcName_ + "$L" + std::to_string(label->num);
    }

    std::string freshEdgeLabel() {
        return funcName_ + "$A64edge" + std::to_string(edgeLabelId_++);
    }

    static quad::QuadTemp *termTemp(quad::QuadTerm *term) {
        if (term == nullptr || term->kind != quad::QuadTermKind::TEMP) {
            return nullptr;
        }
        return term->get_temp();
    }

    static bool sameLabel(tree::Label *left, tree::Label *right) {
        return left != nullptr && right != nullptr && left->num == right->num;
    }

    static bool isExitCallStmt(quad::QuadStm *stm) {
        if (stm == nullptr) {
            return false;
        }
        if (stm->kind == quad::QuadKind::EXTCALL) {
            auto *ext = dynamic_cast<quad::QuadExtCall *>(stm);
            return ext != nullptr && ext->extfun == "exit";
        }
        if (stm->kind == quad::QuadKind::MOVE_EXTCALL) {
            auto *moveExt = dynamic_cast<quad::QuadMoveExtCall *>(stm);
            return moveExt != nullptr && moveExt->extcall != nullptr &&
                   moveExt->extcall->extfun == "exit";
        }
        return false;
    }

    quad::QuadType mergeType(quad::QuadType oldType, quad::QuadType newType) const {
        if (oldType == quad::QuadType::PTR || newType == quad::QuadType::PTR) {
            return quad::QuadType::PTR;
        }
        if (oldType == quad::QuadType::FLOAT || newType == quad::QuadType::FLOAT) {
            return quad::QuadType::FLOAT;
        }
        return quad::QuadType::INT;
    }

    void noteTempType(quad::QuadTemp *temp) {
        if (temp == nullptr || temp->temp == nullptr) {
            return;
        }
        auto found = tempTypes_.find(temp->temp->num);
        if (found == tempTypes_.end()) {
            tempTypes_[temp->temp->num] = temp->type;
        } else {
            found->second = mergeType(found->second, temp->type);
        }
    }

    void noteTermType(quad::QuadTerm *term) {
        noteTempType(termTemp(term));
    }

    void selectResidentTemps(int tempSlotBytes) {
        residentRegs_.clear();
        calleeSaveSlots_.clear();
        phiScratchSlots_.clear();
        if (func_ == nullptr || func_->quadblocklist == nullptr) {
            frameSize_ = alignUpInt(tempSlotBytes, 16);
            return;
        }

        // A function-level assignment to callee-saved GPRs is deliberately
        // conservative: each selected temp owns one register for the entire
        // function.  Float values live here as their raw 32-bit representation.
        // This avoids liveness/interference mistakes and keeps values valid over
        // arbitrary calls.  Edge copies snapshot phi inputs before assigning
        // destinations, so phi-carried induction/reduction temps are eligible.
        std::unordered_map<int, std::size_t> blockIndex;
        std::size_t maxPhiCopies = 0;
        for (std::size_t i = 0; i < func_->quadblocklist->size(); ++i) {
            auto *block = func_->quadblocklist->at(i);
            if (block != nullptr && block->entry_label != nullptr) {
                blockIndex[block->entry_label->num] = i;
            }
            if (block == nullptr || block->quadlist == nullptr) continue;
            std::size_t phiCopies = 0;
            for (auto *stm : *block->quadlist) {
                if (stm == nullptr || stm->kind != quad::QuadKind::PHI) continue;
                ++phiCopies;
            }
            maxPhiCopies = std::max(maxPhiCopies, phiCopies);
        }

        // Structured lowering emits backward CFG edges for loops.  Weight the
        // blocks spanned by those edges so register residency favors dynamic hot
        // code, including generated parallel workers and their nested loops.
        std::vector<int> loopDepth(func_->quadblocklist->size(), 0);
        for (std::size_t i = 0; i < func_->quadblocklist->size(); ++i) {
            auto *block = func_->quadblocklist->at(i);
            if (block == nullptr || block->exit_labels == nullptr) continue;
            for (auto *target : *block->exit_labels) {
                if (target == nullptr) continue;
                auto found = blockIndex.find(target->num);
                if (found == blockIndex.end() || found->second > i) continue;
                for (std::size_t j = found->second; j <= i; ++j) ++loopDepth[j];
            }
        }

        struct Candidate {
            int temp = -1;
            int weighted = 0;
            int accesses = 0;
            bool inLoop = false;
            bool isParam = false;
            std::unordered_set<std::size_t> blocks;
        };
        std::unordered_map<int, Candidate> candidates;
        auto countTemp = [&](tree::Temp *temp, int weight, int depth,
                             std::size_t blockIndexValue) {
            if (temp == nullptr) return;
            auto &candidate = candidates[temp->num];
            candidate.temp = temp->num;
            candidate.weighted += weight;
            ++candidate.accesses;
            candidate.inLoop = candidate.inLoop || depth > 0;
            if (blockIndexValue < func_->quadblocklist->size()) {
                candidate.blocks.insert(blockIndexValue);
            }
        };
        if (func_->params != nullptr) {
            for (auto *param : *func_->params) {
                countTemp(param, 1, 0, func_->quadblocklist->size());
                if (param != nullptr) candidates[param->num].isParam = true;
            }
        }
        for (std::size_t i = 0; i < func_->quadblocklist->size(); ++i) {
            auto *block = func_->quadblocklist->at(i);
            if (block == nullptr || block->quadlist == nullptr) continue;
            int weight = loopDepth[i] >= 2 ? 64 : (loopDepth[i] == 1 ? 8 : 1);
            for (auto *stm : *block->quadlist) {
                if (stm == nullptr) continue;
                if (stm->def != nullptr) {
                    for (auto *temp : *stm->def) countTemp(temp, weight, loopDepth[i], i);
                }
                if (stm->use != nullptr) {
                    for (auto *temp : *stm->use) countTemp(temp, weight, loopDepth[i], i);
                }
            }
        }

        std::vector<Candidate> ranked;
        ranked.reserve(candidates.size());
        for (const auto &entry : candidates) {
            const Candidate &candidate = entry.second;
            if ((candidate.inLoop && candidate.accesses >= 2) || candidate.accesses >= 6) {
                ranked.push_back(candidate);
            }
        }
        std::sort(ranked.begin(), ranked.end(), [](const Candidate &left, const Candidate &right) {
            int leftScore = left.weighted + (left.blocks.size() > 1 ? 160 : 0) +
                            (left.isParam ? 128 : 0);
            int rightScore = right.weighted + (right.blocks.size() > 1 ? 160 : 0) +
                             (right.isParam ? 128 : 0);
            if (leftScore != rightScore) return leftScore > rightScore;
            if (left.accesses != right.accesses) return left.accesses > right.accesses;
            return left.temp < right.temp;
        });

        static constexpr int kCalleeSavedRegs[] = {19, 20, 21, 22, 23, 24, 25, 26, 27, 28};
        std::size_t count = std::min(ranked.size(), std::size(kCalleeSavedRegs));
        // x16/x17 are reserved by frame/address materialization, so phi edge
        // snapshots use only caller-scratch x12-x15 before spilling overflow.
        static constexpr std::size_t kPhiRegisterScratchCount = 4;
        int phiScratchBytes = static_cast<int>(
            maxPhiCopies > kPhiRegisterScratchCount ? maxPhiCopies - kPhiRegisterScratchCount : 0) * 8;
        for (int distance = tempSlotBytes + 8;
             distance <= tempSlotBytes + phiScratchBytes; distance += 8) {
            phiScratchSlots_.push_back(distance);
        }
        for (std::size_t i = 0; i < count; ++i) {
            int reg = kCalleeSavedRegs[i];
            residentRegs_[ranked[i].temp] = reg;
            calleeSaveSlots_.push_back(
                {reg, tempSlotBytes + phiScratchBytes + static_cast<int>(i + 1) * 8});
        }
        frameSize_ = alignUpInt(tempSlotBytes + phiScratchBytes + static_cast<int>(count) * 8, 16);
    }

    void collectTypesAndSlots() {
        tempTypes_.clear();
        slots_.clear();
        if (func_ == nullptr || func_->quadblocklist == nullptr) {
            return;
        }
        if (func_->params != nullptr) {
            for (auto *param : *func_->params) {
                if (param != nullptr && tempTypes_.find(param->num) == tempTypes_.end()) {
                    tempTypes_[param->num] = quad::QuadType::INT;
                }
            }
        }
        for (auto *block : *func_->quadblocklist) {
            if (block == nullptr || block->quadlist == nullptr) {
                continue;
            }
            for (auto *stm : *block->quadlist) {
                if (stm == nullptr) {
                    continue;
                }
                switch (stm->kind) {
                case quad::QuadKind::MOVE: {
                    auto *s = static_cast<quad::QuadMove *>(stm);
                    noteTempType(s->dst);
                    noteTermType(s->src);
                    break;
                }
                case quad::QuadKind::LOAD: {
                    auto *s = static_cast<quad::QuadLoad *>(stm);
                    noteTempType(s->dst);
                    noteTermType(s->src);
                    break;
                }
                case quad::QuadKind::STORE: {
                    auto *s = static_cast<quad::QuadStore *>(stm);
                    noteTermType(s->src);
                    noteTermType(s->dst);
                    break;
                }
                case quad::QuadKind::MOVE_BINOP: {
                    auto *s = static_cast<quad::QuadMoveBinop *>(stm);
                    noteTempType(s->dst);
                    noteTermType(s->left);
                    noteTermType(s->right);
                    break;
                }
                case quad::QuadKind::CALL: {
                    auto *s = static_cast<quad::QuadCall *>(stm);
                    noteTermType(s->obj_term);
                    if (s->args != nullptr) {
                        for (auto *arg : *s->args) noteTermType(arg);
                    }
                    break;
                }
                case quad::QuadKind::MOVE_CALL: {
                    auto *s = static_cast<quad::QuadMoveCall *>(stm);
                    noteTempType(s->dst);
                    if (s->call != nullptr) {
                        noteTermType(s->call->obj_term);
                        if (s->call->args != nullptr) {
                            for (auto *arg : *s->call->args) noteTermType(arg);
                        }
                    }
                    break;
                }
                case quad::QuadKind::EXTCALL: {
                    auto *s = static_cast<quad::QuadExtCall *>(stm);
                    if (s->args != nullptr) {
                        for (auto *arg : *s->args) noteTermType(arg);
                    }
                    break;
                }
                case quad::QuadKind::MOVE_EXTCALL: {
                    auto *s = static_cast<quad::QuadMoveExtCall *>(stm);
                    noteTempType(s->dst);
                    if (s->extcall != nullptr && s->extcall->args != nullptr) {
                        for (auto *arg : *s->extcall->args) noteTermType(arg);
                    }
                    break;
                }
                case quad::QuadKind::CJUMP: {
                    auto *s = static_cast<quad::QuadCJump *>(stm);
                    noteTermType(s->left);
                    noteTermType(s->right);
                    break;
                }
                case quad::QuadKind::PHI: {
                    auto *s = static_cast<quad::QuadPhi *>(stm);
                    noteTempType(s->temp_exp);
                    if (s->args != nullptr) {
                        for (auto &arg : *s->args) {
                            if (arg.first != nullptr) {
                                auto found = tempTypes_.find(arg.first->num);
                                quad::QuadType ty = s->temp_exp == nullptr ? quad::QuadType::INT
                                                                           : s->temp_exp->type;
                                if (found == tempTypes_.end()) {
                                    tempTypes_[arg.first->num] = ty;
                                }
                            }
                        }
                    }
                    break;
                }
                case quad::QuadKind::RETURN:
                    noteTermType(static_cast<quad::QuadReturn *>(stm)->exp);
                    break;
                case quad::QuadKind::PTR_CALC: {
                    auto *s = static_cast<quad::QuadPtrCalc *>(stm);
                    noteTermType(s->dst);
                    noteTermType(s->ptr);
                    noteTermType(s->offset);
                    break;
                }
                default:
                    break;
                }
            }
        }
        int offset = 0;
        for (const auto &entry : tempTypes_) {
            offset += 8;
            slots_[entry.first] = -offset;
        }
        selectResidentTemps(offset);
    }

    void collectFunctionSignatures(quad::QuadProgram *program) {
        functionParamTypes_.clear();
        if (program == nullptr || program->quadFuncDeclList == nullptr) {
            return;
        }
        auto *savedFunc = func_;
        std::string savedName = funcName_;
        auto savedTypes = tempTypes_;
        auto savedSlots = slots_;
        auto savedResidentRegs = residentRegs_;
        auto savedCalleeSaveSlots = calleeSaveSlots_;
        auto savedPhiScratchSlots = phiScratchSlots_;
        int savedFrameSize = frameSize_;

        for (auto *func : *program->quadFuncDeclList) {
            if (func == nullptr) {
                continue;
            }
            func_ = func;
            funcName_ = cleanAsmFunctionName(func->funcname);
            collectTypesAndSlots();
            std::vector<quad::QuadType> params;
            if (func->params != nullptr) {
                for (auto *param : *func->params) {
                    params.push_back(tempType(param));
                }
            }
            functionParamTypes_[funcName_] = std::move(params);
        }

        func_ = savedFunc;
        funcName_ = savedName;
        tempTypes_ = std::move(savedTypes);
        slots_ = std::move(savedSlots);
        residentRegs_ = std::move(savedResidentRegs);
        calleeSaveSlots_ = std::move(savedCalleeSaveSlots);
        phiScratchSlots_ = std::move(savedPhiScratchSlots);
        frameSize_ = savedFrameSize;
    }

    quad::QuadType tempType(tree::Temp *temp, quad::QuadType fallback = quad::QuadType::INT) const {
        if (temp == nullptr) {
            return fallback;
        }
        auto found = tempTypes_.find(temp->num);
        return found == tempTypes_.end() ? fallback : found->second;
    }

    quad::QuadType termType(quad::QuadTerm *term, quad::QuadType fallback = quad::QuadType::INT) const {
        if (term == nullptr) {
            return fallback;
        }
        if (term->kind == quad::QuadTermKind::NAME) {
            return quad::QuadType::PTR;
        }
        auto *qt = termTemp(term);
        if (qt == nullptr) {
            return fallback;
        }
        return tempType(qt->temp, qt->type);
    }

    quad::QuadType expectedArgType(const std::string &name, std::size_t index,
                                   quad::QuadType fallback) const {
        auto found = functionParamTypes_.find(cleanAsmFunctionName(name));
        if (found != functionParamTypes_.end() && index < found->second.size()) {
            return found->second[index];
        }
        if (name == "putfloat" && index == 0) return quad::QuadType::FLOAT;
        if ((name == "getarray" || name == "getfarray") && index == 0) return quad::QuadType::PTR;
        if ((name == "putarray" || name == "putfarray") && index == 1) return quad::QuadType::PTR;
        if (name == "free" && index == 0) return quad::QuadType::PTR;
        if (name == "memset" && index == 0) return quad::QuadType::PTR;
        if (name == "__sysy_parallel_for_range" || name == "__sysy_parallel_reduce_int_range") {
            if (index == 2 || index == 3) return quad::QuadType::PTR;
            if (index == 0 || index == 1 || index == 4) return quad::QuadType::INT;
        }
        return fallback;
    }

    void loadImm32(const std::string &reg, std::uint32_t value) {
        out_ << "\tmovz " << reg << ", #" << (value & 0xffffu) << "\n";
        std::uint32_t high = (value >> 16) & 0xffffu;
        if (high != 0) {
            out_ << "\tmovk " << reg << ", #" << high << ", lsl #16\n";
        }
    }

    void loadImm64(const std::string &reg, std::uint64_t value) {
        out_ << "\tmovz " << reg << ", #" << (value & 0xffffull) << "\n";
        for (int shift = 16; shift < 64; shift += 16) {
            std::uint64_t part = (value >> shift) & 0xffffull;
            if (part != 0) {
                out_ << "\tmovk " << reg << ", #" << part << ", lsl #" << shift << "\n";
            }
        }
    }

    void emitAddSubImm64(const std::string &op, const std::string &dst,
                         const std::string &base, int imm) {
        if (imm >= 0 && imm <= 4095) {
            out_ << "\t" << op << " " << dst << ", " << base << ", #" << imm << "\n";
        } else {
            loadImm64("x17", static_cast<std::uint64_t>(imm));
            out_ << "\t" << op << " " << dst << ", " << base << ", x17\n";
        }
    }

    void slotAddress(tree::Temp *temp, const std::string &addrReg = "x16") {
        int offset = slots_[temp->num];
        int distance = -offset;
        emitAddSubImm64("sub", addrReg, "x29", distance);
    }

    void stackAddressFromFrame(int offset, const std::string &addrReg = "x16") {
        emitAddSubImm64("add", addrReg, "x29", offset);
    }

    void loadTemp(tree::Temp *temp, quad::QuadType type, const std::string &reg) {
        auto resident = residentRegs_.find(temp->num);
        if (resident != residentRegs_.end()) {
            std::string source = (type == quad::QuadType::PTR ? "x" : "w") +
                                 std::to_string(resident->second);
            if (source != reg) out_ << "\tmov " << reg << ", " << source << "\n";
            return;
        }
        int distance = -slots_[temp->num];
        if (distance <= 256) {
            out_ << "\tldur " << reg << ", [x29, #-" << distance << "]\n";
            return;
        }
        slotAddress(temp);
        out_ << "\tldr " << reg << ", [x16]\n";
    }

    void storeTemp(tree::Temp *temp, quad::QuadType type, const std::string &reg) {
        auto resident = residentRegs_.find(temp->num);
        if (resident != residentRegs_.end()) {
            std::string destination = (type == quad::QuadType::PTR ? "x" : "w") +
                                      std::to_string(resident->second);
            if (destination != reg) out_ << "\tmov " << destination << ", " << reg << "\n";
            return;
        }
        int distance = -slots_[temp->num];
        if (distance <= 256) {
            out_ << "\tstur " << reg << ", [x29, #-" << distance << "]\n";
            return;
        }
        slotAddress(temp);
        out_ << "\tstr " << reg << ", [x16]\n";
    }

    void saveResidentRegisters() {
        for (std::size_t i = 0; i < calleeSaveSlots_.size();) {
            if (i + 1 < calleeSaveSlots_.size() &&
                calleeSaveSlots_[i + 1].second == calleeSaveSlots_[i].second + 8 &&
                calleeSaveSlots_[i + 1].second <= 512) {
                out_ << "\tstp x" << calleeSaveSlots_[i + 1].first << ", x"
                     << calleeSaveSlots_[i].first << ", [x29, #-"
                     << calleeSaveSlots_[i + 1].second << "]\n";
                i += 2;
                continue;
            }
            int reg = calleeSaveSlots_[i].first;
            int distance = calleeSaveSlots_[i].second;
            if (distance <= 256) {
                out_ << "\tstur x" << reg << ", [x29, #-" << distance << "]\n";
            } else {
                emitAddSubImm64("sub", "x16", "x29", distance);
                out_ << "\tstr x" << reg << ", [x16]\n";
            }
            ++i;
        }
    }

    void restoreResidentRegisters() {
        for (std::size_t end = calleeSaveSlots_.size(); end > 0;) {
            if (end >= 2 &&
                calleeSaveSlots_[end - 1].second == calleeSaveSlots_[end - 2].second + 8 &&
                calleeSaveSlots_[end - 1].second <= 512) {
                out_ << "\tldp x" << calleeSaveSlots_[end - 1].first << ", x"
                     << calleeSaveSlots_[end - 2].first << ", [x29, #-"
                     << calleeSaveSlots_[end - 1].second << "]\n";
                end -= 2;
                continue;
            }
            int reg = calleeSaveSlots_[end - 1].first;
            int distance = calleeSaveSlots_[end - 1].second;
            if (distance <= 256) {
                out_ << "\tldur x" << reg << ", [x29, #-" << distance << "]\n";
            } else {
                emitAddSubImm64("sub", "x16", "x29", distance);
                out_ << "\tldr x" << reg << ", [x16]\n";
            }
            --end;
        }
    }

    void storeRawFrameValue(int distance, quad::QuadType type, const std::string &reg) {
        if (distance <= 256) {
            out_ << "\tstur " << reg << ", [x29, #-" << distance << "]\n";
        } else {
            emitAddSubImm64("sub", "x16", "x29", distance);
            out_ << "\tstr " << reg << ", [x16]\n";
        }
    }

    void loadRawFrameValue(int distance, quad::QuadType type, const std::string &reg) {
        if (distance <= 256) {
            out_ << "\tldur " << reg << ", [x29, #-" << distance << "]\n";
        } else {
            emitAddSubImm64("sub", "x16", "x29", distance);
            out_ << "\tldr " << reg << ", [x16]\n";
        }
    }

    void loadAddress(const std::string &reg, const std::string &symbol) {
        out_ << "\tadrp " << reg << ", " << symbol << "\n";
        out_ << "\tadd " << reg << ", " << reg << ", :lo12:" << symbol << "\n";
    }

    void loadTerm(quad::QuadTerm *term, quad::QuadType type, const std::string &reg) {
        if (term == nullptr) {
            if (type == quad::QuadType::PTR) {
                out_ << "\tmov " << reg << ", xzr\n";
            } else {
                out_ << "\tmov " << reg << ", wzr\n";
            }
            return;
        }
        if (term->kind == quad::QuadTermKind::CONST) {
            if (type == quad::QuadType::PTR) {
                loadImm64(reg, static_cast<std::uint64_t>(static_cast<std::uint32_t>(term->get_const())));
            } else {
                loadImm32(reg, static_cast<std::uint32_t>(term->get_const()));
            }
            return;
        }
        if (term->kind == quad::QuadTermKind::NAME) {
            loadAddress(reg, term->get_name());
            return;
        }
        auto *qt = termTemp(term);
        if (qt == nullptr || qt->temp == nullptr) {
            return;
        }
        loadTemp(qt->temp, type, reg);
    }

    void storeTermToTemp(quad::QuadTemp *dst, quad::QuadTerm *src) {
        if (dst == nullptr || dst->temp == nullptr) {
            return;
        }
        quad::QuadType type = dst->type;
        if (type == quad::QuadType::PTR) {
            loadTerm(src, type, "x9");
            storeTemp(dst->temp, type, "x9");
        } else {
            loadTerm(src, type, "w9");
            storeTemp(dst->temp, type, "w9");
        }
    }

    void emitPrologueParams() {
        if (func_ == nullptr || func_->params == nullptr) {
            return;
        }
        int gp = 0;
        int fp = 0;
        int stackOffset = 16;
        for (auto *param : *func_->params) {
            if (param == nullptr) {
                continue;
            }
            quad::QuadType type = tempType(param);
            if (type == quad::QuadType::FLOAT) {
                if (fp < 8) {
                    out_ << "\tfmov w9, s" << fp << "\n";
                    storeTemp(param, type, "w9");
                    ++fp;
                } else {
                    stackAddressFromFrame(stackOffset);
                    out_ << "\tldr w9, [x16]\n";
                    storeTemp(param, type, "w9");
                    stackOffset += 8;
                }
            } else if (type == quad::QuadType::PTR) {
                if (gp < 8) {
                    storeTemp(param, type, "x" + std::to_string(gp));
                    ++gp;
                } else {
                    stackAddressFromFrame(stackOffset);
                    out_ << "\tldr x9, [x16]\n";
                    storeTemp(param, type, "x9");
                    stackOffset += 8;
                }
            } else {
                if (gp < 8) {
                    storeTemp(param, type, "w" + std::to_string(gp));
                    ++gp;
                } else {
                    stackAddressFromFrame(stackOffset);
                    out_ << "\tldr w9, [x16]\n";
                    storeTemp(param, type, "w9");
                    stackOffset += 8;
                }
            }
        }
    }

    void emitFunction(quad::QuadFuncDecl *func) {
        func_ = func;
        funcName_ = cleanAsmFunctionName(func->funcname);
        edgeLabelId_ = 0;
        collectTypesAndSlots();

        out_ << "\n.balign 4\n";
        out_ << ".global " << funcName_ << "\n";
        out_ << ".type " << funcName_ << ", %function\n";
        out_ << funcName_ << ":\n";
        emitLine("stp x29, x30, [sp, #-16]!");
        emitLine("mov x29, sp");
        if (frameSize_ > 0) {
            emitAddSubImm64("sub", "sp", "sp", frameSize_);
        }
        saveResidentRegisters();
        emitPrologueParams();

        if (func_->quadblocklist != nullptr) {
            for (auto *block : *func_->quadblocklist) {
                emitBlock(block);
            }
        }
        out_ << ".size " << funcName_ << ", .-" << funcName_ << "\n";
    }

    void emitEpilogue() {
        restoreResidentRegisters();
        emitLine("mov sp, x29");
        emitLine("ldp x29, x30, [sp], #16");
        emitLine("ret");
    }

    void emitBlock(quad::QuadBlock *block) {
        if (block == nullptr || block->entry_label == nullptr || block->quadlist == nullptr) {
            return;
        }
        out_ << labelName(block->entry_label) << ":\n";
        for (auto *stm : *block->quadlist) {
            if (stm == nullptr) {
                continue;
            }
            if (stm->kind == quad::QuadKind::LABEL || stm->kind == quad::QuadKind::PHI ||
                stm->kind == quad::QuadKind::JUMP || stm->kind == quad::QuadKind::CJUMP ||
                stm->kind == quad::QuadKind::RETURN) {
                continue;
            }
            emitStatement(stm);
        }

        quad::QuadStm *last = block->quadlist->empty() ? nullptr : block->quadlist->back();
        if (last == nullptr || isExitCallStmt(last)) {
            return;
        }
        if (last->kind == quad::QuadKind::RETURN) {
            emitReturn(static_cast<quad::QuadReturn *>(last));
            return;
        }
        if (last->kind == quad::QuadKind::JUMP) {
            auto *jump = static_cast<quad::QuadJump *>(last);
            emitPhiCopies(findBlock(jump->label), block->entry_label);
            emitLine("b " + labelName(jump->label));
            return;
        }
        if (last->kind == quad::QuadKind::CJUMP) {
            emitCJump(static_cast<quad::QuadCJump *>(last), block->entry_label);
            return;
        }
        if (block->exit_labels != nullptr && block->exit_labels->size() == 1) {
            tree::Label *target = block->exit_labels->front();
            emitPhiCopies(findBlock(target), block->entry_label);
            emitLine("b " + labelName(target));
        }
    }

    quad::QuadBlock *findBlock(tree::Label *label) {
        if (label == nullptr || func_ == nullptr || func_->quadblocklist == nullptr) {
            return nullptr;
        }
        for (auto *block : *func_->quadblocklist) {
            if (block != nullptr && block->entry_label != nullptr &&
                block->entry_label->num == label->num) {
                return block;
            }
        }
        return nullptr;
    }

    void emitPhiCopies(quad::QuadBlock *target, tree::Label *fromLabel) {
        if (target == nullptr || target->quadlist == nullptr || fromLabel == nullptr) {
            return;
        }
        struct PhiCopy {
            quad::QuadTemp *dst = nullptr;
            tree::Temp *src = nullptr;
            quad::QuadType type = quad::QuadType::INT;
        };
        std::vector<PhiCopy> copies;
        for (auto *stm : *target->quadlist) {
            if (stm == nullptr || stm->kind == quad::QuadKind::LABEL) {
                continue;
            }
            if (stm->kind != quad::QuadKind::PHI) {
                break;
            }
            auto *phi = static_cast<quad::QuadPhi *>(stm);
            if (phi->temp_exp == nullptr || phi->args == nullptr) {
                continue;
            }
            for (auto &arg : *phi->args) {
                if (arg.first != nullptr && sameLabel(arg.second, fromLabel)) {
                    copies.push_back(PhiCopy{phi->temp_exp, arg.first, phi->temp_exp->type});
                    break;
                }
            }
        }
        static constexpr int kPhiScratchRegs[] = {12, 13, 14, 15};
        for (std::size_t i = 0; i < copies.size(); ++i) {
            const auto &copy = copies[i];
            quad::QuadTerm src(new quad::QuadTemp(copy.src, copy.type));
            std::string width = copy.type == quad::QuadType::PTR ? "x" : "w";
            if (i < std::size(kPhiScratchRegs)) {
                loadTerm(&src, copy.type, width + std::to_string(kPhiScratchRegs[i]));
            } else {
                loadTerm(&src, copy.type, width + "9");
                storeRawFrameValue(phiScratchSlots_[i - std::size(kPhiScratchRegs)], copy.type,
                                   width + "9");
            }
        }
        for (std::size_t i = 0; i < copies.size(); ++i) {
            const auto &copy = copies[i];
            std::string width = copy.type == quad::QuadType::PTR ? "x" : "w";
            if (i < std::size(kPhiScratchRegs)) {
                storeTemp(copy.dst->temp, copy.type,
                          width + std::to_string(kPhiScratchRegs[i]));
            } else {
                loadRawFrameValue(phiScratchSlots_[i - std::size(kPhiScratchRegs)], copy.type,
                                  width + "9");
                storeTemp(copy.dst->temp, copy.type, width + "9");
            }
        }
    }

    void emitCJump(quad::QuadCJump *cjump, tree::Label *fromLabel) {
        if (cjump == nullptr) {
            return;
        }
        quad::QuadType leftType = termType(cjump->left);
        quad::QuadType rightType = termType(cjump->right);
        bool pointerCompare = leftType == quad::QuadType::PTR ||
                              rightType == quad::QuadType::PTR;
        if (pointerCompare) {
            if (leftType == quad::QuadType::PTR) {
                loadTerm(cjump->left, leftType, "x9");
            } else {
                loadTerm(cjump->left, leftType, "w9");
                emitLine("uxtw x9, w9");
            }
            if (rightType == quad::QuadType::PTR) {
                loadTerm(cjump->right, rightType, "x10");
            } else {
                loadTerm(cjump->right, rightType, "w10");
                emitLine("uxtw x10, w10");
            }
        } else {
            loadTerm(cjump->left, leftType, "w9");
            loadTerm(cjump->right, rightType, "w10");
        }
        emitLine(pointerCompare ? "cmp x9, x10" : "cmp w9, w10");
        std::string trueEdge = freshEdgeLabel();
        std::string doneEdge = freshEdgeLabel();
        emitLine(aarch64BranchMnemonic(cjump->relop) + " " + trueEdge);
        emitPhiCopies(findBlock(cjump->f), fromLabel);
        emitLine("b " + labelName(cjump->f));
        out_ << trueEdge << ":\n";
        emitPhiCopies(findBlock(cjump->t), fromLabel);
        emitLine("b " + labelName(cjump->t));
        out_ << doneEdge << ":\n";
    }

    void emitReturn(quad::QuadReturn *ret) {
        if (ret != nullptr && ret->exp != nullptr) {
            // Constants do not carry a QuadType, so the function's declared
            // return type is the authoritative ABI type here.  In particular,
            // a float literal such as `return 0.0` must be returned in s0, not
            // w0 with a stale value left in s0.
            quad::QuadType type = func_ == nullptr ? termType(ret->exp)
                                                   : func_->return_type;
            if (type == quad::QuadType::FLOAT) {
                loadTerm(ret->exp, type, "w9");
                emitLine("fmov s0, w9");
            } else if (type == quad::QuadType::PTR) {
                loadTerm(ret->exp, type, "x0");
            } else {
                loadTerm(ret->exp, type, "w0");
            }
        }
        emitEpilogue();
    }

    void emitStatement(quad::QuadStm *stm) {
        switch (stm->kind) {
        case quad::QuadKind::MOVE:
            storeTermToTemp(static_cast<quad::QuadMove *>(stm)->dst,
                            static_cast<quad::QuadMove *>(stm)->src);
            break;
        case quad::QuadKind::LOAD:
            emitLoad(static_cast<quad::QuadLoad *>(stm));
            break;
        case quad::QuadKind::STORE:
            emitStore(static_cast<quad::QuadStore *>(stm));
            break;
        case quad::QuadKind::MOVE_BINOP:
            emitBinop(static_cast<quad::QuadMoveBinop *>(stm));
            break;
        case quad::QuadKind::PTR_CALC:
            emitPtrCalc(static_cast<quad::QuadPtrCalc *>(stm));
            break;
        case quad::QuadKind::CALL:
            emitCall(static_cast<quad::QuadCall *>(stm), nullptr, quad::QuadType::INT);
            break;
        case quad::QuadKind::MOVE_CALL: {
            auto *s = static_cast<quad::QuadMoveCall *>(stm);
            emitCall(s->call, s->dst, s->dst == nullptr ? quad::QuadType::INT : s->dst->type);
            break;
        }
        case quad::QuadKind::EXTCALL:
            emitExtCall(static_cast<quad::QuadExtCall *>(stm), nullptr, quad::QuadType::INT);
            break;
        case quad::QuadKind::MOVE_EXTCALL: {
            auto *s = static_cast<quad::QuadMoveExtCall *>(stm);
            emitExtCall(s->extcall, s->dst, s->dst == nullptr ? quad::QuadType::INT : s->dst->type);
            break;
        }
        default:
            break;
        }
    }

    void emitLoad(quad::QuadLoad *load) {
        if (load == nullptr || load->dst == nullptr || load->dst->temp == nullptr) {
            return;
        }
        loadTerm(load->src, quad::QuadType::PTR, "x9");
        if (load->dst->type == quad::QuadType::PTR) {
            emitLine("ldr x10, [x9]");
            storeTemp(load->dst->temp, load->dst->type, "x10");
        } else {
            emitLine("ldr w10, [x9]");
            storeTemp(load->dst->temp, load->dst->type, "w10");
        }
    }

    void emitStore(quad::QuadStore *store) {
        if (store == nullptr) {
            return;
        }
        quad::QuadType type = termType(store->src);
        loadTerm(store->dst, quad::QuadType::PTR, "x10");
        if (type == quad::QuadType::PTR) {
            loadTerm(store->src, type, "x9");
            emitLine("str x9, [x10]");
        } else {
            loadTerm(store->src, type, "w9");
            emitLine("str w9, [x10]");
        }
    }

    void emitBinop(quad::QuadMoveBinop *binop) {
        if (binop == nullptr || binop->dst == nullptr || binop->dst->temp == nullptr) {
            return;
        }
        loadTerm(binop->left, quad::QuadType::INT, "w9");
        loadTerm(binop->right, quad::QuadType::INT, "w10");
        if (binop->binop == "+") {
            emitLine("add w11, w9, w10");
        } else if (binop->binop == "-") {
            emitLine("sub w11, w9, w10");
        } else if (binop->binop == "*") {
            emitLine("mul w11, w9, w10");
        } else if (binop->binop == "/") {
            emitLine("sdiv w11, w9, w10");
        } else if (binop->binop == "xor") {
            emitLine("eor w11, w9, w10");
        } else {
            emitLine("add w11, w9, w10");
        }
        storeTemp(binop->dst->temp, binop->dst->type, "w11");
    }

    void emitPtrCalc(quad::QuadPtrCalc *ptrCalc) {
        if (ptrCalc == nullptr || ptrCalc->dst == nullptr) {
            return;
        }
        auto *dst = termTemp(ptrCalc->dst);
        if (dst == nullptr || dst->temp == nullptr) {
            return;
        }
        loadTerm(ptrCalc->ptr, quad::QuadType::PTR, "x9");
        loadTerm(ptrCalc->offset, quad::QuadType::INT, "w10");
        emitLine("sxtw x10, w10");
        emitLine("add x11, x9, x10");
        storeTemp(dst->temp, quad::QuadType::PTR, "x11");
    }

    void moveArgToRegister(quad::QuadTerm *arg, quad::QuadType type, int index) {
        if (type == quad::QuadType::FLOAT) {
            loadTerm(arg, type, "w9");
            emitLine("fmov s" + std::to_string(index) + ", w9");
        } else if (type == quad::QuadType::PTR) {
            loadTerm(arg, type, "x" + std::to_string(index));
        } else {
            loadTerm(arg, type, "w" + std::to_string(index));
        }
    }

    void storeStackArg(int offset, const StackArg &arg) {
        if (arg.variadicDouble) {
            loadTerm(arg.term, quad::QuadType::FLOAT, "w9");
            emitLine("fmov s31, w9");
            emitLine("fcvt d31, s31");
            emitLine("str d31, [sp, #" + std::to_string(offset) + "]");
            return;
        }
        if (arg.type == quad::QuadType::PTR) {
            loadTerm(arg.term, arg.type, "x9");
            emitLine("str x9, [sp, #" + std::to_string(offset) + "]");
        } else {
            loadTerm(arg.term, arg.type, "w9");
            emitLine("str w9, [sp, #" + std::to_string(offset) + "]");
        }
    }

    void emitCall(quad::QuadCall *call, quad::QuadTemp *dst, quad::QuadType dstType) {
        if (call == nullptr) {
            return;
        }
        if (call->obj_term != nullptr) {
            loadTerm(call->obj_term, quad::QuadType::PTR, "x15");
        }
        emitPreparedCall(call->obj_term == nullptr ? call->name : "", call->args,
                         dst, dstType, call->obj_term != nullptr);
    }

    void emitExtCall(quad::QuadExtCall *call, quad::QuadTemp *dst, quad::QuadType dstType) {
        if (call == nullptr) {
            return;
        }
        if (isFloatHelper(call->extfun)) {
            emitFloatHelper(call->extfun, call->args, dst);
            return;
        }
        if (isEncodedPutf(call->extfun)) {
            emitPutf(call, dst);
            return;
        }
        if (isRuntimeParallelSymbol(call->extfun)) {
            needsParallelRuntime_ = true;
        }
        emitPreparedCall(call->extfun, call->args, dst, dstType, false);
    }

    void emitPreparedCall(const std::string &name, std::vector<quad::QuadTerm *> *args,
                          quad::QuadTemp *dst, quad::QuadType dstType, bool indirect) {
        int gp = 0;
        int fp = 0;
        std::vector<StackArg> stackArgs;
        if (args != nullptr) {
            for (std::size_t i = 0; i < args->size(); ++i) {
                auto *arg = args->at(i);
                quad::QuadType type = indirect ? termType(arg)
                                                : expectedArgType(name, i, termType(arg));
                if (type == quad::QuadType::FLOAT) {
                    if (fp < 8) {
                        moveArgToRegister(arg, type, fp++);
                    } else {
                        stackArgs.push_back(StackArg{arg, type, false});
                    }
                } else {
                    if (gp < 8) {
                        moveArgToRegister(arg, type, gp++);
                    } else {
                        stackArgs.push_back(StackArg{arg, type, false});
                    }
                }
            }
        }
        int stackBytes = alignUpInt(static_cast<int>(stackArgs.size()) * 8, 16);
        if (stackBytes > 0) {
            emitAddSubImm64("sub", "sp", "sp", stackBytes);
            for (std::size_t i = 0; i < stackArgs.size(); ++i) {
                storeStackArg(static_cast<int>(i) * 8, stackArgs[i]);
            }
        }
        if (indirect) {
            emitLine("blr x15");
        } else {
            emitLine("bl " + name);
        }
        if (stackBytes > 0) {
            emitAddSubImm64("add", "sp", "sp", stackBytes);
        }
        if (dst != nullptr && dst->temp != nullptr) {
            if (dstType == quad::QuadType::FLOAT) {
                emitLine("fmov w9, s0");
                storeTemp(dst->temp, dstType, "w9");
            } else if (dstType == quad::QuadType::PTR) {
                storeTemp(dst->temp, dstType, "x0");
            } else {
                storeTemp(dst->temp, dstType, "w0");
            }
        }
    }

    void emitFloatHelper(const std::string &name, std::vector<quad::QuadTerm *> *args,
                         quad::QuadTemp *dst) {
        auto argAt = [&](std::size_t i) -> quad::QuadTerm * {
            return args != nullptr && i < args->size() ? args->at(i) : nullptr;
        };
        if (name == "__sysy_i2f_bits") {
            loadTerm(argAt(0), quad::QuadType::INT, "w9");
            emitLine("scvtf s0, w9");
            emitLine("fmov w10, s0");
        } else if (name == "__sysy_f2i_bits") {
            loadTerm(argAt(0), quad::QuadType::FLOAT, "w9");
            emitLine("fmov s0, w9");
            emitLine("fcvtzs w10, s0");
        } else if (name == "__sysy_fneg_bits") {
            loadTerm(argAt(0), quad::QuadType::FLOAT, "w9");
            emitLine("fmov s0, w9");
            emitLine("fneg s0, s0");
            emitLine("fmov w10, s0");
        } else {
            loadTerm(argAt(0), quad::QuadType::FLOAT, "w9");
            loadTerm(argAt(1), quad::QuadType::FLOAT, "w10");
            emitLine("fmov s0, w9");
            emitLine("fmov s1, w10");
            if (name == "__sysy_fadd_bits") {
                emitLine("fadd s0, s0, s1");
                emitLine("fmov w10, s0");
            } else if (name == "__sysy_fsub_bits") {
                emitLine("fsub s0, s0, s1");
                emitLine("fmov w10, s0");
            } else if (name == "__sysy_fmul_bits") {
                emitLine("fmul s0, s0, s1");
                emitLine("fmov w10, s0");
            } else if (name == "__sysy_fdiv_bits") {
                emitLine("fdiv s0, s0, s1");
                emitLine("fmov w10, s0");
            } else {
                emitLine("fcmp s0, s1");
                if (name == "__sysy_fcmpeq_bits") {
                    emitLine("cset w10, eq");
                } else if (name == "__sysy_fcmpne_bits") {
                    emitLine("cset w10, ne");
                } else if (name == "__sysy_fcmplt_bits") {
                    emitLine("cset w10, mi");
                } else if (name == "__sysy_fcmple_bits") {
                    emitLine("cset w10, ls");
                } else if (name == "__sysy_fcmpgt_bits") {
                    emitLine("cset w10, gt");
                } else if (name == "__sysy_fcmpge_bits") {
                    emitLine("cset w10, ge");
                } else {
                    emitLine("mov w10, wzr");
                }
            }
        }
        if (dst != nullptr && dst->temp != nullptr) {
            storeTemp(dst->temp, dst->type, "w10");
        }
    }

    void emitPutf(quad::QuadExtCall *call, quad::QuadTemp *dst) {
        std::vector<char> specs = encodedPutfSpecs(call->extfun);
        auto *args = call->args;
        if (args == nullptr || args->empty()) {
            emitLine("bl putf");
            return;
        }
        loadTerm(args->at(0), quad::QuadType::PTR, "x0");
        int gp = 1;
        int fp = 0;
        std::vector<StackArg> stackArgs;
        for (std::size_t i = 0; i < specs.size() && i + 1 < args->size(); ++i) {
            auto *arg = args->at(i + 1);
            if (specs[i] == 'f') {
                if (fp < 8) {
                    loadTerm(arg, quad::QuadType::FLOAT, "w9");
                    emitLine("fmov s" + std::to_string(fp) + ", w9");
                    emitLine("fcvt d" + std::to_string(fp) + ", s" + std::to_string(fp));
                    ++fp;
                } else {
                    stackArgs.push_back(StackArg{arg, quad::QuadType::FLOAT, true});
                }
            } else {
                if (gp < 8) {
                    loadTerm(arg, quad::QuadType::INT, "w" + std::to_string(gp));
                    ++gp;
                } else {
                    stackArgs.push_back(StackArg{arg, quad::QuadType::INT, false});
                }
            }
        }
        int stackBytes = alignUpInt(static_cast<int>(stackArgs.size()) * 8, 16);
        if (stackBytes > 0) {
            emitAddSubImm64("sub", "sp", "sp", stackBytes);
            for (std::size_t i = 0; i < stackArgs.size(); ++i) {
                storeStackArg(static_cast<int>(i) * 8, stackArgs[i]);
            }
        }
        emitLine("bl putf");
        if (stackBytes > 0) {
            emitAddSubImm64("add", "sp", "sp", stackBytes);
        }
        if (dst != nullptr && dst->temp != nullptr) {
            storeTemp(dst->temp, dst->type, "w0");
        }
    }

    void emitParallelRuntime() {
        out_ << R"(
.text
.balign 4
.type __sysy_parallel_for_entry, %function
__sysy_parallel_for_entry:
	stp x29, x30, [sp, #-16]!
	mov x29, sp
	mov x4, x0
	ldr w0, [x4, #0]
	ldr w1, [x4, #4]
	ldr x2, [x4, #8]
	ldr x3, [x4, #16]
	blr x3
	mov x0, xzr
	ldp x29, x30, [sp], #16
	ret
.size __sysy_parallel_for_entry, .-__sysy_parallel_for_entry

.balign 4
.type __sysy_parallel_reduce_int_entry, %function
__sysy_parallel_reduce_int_entry:
	stp x29, x30, [sp, #-16]!
	mov x29, sp
	mov x4, x0
	ldr w0, [x4, #0]
	ldr w1, [x4, #4]
	ldr x2, [x4, #8]
	ldr x3, [x4, #16]
	blr x3
	str w0, [x4, #24]
	mov x0, xzr
	ldp x29, x30, [sp], #16
	ret
.size __sysy_parallel_reduce_int_entry, .-__sysy_parallel_reduce_int_entry

.balign 4
.global __sysy_parallel_for_range
.type __sysy_parallel_for_range, %function
__sysy_parallel_for_range:
	stp x29, x30, [sp, #-16]!
	mov x29, sp
	stp x19, x20, [sp, #-16]!
	stp x21, x22, [sp, #-16]!
	stp x23, x24, [sp, #-16]!
	sub sp, sp, #48
	mov w19, w0
	mov w20, w1
	mov x21, x2
	mov x22, x3
	sub w9, w20, w19
	cmp w9, #1
	blt .Lsysy_parallel_for_direct
	cmp w4, #1
	blt .Lsysy_parallel_for_direct
	umull x10, w9, w4
	movz x11, #16384
	cmp x10, x11
	blt .Lsysy_parallel_for_direct
	asr w23, w9, #1
	add w23, w19, w23
	add x24, sp, #0
	str w23, [x24, #0]
	str w20, [x24, #4]
	str x21, [x24, #8]
	str x22, [x24, #16]
	add x0, sp, #32
	mov x1, xzr
	adrp x2, __sysy_parallel_for_entry
	add x2, x2, :lo12:__sysy_parallel_for_entry
	mov x3, x24
	bl pthread_create
	cbnz w0, .Lsysy_parallel_for_direct
	mov w0, w19
	mov w1, w23
	mov x2, x21
	mov x3, x22
	blr x3
	ldr x0, [sp, #32]
	mov x1, xzr
	bl pthread_join
	b .Lsysy_parallel_for_done
.Lsysy_parallel_for_direct:
	mov w0, w19
	mov w1, w20
	mov x2, x21
	mov x3, x22
	blr x3
.Lsysy_parallel_for_done:
	add sp, sp, #48
	ldp x23, x24, [sp], #16
	ldp x21, x22, [sp], #16
	ldp x19, x20, [sp], #16
	ldp x29, x30, [sp], #16
	ret
.size __sysy_parallel_for_range, .-__sysy_parallel_for_range

.balign 4
.global __sysy_parallel_reduce_int_range
.type __sysy_parallel_reduce_int_range, %function
__sysy_parallel_reduce_int_range:
	stp x29, x30, [sp, #-16]!
	mov x29, sp
	stp x19, x20, [sp, #-16]!
	stp x21, x22, [sp, #-16]!
	stp x23, x24, [sp, #-16]!
	sub sp, sp, #48
	mov w19, w0
	mov w20, w1
	mov x21, x2
	mov x22, x3
	sub w9, w20, w19
	cmp w9, #1
	blt .Lsysy_parallel_reduce_direct
	cmp w4, #1
	blt .Lsysy_parallel_reduce_direct
	umull x10, w9, w4
	movz x11, #16384
	cmp x10, x11
	blt .Lsysy_parallel_reduce_direct
	asr w23, w9, #1
	add w23, w19, w23
	add x24, sp, #0
	str w23, [x24, #0]
	str w20, [x24, #4]
	str x21, [x24, #8]
	str x22, [x24, #16]
	str wzr, [x24, #24]
	add x0, sp, #32
	mov x1, xzr
	adrp x2, __sysy_parallel_reduce_int_entry
	add x2, x2, :lo12:__sysy_parallel_reduce_int_entry
	mov x3, x24
	bl pthread_create
	cbnz w0, .Lsysy_parallel_reduce_direct
	mov w0, w19
	mov w1, w23
	mov x2, x21
	mov x3, x22
	blr x3
	mov w19, w0
	ldr x0, [sp, #32]
	mov x1, xzr
	bl pthread_join
	ldr w0, [sp, #24]
	add w0, w19, w0
	b .Lsysy_parallel_reduce_done
.Lsysy_parallel_reduce_direct:
	mov w0, w19
	mov w1, w20
	mov x2, x21
	mov x3, x22
	blr x3
.Lsysy_parallel_reduce_done:
	add sp, sp, #48
	ldp x23, x24, [sp], #16
	ldp x21, x22, [sp], #16
	ldp x19, x20, [sp], #16
	ldp x29, x30, [sp], #16
	ret
.size __sysy_parallel_reduce_int_range, .-__sysy_parallel_reduce_int_range
)";
    }
};

} // namespace

OptMode optModeFromCompilerFlag(const std::string &flag) {
    if (flag == "-O0" || flag == "none" || flag == "no" || flag == "noopt") {
        return OptMode::None;
    }
    if (flag == "const" || flag == "sccp") {
        return OptMode::Const;
    }
    if (flag == "loop1" || flag == "licm") {
        return OptMode::Loop1;
    }
    if (flag == "loop2" || flag == "iv" || flag == "strength") {
        return OptMode::Loop2;
    }
    if (flag == "allloop" || flag == "loops") {
        return OptMode::AllLoop;
    }
    return OptMode::AllOpt;
}

BackendResult compileTreeToAarch64(tree::Program *program, const BackendOptions &options) {
    ProfileTimer profile;
    BackendResult result;
    if (program == nullptr) {
        result.error = "missing Tree IR program";
        return result;
    }

    tree::Program *canonIr = canon(program);
    profile.mark("canon");
    quad::QuadProgram *quadProgram = tree2quad(canonIr);
    if (quadProgram == nullptr) {
        result.error = "IR to Quad failed";
        return result;
    }
    profile.mark("tree2quad");
    maybeWriteQuad(options, ".4.quad", quadProgram);

    quad::QuadProgram *blocked = blocking(quadProgram);
    if (blocked == nullptr) {
        result.error = "Quad blocking failed";
        return result;
    }
    profile.mark("blocking");
    maybeWriteQuad(options, ".4-block.quad", blocked);

    auto *blockedFlow = computeFlow(blocked);
    if (blockedFlow == nullptr) {
        result.error = "flow analysis failed";
        return result;
    }
    profile.mark("flow");

    quad::QuadProgram *ssa = quad2ssa(blockedFlow);
    if (ssa == nullptr) {
        result.error = "Quad to SSA failed";
        return result;
    }
    profile.mark("ssa");
    maybeWriteQuad(options, ".4-ssa.quad", ssa);

    quad::QuadProgram *optimizedSsa = ssa;
    if (options.optMode == OptMode::None) {
        refreshQuadExtents(optimizedSsa);
        profile.mark("refresh");
    }
    if (optModeUsesSccp(options.optMode)) {
        optimizedSsa = optProg(optimizedSsa);
        if (optimizedSsa == nullptr) {
            result.error = "SCCP optimization failed";
            return result;
        }
        refreshQuadExtents(optimizedSsa);
        profile.mark("sccp");
    }
    if (optModeUsesSccp(options.optMode) && optimizationPassEnabled("algebrasimp")) {
        optimizedSsa = runAlgebraSimpPass(optimizedSsa);
        if (optimizedSsa == nullptr) {
            result.error = "AlgebraSimp optimization failed";
            return result;
        }
        profile.mark("algebrasimp");
    }
    if (optModeUsesSccp(options.optMode) && optimizationPassEnabled("memopt")) {
        optimizedSsa = runMemOptPass(optimizedSsa);
        if (optimizedSsa == nullptr) {
            result.error = "MemOpt optimization failed";
            return result;
        }
        profile.mark("memopt");
    }

    if (optModeUsesSccp(options.optMode) && optimizationPassEnabled("inline")) {
        optimizedSsa = runInlinePass(optimizedSsa);
        if (optimizedSsa == nullptr) {
            result.error = "Inlining optimization failed";
            return result;
        }
        profile.mark("inline");
    }

    if (optModeUsesSccp(options.optMode) && optimizationPassEnabled("funcspec")) {
        optimizedSsa = runFuncSpecPass(optimizedSsa);
        if (optimizedSsa == nullptr) {
            result.error = "FuncSpec optimization failed";
            return result;
        }
        profile.mark("funcspec");
    }

    if (optModeUsesSccp(options.optMode) && optimizationPassEnabled("memopt")) {
        optimizedSsa = runMemOptPass(optimizedSsa);
        if (optimizedSsa == nullptr) {
            result.error = "MemOpt round 2 failed";
            return result;
        }
        profile.mark("memopt-r2");
    }
    if (optModeUsesSccp(options.optMode) && optimizationPassEnabled("algebrasimp")) {
        optimizedSsa = runAlgebraSimpPass(optimizedSsa);
        if (optimizedSsa == nullptr) {
            result.error = "AlgebraSimp round 2 failed";
            return result;
        }
        profile.mark("algebrasimp-r2");
    }
    if (optModeUsesLicm(options.optMode)) {
        optimizedSsa = runLicmPass(optimizedSsa);
        if (optimizedSsa == nullptr) {
            result.error = "LICM optimization failed";
            return result;
        }
        profile.mark("licm");
    }
    if (optModeUsesIv(options.optMode)) {
        optimizedSsa = runIvPass(optimizedSsa);
        if (optimizedSsa == nullptr) {
            result.error = "loop induction optimization failed";
            return result;
        }
        profile.mark("iv");
    }
    // Run SSA substitution passes after loop transforms. Besides keeping their
    // rebuilt metadata fresh for final flow/RA, this prevents value-numbering
    // simplifications from changing the pattern language consumed by the
    // legacy induction pass.
    if (optModeUsesGvn(options.optMode) && optimizationPassEnabled("gvn")) {
        optimizedSsa = runGvnPass(optimizedSsa);
        if (optimizedSsa == nullptr) {
            result.error = "GVN optimization failed";
            return result;
        }
        profile.mark("gvn");
    }
    if (optModeUsesSccp(options.optMode) && optimizationPassEnabled("copyprop")) {
        optimizedSsa = runCopyPropPass(optimizedSsa);
        if (optimizedSsa == nullptr) {
            result.error = "CopyProp optimization failed";
            return result;
        }
        profile.mark("copyprop");
    }
    maybeWriteQuad(options, ".4-ssa-final.quad", optimizedSsa);

    quad::QuadProgram *ssaProgramForInstr = optimizedSsa;
    if (options.optMode != OptMode::None) {
        auto *optimizedFlow = computeFlow(optimizedSsa);
        if (optimizedFlow == nullptr) {
            result.error = "optimized flow analysis failed";
            return result;
        }
        profile.mark("optimized-flow");

        auto *funcList = new std::vector<quad::QuadFuncDecl *>();
        int lastLabel = optimizedSsa->last_label_num;
        int lastTemp = optimizedSsa->last_temp_num;
        for (auto *ffi : *optimizedFlow) {
            if (ffi == nullptr || ffi->cfi == nullptr || ffi->cfi->func == nullptr) {
                continue;
            }
            funcList->push_back(ffi->cfi->func);
            if (ffi->programLastLabelNum >= 0) {
                lastLabel = ffi->programLastLabelNum;
            }
            if (ffi->programLastTempNum >= 0) {
                lastTemp = ffi->programLastTempNum;
            }
        }
        ssaProgramForInstr = new quad::QuadProgram(funcList, lastLabel, lastTemp);
    } else {
        profile.mark("optimized-flow-skipped");
    }

    Aarch64StackEmitter emitter;
    result.ok = true;
    result.assembly = emitter.emit(ssaProgramForInstr);
    profile.mark("aarch64-stack-asm");
    return result;
}

} // namespace backend
