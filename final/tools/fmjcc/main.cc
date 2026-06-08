#include <cstdlib>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "ASTheader.hh"
#include "FDMJAST.hh"
#include "advDFG.hh"
#include "asmprog.hh"
#include "asmprogpass.hh"
#include "ast2tree.hh"
#include "ast2xml.hh"
#include "blocking.hh"
#include "canon.hh"
#include "coloring.hh"
#include "flow2xml.hh"
#include "flowinfo.hh"
#include "ig.hh"
#include "instrSelection.hh"
#include "loopheader.hh"
#include "loopinductionopt.hh"
#include "looplicm.hh"
#include "opt.hh"
#include "preSchedule.hh"
#include "quad.hh"
#include "quad2xml.hh"
#include "quadssa.hh"
#include "schedule.hh"
#include "semant.hh"
#include "tinyxml2.hh"
#include "tree2quad.hh"
#include "tree2xml.hh"
#include "xml2ast.hh"

using namespace std;
namespace fs = std::filesystem;

namespace {

string stripSuffix(const string &path, const string &suffix) {
    if (path.size() >= suffix.size() && path.substr(path.size() - suffix.size()) == suffix) {
        return path.substr(0, path.size() - suffix.size());
    }
    return path;
}

bool writeText(const string &path, const string &text) {
    ofstream out(path);
    if (!out) return false;
    out << text;
    return true;
}

const char *instrKindName(instr::AssemInstr::Kind kind) {
    switch (kind) {
        case instr::AssemInstr::I_OPER: return "I_OPER";
        case instr::AssemInstr::I_LABEL: return "I_LABEL";
        case instr::AssemInstr::I_MOVE: return "I_MOVE";
        case instr::AssemInstr::I_CALL: return "I_CALL";
        case instr::AssemInstr::I_EXTCALL: return "I_EXTCALL";
    }
    return "I_OPER";
}

void appendTempList(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parent, const char *name, const vector<tree::Temp *> &temps) {
    auto *list = doc.NewElement(name);
    for (size_t i = 0; i < temps.size(); ++i) {
        if (temps[i] == nullptr) continue;
        auto *temp = doc.NewElement("Temp");
        temp->SetAttribute("index", static_cast<int>(i));
        temp->SetAttribute("num", temps[i]->num);
        string tempName = temps[i]->str();
        temp->SetAttribute("name", tempName.c_str());
        list->InsertEndChild(temp);
    }
    parent->InsertEndChild(list);
}

void appendJumpList(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parent, const instr::AssemTargets &jumps) {
    auto *list = doc.NewElement("Jumps");
    for (size_t i = 0; i < jumps.labels.size(); ++i) {
        if (jumps.labels[i] == nullptr) continue;
        auto *label = doc.NewElement("Label");
        label->SetAttribute("index", static_cast<int>(i));
        label->SetAttribute("num", jumps.labels[i]->num);
        string labelName = jumps.labels[i]->str();
        label->SetAttribute("name", labelName.c_str());
        list->InsertEndChild(label);
    }
    parent->InsertEndChild(list);
}

bool writeScheduleXml(const instr::ScheduleProg *schedule, const string &path) {
    if (schedule == nullptr) return false;

    tinyxml2::XMLDocument doc;
    auto *root = doc.NewElement("ScheduleProgram");
    if (schedule->quadProgram != nullptr) {
        root->SetAttribute("program_last_label_num", schedule->quadProgram->last_label_num);
        root->SetAttribute("program_last_temp_num", schedule->quadProgram->last_temp_num);
    }
    root->SetAttribute("function_count", static_cast<int>(schedule->funcSchedules.size()));
    doc.InsertEndChild(root);

    for (size_t i = 0; i < schedule->funcSchedules.size(); ++i) {
        auto *funcSchedule = schedule->funcSchedules[i];
        if (funcSchedule == nullptr || funcSchedule->quadFunc == nullptr) continue;

        auto *func = doc.NewElement("Function");
        func->SetAttribute("index", static_cast<int>(i));
        func->SetAttribute("name", funcSchedule->quadFunc->funcname.c_str());
        func->SetAttribute("last_label_num", funcSchedule->quadFunc->last_label_num);
        func->SetAttribute("last_temp_num", funcSchedule->quadFunc->last_temp_num);
        func->SetAttribute("instruction_count", static_cast<int>(funcSchedule->linearizedInstructions.instrs.size()));

        const auto &instrs = funcSchedule->linearizedInstructions.instrs;
        for (size_t j = 0; j < instrs.size(); ++j) {
            const auto &instr = instrs[j];
            auto *ins = doc.NewElement("Instruction");
            ins->SetAttribute("index", static_cast<int>(j));
            ins->SetAttribute("kind", instrKindName(instr.kind));
            ins->SetAttribute("assem", instr.assem.c_str());
            if (instr.kind == instr::AssemInstr::I_LABEL && instr.label != nullptr) {
                auto *label = doc.NewElement("Label");
                label->SetAttribute("num", instr.label->num);
                string labelName = instr.label->str();
                label->SetAttribute("name", labelName.c_str());
                ins->InsertEndChild(label);
            }
            appendTempList(doc, ins, "Dst", instr.dst);
            appendTempList(doc, ins, "Src", instr.src);
            appendJumpList(doc, ins, instr.jumps);
            func->InsertEndChild(ins);
        }

        root->InsertEndChild(func);
    }

    return doc.SaveFile(path.c_str()) == tinyxml2::XML_SUCCESS;
}

instr::AsmProg *scheduleToAsmProg(const instr::ScheduleProg *schedule) {
    auto *program = new instr::AsmProg();
    if (schedule == nullptr) return program;

    for (auto *funcSchedule : schedule->funcSchedules) {
        if (funcSchedule == nullptr || funcSchedule->quadFunc == nullptr) continue;
        instr::AsmFunction func(funcSchedule->quadFunc->funcname);
        for (const auto &ins : funcSchedule->linearizedInstructions.instrs) {
            func.instructions.push_back(ins);
        }
        program->functions.push_back(func);
    }
    return program;
}

set<FuncFlowInfo *> *computeFlow(quad::QuadProgram *program, const string &xmlPath) {
    auto *dataFlows = dataFLowProg(program);
    if (dataFlows == nullptr) return nullptr;

    auto *flows = new set<FuncFlowInfo *>();
    for (auto *dfi : *dataFlows) {
        if (dfi == nullptr || dfi->func == nullptr) continue;
        auto *cfi = new ControlFlowInfo(dfi->func);
        cfi->computeEverything();
        flows->insert(new FuncFlowInfo(cfi, dfi, program->last_label_num, program->last_temp_num));
    }

    if (!xmlPath.empty() && !flowinfo2xml(flows, xmlPath.c_str())) {
        cerr << "Error: failed to write flow XML: " << xmlPath << endl;
        return nullptr;
    }
    return flows;
}

void noteTemp(tree::Temp *temp, int &maxTemp) {
    if (temp != nullptr) maxTemp = max(maxTemp, temp->num);
}

void noteLabel(tree::Label *label, int &maxLabel) {
    if (label != nullptr) maxLabel = max(maxLabel, label->num);
}

void noteQuadTemp(quad::QuadTemp *temp, int &maxTemp) {
    if (temp != nullptr) noteTemp(temp->temp, maxTemp);
}

void noteQuadTerm(quad::QuadTerm *term, int &maxTemp) {
    if (term != nullptr && term->kind == quad::QuadTermKind::TEMP) {
        noteQuadTemp(term->get_temp(), maxTemp);
    }
}

void noteTempSet(set<tree::Temp *> *temps, int &maxTemp) {
    if (temps == nullptr) return;
    for (auto *temp : *temps) noteTemp(temp, maxTemp);
}

void noteTermList(vector<quad::QuadTerm *> *terms, int &maxTemp) {
    if (terms == nullptr) return;
    for (auto *term : *terms) noteQuadTerm(term, maxTemp);
}

void noteCall(quad::QuadCall *call, int &maxTemp) {
    if (call == nullptr) return;
    noteQuadTerm(call->obj_term, maxTemp);
    noteTermList(call->args, maxTemp);
}

void noteExtCall(quad::QuadExtCall *call, int &maxTemp) {
    if (call == nullptr) return;
    noteTermList(call->args, maxTemp);
}

void noteStatementExtents(quad::QuadStm *stm, int &maxLabel, int &maxTemp) {
    if (stm == nullptr) return;
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
    if (program == nullptr || program->quadFuncDeclList == nullptr) return;

    int programMaxLabel = program->last_label_num;
    int programMaxTemp = program->last_temp_num;
    for (auto *func : *program->quadFuncDeclList) {
        if (func == nullptr) continue;

        int funcMaxLabel = func->last_label_num;
        int funcMaxTemp = func->last_temp_num;
        if (func->params != nullptr) {
            for (auto *param : *func->params) noteTemp(param, funcMaxTemp);
        }
        if (func->quadblocklist != nullptr) {
            for (auto *block : *func->quadblocklist) {
                if (block == nullptr) continue;
                noteLabel(block->entry_label, funcMaxLabel);
                if (block->exit_labels != nullptr) {
                    for (auto *label : *block->exit_labels) noteLabel(label, funcMaxLabel);
                }
                if (block->quadlist != nullptr) {
                    for (auto *stm : *block->quadlist) noteStatementExtents(stm, funcMaxLabel, funcMaxTemp);
                }
            }
        }
        func->last_label_num = funcMaxLabel;
        func->last_temp_num = funcMaxTemp;
        programMaxLabel = max(programMaxLabel, funcMaxLabel);
        programMaxTemp = max(programMaxTemp, funcMaxTemp);
    }
    program->last_label_num = programMaxLabel;
    program->last_temp_num = programMaxTemp;
}

int runParser(const string &base) {
    string fileFmj = base + ".fmj";
    string fileAst = base + ".2.ast";

    cout << "------Parsing fmj source file: " << fileFmj << "------------" << endl;
    ifstream fmjFile(fileFmj);
    if (!fmjFile) {
        cerr << "Error: cannot open file " << fileFmj << endl;
        return 1;
    }

    fdmj::Program *root = fdmj::fdmjParser(fmjFile, false);
    if (root == nullptr) {
        cout << "AST is not valid!" << endl;
        return 1;
    }

    cout << "Convert AST  to XML..." << endl;
    tinyxml2::XMLDocument *xml = ast2xml(root, nullptr, true, false);
    if (xml == nullptr) {
        delete root;
        return 1;
    }

    tinyxml2::XMLError saveRc = xml->SaveFile(fileAst.c_str());
    cout << "Writing AST to file: " << fileAst << endl;
    bool ok = saveRc == tinyxml2::XML_SUCCESS && !xml->Error();
    delete xml;
    delete root;
    return ok ? 0 : 1;
}

} // namespace

