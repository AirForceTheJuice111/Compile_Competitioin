#include "backend_driver.hh"

#include "advDFG.hh"
#include "asmprog.hh"
#include "asmprogpass.hh"
#include "blocking.hh"
#include "canon.hh"
#include "coloring.hh"
#include "flowinfo.hh"
#include "ig.hh"
#include "instrSelection.hh"
#include "loopheader.hh"
#include "loopinductionopt.hh"
#include "looplicm.hh"
#include "opt.hh"
#include "preSchedule.hh"
#include "quad.hh"
#include "quadssa.hh"
#include "schedule.hh"
#include "tree2quad.hh"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
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

bool optModeUsesLicm(OptMode mode) {
    return mode == OptMode::Loop1 || mode == OptMode::AllLoop || mode == OptMode::AllOpt;
}

bool optModeUsesIv(OptMode mode) {
    return mode == OptMode::Loop2 || mode == OptMode::AllLoop || mode == OptMode::AllOpt;
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

instr::AsmProg *scheduleToAsmProg(const instr::ScheduleProg *schedule) {
    auto *program = new instr::AsmProg();
    if (schedule == nullptr) {
        return program;
    }

    for (auto *funcSchedule : schedule->funcSchedules) {
        if (funcSchedule == nullptr || funcSchedule->quadFunc == nullptr) {
            continue;
        }
        instr::AsmFunction func(funcSchedule->quadFunc->funcname);
        for (const auto &ins : funcSchedule->linearizedInstructions.instrs) {
            func.instructions.push_back(ins);
        }
        program->functions.push_back(func);
    }
    return program;
}

void maybeWriteQuad(const BackendOptions &options, const std::string &suffix, quad::QuadProgram *program) {
    if (!options.emitDebugFiles || options.debugBase.empty() || program == nullptr) {
        return;
    }
    std::string text;
    program->print(text, 0, true);
    writeText(options.debugBase + suffix, text);
}

std::string tempListText(const std::vector<tree::Temp *> &temps) {
    std::ostringstream out;
    bool first = true;
    for (auto *temp : temps) {
        if (!first) {
            out << ", ";
        }
        if (temp == nullptr) {
            out << "null";
        } else {
            out << "t" << temp->num;
        }
        first = false;
    }
    return out.str();
}

std::string labelListText(const std::vector<tree::Label *> &labels) {
    std::ostringstream out;
    bool first = true;
    for (auto *label : labels) {
        if (!first) {
            out << ", ";
        }
        if (label == nullptr) {
            out << "null";
        } else {
            out << "L" << label->num;
        }
        first = false;
    }
    return out.str();
}

std::string asmKindText(instr::AssemInstr::Kind kind) {
    switch (kind) {
    case instr::AssemInstr::I_OPER:
        return "OPER";
    case instr::AssemInstr::I_LABEL:
        return "LABEL";
    case instr::AssemInstr::I_MOVE:
        return "MOVE";
    case instr::AssemInstr::I_CALL:
        return "CALL";
    case instr::AssemInstr::I_EXTCALL:
        return "EXTCALL";
    }
    return "UNKNOWN";
}

void maybeWriteAsmDebug(const BackendOptions &options, const std::string &suffix, instr::AsmProg *program) {
    if (!options.emitDebugFiles || options.debugBase.empty() || program == nullptr) {
        return;
    }
    std::ostringstream out;
    for (const auto &func : program->functions) {
        out << "Function " << func.name << "\n";
        for (std::size_t i = 0; i < func.instructions.size(); ++i) {
            const auto &ins = func.instructions[i];
            out << i << ": " << asmKindText(ins.kind) << " " << ins.assem
                << " dst=[" << tempListText(ins.dst) << "]"
                << " src=[" << tempListText(ins.src) << "]"
                << " jumps=[" << labelListText(ins.jumps.labels) << "]";
            if (ins.label != nullptr) {
                out << " label=L" << ins.label->num;
            }
            out << "\n";
        }
    }
    writeText(options.debugBase + suffix, out.str());
}

void maybeWriteColoringDebug(const BackendOptions &options, const std::string &suffix,
                             const std::vector<Coloring *> &colorings) {
    if (!options.emitDebugFiles || options.debugBase.empty()) {
        return;
    }
    std::ostringstream out;
    for (std::size_t i = 0; i < colorings.size(); ++i) {
        out << "Coloring " << i << "\n";
        if (colorings[i] == nullptr) {
            out << "<null>\n";
        } else {
            out << colorings[i]->printColoring();
        }
    }
    writeText(options.debugBase + suffix, out.str());
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

BackendResult compileTreeToArm(tree::Program *program, const BackendOptions &options) {
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

    instr::advDFGprog *graphProgram = instr::buildAdvDFGprog(ssaProgramForInstr);
    instr::preScheduleProg *preScheduleProgram = instr::buildPreScheduleProg(ssaProgramForInstr);
    if (graphProgram == nullptr || preScheduleProgram == nullptr) {
        result.error = "failed to build instruction-selection inputs";
        return result;
    }
    profile.mark("build-instr-graphs");

    instr::runInstructionSelectionPass(*graphProgram, *preScheduleProgram);
    profile.mark("instr-selection");
    instr::ScheduleProg *scheduleProgram = instr::scheduleProg(preScheduleProgram);
    if (scheduleProgram == nullptr) {
        result.error = "scheduling failed";
        return result;
    }
    profile.mark("scheduling");

    instr::AsmProg *asmProgram = scheduleToAsmProg(scheduleProgram);
    maybeWriteAsmDebug(options, ".5-schedule.asmdbg", asmProgram);
    instr::preDataFlowPass(asmProgram);
    maybeWriteAsmDebug(options, ".5-predataflow.asmdbg", asmProgram);
    profile.mark("asm-predataflow");
    std::vector<InterferenceGraph *> graphs = buildIgProg(asmProgram);
    profile.mark("interference");
    std::vector<Coloring *> colorings;
    for (auto *graph : graphs) {
        colorings.push_back(coloring(graph, options.registerCount));
    }
    profile.mark("coloring");
    maybeWriteColoringDebug(options, ".5-colors.txt", colorings);

    instr::AsmProg *colored = instr::asmprog2colored(asmProgram, colorings);
    if (colored == nullptr) {
        result.error = "register allocation failed";
        return result;
    }
    profile.mark("asm-coloring");

    result.ok = true;
    result.assembly = colored->to_string();
    profile.mark("asm-string");
    return result;
}

} // namespace backend