int main(int argc, char **argv) {
    int k = 9;
    bool enableOptimizations = true;
    string input;
    string output;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--k" && i + 1 < argc) {
            k = stoi(argv[++i]);
        } else if (arg == "-o" && i + 1 < argc) {
            output = argv[++i];
        } else if (arg == "--no-opt") {
            enableOptimizations = false;
        } else if (arg == "--help" || arg == "-h") {
            cout << "Usage: " << argv[0] << " [--k 9] [--no-opt] [-o output.s] <file.fmj|base>\n";
            return 0;
        } else if (input.empty()) {
            input = arg;
        } else {
            cerr << "Unexpected argument: " << arg << endl;
            return 2;
        }
    }

    if (input.empty()) {
        cerr << "Usage: " << argv[0] << " [--k 9] [--no-opt] [-o output.s] <file.fmj|base>" << endl;
        return 2;
    }

    string base = stripSuffix(input, ".fmj");
    if (output.empty()) output = base + ".s";

    if (runParser(base) != 0) {
        cerr << "Error: parser rejected input " << base << ".fmj" << endl;
        return 1;
    }

    AST_Semant_Map *parsedSemant = nullptr;
    fdmj::Program *root = xml2ast(base + ".2.ast", &parsedSemant);
    if (root == nullptr) {
        cerr << "Error: failed to read parser AST" << endl;
        return 1;
    }

    AST_Semant_Map *semantMap = semant_analyze(root);
    if (semantMap == nullptr) {
        cerr << "Error: semantic analysis failed" << endl;
        return 1;
    }

    auto *semantXml = ast2xml(root, semantMap, true, true);
    semantXml->SaveFile((base + ".2-semant.ast").c_str());

    tree::Program *ir = ast2tree(root, semantMap);
    if (ir == nullptr) {
        cerr << "Error: AST to IR failed" << endl;
        return 1;
    }
    tree2xml(ir)->SaveFile((base + ".3.irp").c_str());

    tree::Program *canonIr = canon(ir);
    tree2xml(canonIr)->SaveFile((base + ".3-canon.irp").c_str());

    quad::QuadProgram *quadProgram = tree2quad(canonIr);
    if (quadProgram == nullptr) {
        cerr << "Error: IR to Quad failed" << endl;
        return 1;
    }
    quad2xml(quadProgram, (base + ".4-xml.quad").c_str());
    string quadText;
    quadProgram->print(quadText, 0, true);
    writeText(base + ".4.quad", quadText);

    quad::QuadProgram *blocked = blocking(quadProgram);
    if (blocked == nullptr) {
        cerr << "Error: Quad blocking failed" << endl;
        return 1;
    }
    string blockText;
    blocked->print(blockText, 0, true);
    writeText(base + ".4-block.quad", blockText);

    auto *blockedFlow = computeFlow(blocked, base + ".4-quadwithflow-xml.quad");
    if (blockedFlow == nullptr) return 1;

    quad::QuadProgram *ssa = quad2ssa(blockedFlow);
    if (ssa == nullptr) {
        cerr << "Error: Quad to SSA failed" << endl;
        return 1;
    }
    string ssaText;
    ssa->print(ssaText, 0, true);
    writeText(base + ".4-ssa.quad", ssaText);
    quad2xml(ssa, (base + ".4-ssa-xml.quad").c_str());

    auto *ssaFlow = computeFlow(ssa, base + ".4-ssa-withflow-xml.quad");
    if (ssaFlow == nullptr) return 1;

    quad::QuadProgram *optimizedSsa = nullptr;
    if (!enableOptimizations) {
        refreshQuadExtents(ssa);
        optimizedSsa = ssa;
        string noOptSsaText;
        optimizedSsa->print(noOptSsaText, 0, true);
        writeText(base + ".4-ssa-noopt.quad", noOptSsaText);
        quad2xml(optimizedSsa, (base + ".4-ssa-noopt-xml.quad").c_str());
    } else {
        quad::QuadProgram *sccp = optProg(ssa);
        if (sccp == nullptr) {
            cerr << "Error: HW8 SCCP optimization failed" << endl;
            return 1;
        }
        refreshQuadExtents(sccp);
        string sccpText;
        sccp->print(sccpText, 0, true);
        writeText(base + ".4-ssa-opt.quad", sccpText);
        quad2xml(sccp, (base + ".4-ssa-opt-xml.quad").c_str());

        auto *sccpFlow = computeFlow(sccp, base + ".4-ssa-opt-withflow-xml.quad");
        if (sccpFlow == nullptr) return 1;

        auto *licmFuncs = new vector<quad::QuadFuncDecl *>();
        int licmLastLabel = sccp->last_label_num;
        int licmLastTemp = sccp->last_temp_num;
        for (auto *ffi : *sccpFlow) {
            if (ffi == nullptr || ffi->cfi == nullptr || ffi->cfi->func == nullptr) continue;
            auto *func = ffi->cfi->func;
            auto *loopHeaders = findLoopHeadersWithFlow(func, ffi->cfi);
            licmFuncs->push_back(loopHoistFunc(func, loopHeaders));
            if (ffi->programLastLabelNum >= 0) licmLastLabel = ffi->programLastLabelNum;
            if (ffi->programLastTempNum >= 0) licmLastTemp = ffi->programLastTempNum;
        }
        auto *licm = new quad::QuadProgram(licmFuncs, licmLastLabel, licmLastTemp);
        refreshQuadExtents(licm);
        string licmText;
        licm->print(licmText, 0, true);
        writeText(base + ".4-ssa-loopopt.quad", licmText);
        quad2xml(licm, (base + ".4-ssa-loopopt-xml.quad").c_str());

        auto *licmFlow = computeFlow(licm, base + ".4-ssa-loopopt-withflow-xml.quad");
        if (licmFlow == nullptr) return 1;

        auto *ivFuncs = new vector<quad::QuadFuncDecl *>();
        int ivLastLabel = licm->last_label_num;
        int ivLastTemp = licm->last_temp_num;
        for (auto *ffi : *licmFlow) {
            if (ffi == nullptr || ffi->cfi == nullptr || ffi->cfi->func == nullptr) continue;
            auto *funcList = new vector<quad::QuadFuncDecl *>();
            funcList->push_back(ffi->cfi->func);
            auto *funcProg = new quad::QuadProgram(funcList, licm->last_label_num, licm->last_temp_num);

            auto *strengthReduced = loopInductionStrengthReductionPass(funcProg, ffi->cfi);
            refreshQuadExtents(strengthReduced);
            auto *cleaned = loopInductionCleanupPass(strengthReduced);
            refreshQuadExtents(cleaned);

            if (cleaned != nullptr && cleaned->quadFuncDeclList != nullptr && !cleaned->quadFuncDeclList->empty()) {
                ivFuncs->push_back(cleaned->quadFuncDeclList->at(0));
            }
            if (ffi->programLastLabelNum >= 0) ivLastLabel = ffi->programLastLabelNum;
            if (ffi->programLastTempNum >= 0) ivLastTemp = ffi->programLastTempNum;
        }
        optimizedSsa = new quad::QuadProgram(ivFuncs, ivLastLabel, ivLastTemp);
        refreshQuadExtents(optimizedSsa);
        string optSsaText;
        optimizedSsa->print(optSsaText, 0, true);
        writeText(base + ".4-ssa-loopivopt.quad", optSsaText);
        quad2xml(optimizedSsa, (base + ".4-ssa-loopivopt-xml.quad").c_str());
    }

    string optimizedFlowPath = enableOptimizations
        ? base + ".4-ssa-loopivopt-withflow-xml.quad"
        : base + ".4-ssa-noopt-withflow-xml.quad";
    auto *optimizedFlow = computeFlow(optimizedSsa, optimizedFlowPath);
    if (optimizedFlow == nullptr) return 1;

    auto *funcList = new vector<quad::QuadFuncDecl *>();
    int lastLabel = optimizedSsa->last_label_num;
    int lastTemp = optimizedSsa->last_temp_num;
    for (auto *ffi : *optimizedFlow) {
        if (ffi == nullptr || ffi->cfi == nullptr || ffi->cfi->func == nullptr) continue;
        funcList->push_back(ffi->cfi->func);
        if (ffi->programLastLabelNum >= 0) lastLabel = ffi->programLastLabelNum;
        if (ffi->programLastTempNum >= 0) lastTemp = ffi->programLastTempNum;
    }
    auto *ssaProgramForInstr = new quad::QuadProgram(funcList, lastLabel, lastTemp);

    instr::advDFGprog *graphProgram = instr::buildAdvDFGprog(ssaProgramForInstr);
    instr::preScheduleProg *preScheduleProgram = instr::buildPreScheduleProg(ssaProgramForInstr);
    if (graphProgram == nullptr || preScheduleProgram == nullptr) {
        cerr << "Error: failed to build instruction-selection inputs" << endl;
        return 1;
    }
    instr::runInstructionSelectionPass(*graphProgram, *preScheduleProgram);
    instr::ScheduleProg *scheduleProgram = instr::scheduleProg(preScheduleProgram);
    if (scheduleProgram == nullptr) {
        cerr << "Error: scheduling failed" << endl;
        return 1;
    }
    scheduleProgram->writeToFile(base);
    writeScheduleXml(scheduleProgram, base + ".5-xml.asm");

    instr::AsmProg *asmProgram = scheduleToAsmProg(scheduleProgram);
    instr::preDataFlowPass(asmProgram);
    vector<InterferenceGraph *> graphs = buildIgProg(asmProgram);
    vector<Coloring *> colorings;
    for (auto *graph : graphs) {
        colorings.push_back(coloring(graph, k));
    }

    instr::AsmProg *colored = instr::asmprog2colored(asmProgram, colorings);
    if (colored == nullptr) {
        cerr << "Error: register allocation failed" << endl;
        return 1;
    }

    string coloredText = colored->to_string();
    writeText(base + ".colored.s", coloredText);
    if (!writeText(output, coloredText)) {
        cerr << "Error: failed to write output " << output << endl;
        return 1;
    }

    return 0;
}
