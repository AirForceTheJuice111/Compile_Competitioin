#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <sstream>
#include <map>
#include <set>
#include "asmprogpass.hh"
#include "temp.hh"

using namespace std;
using namespace tree;

namespace instr {

std::string getRegName(int colorNum) {
    if (colorNum == 11) return "fp";
    if (colorNum == 13) return "sp";
    if (colorNum == 14) return "lr";
    if (colorNum == 15) return "pc";
    return "r" + to_string(colorNum);
}

std::string getTempRegName(int tempNum, const Coloring* coloring) {
    if (Coloring::isMachineReg(tempNum)) return getRegName(tempNum);
    if (coloring == nullptr) return "r0";
    auto found = coloring->colors.find(tempNum);
    if (found == coloring->colors.end()) return "r0";
    return getRegName(found->second);
}

namespace {

string replaceAll(string text, const string &from, const string &to) {
    size_t pos = text.find(from);
    while (pos != string::npos) {
        text.replace(pos, from.size(), to);
        pos = text.find(from, pos + to.size());
    }
    return text;
}

bool isSpilled(int tempNum, const Coloring *coloring) {
    return coloring != nullptr && coloring->spilled.find(tempNum) != coloring->spilled.end();
}

bool isMoveInstruction(const AssemInstr &instr) {
    return instr.kind == AssemInstr::I_MOVE && instr.dst.size() == 1 && instr.src.size() == 1 &&
           instr.dst[0] != nullptr && instr.src[0] != nullptr;
}

int spillOffset(int tempNum, const map<int, int> &spillSlots) {
    auto found = spillSlots.find(tempNum);
    return found == spillSlots.end() ? -40 : found->second;
}

string loadSpill(const string &reg, int offset) {
    return "ldr " + reg + ", [fp, #" + to_string(offset) + "]";
}

string storeSpill(const string &reg, int offset) {
    return "str " + reg + ", [fp, #" + to_string(offset) + "]";
}

string patchFrameInstruction(const string &assem, int localSize) {
    int fpOffset = localSize + 32;
    if (assem == "sub sp, sp, #4") return "sub sp, sp, #" + to_string(localSize);
    if (assem == "add fp, sp, #36") return "add fp, sp, #" + to_string(fpOffset);
    if (assem == "sub sp, fp, #36") return "sub sp, fp, #" + to_string(fpOffset);
    if (assem == "add sp, sp, #4") return "add sp, sp, #" + to_string(localSize);
    return assem;
}

vector<string> coloredInstructionLines(
    const AssemInstr &instr,
    const Coloring *coloring,
    const map<int, int> &spillSlots,
    int localSize
) {
    vector<string> out;
    if (isMoveInstruction(instr) && !isSpilled(instr.dst[0]->num, coloring) &&
        !isSpilled(instr.src[0]->num, coloring) &&
        getTempRegName(instr.dst[0]->num, coloring) == getTempRegName(instr.src[0]->num, coloring)) {
        return out;
    }

    string text = patchFrameInstruction(instr.assem, localSize);
    vector<string> before;
    vector<string> after;
    vector<string> scratchRegs = {"r9", "r10"};
    size_t nextScratch = 0;

    for (size_t i = 0; i < instr.src.size(); ++i) {
        auto *temp = instr.src[i];
        if (temp == nullptr) continue;

        string replacement;
        if (isSpilled(temp->num, coloring)) {
            string scratch = scratchRegs[min(nextScratch, scratchRegs.size() - 1)];
            ++nextScratch;
            before.push_back(loadSpill(scratch, spillOffset(temp->num, spillSlots)));
            replacement = scratch;
        } else {
            replacement = getTempRegName(temp->num, coloring);
        }
        text = replaceAll(text, "`s" + to_string(i), replacement);
    }

    for (size_t i = 0; i < instr.dst.size(); ++i) {
        auto *temp = instr.dst[i];
        if (temp == nullptr) continue;

        string replacement;
        if (isSpilled(temp->num, coloring)) {
            string scratch = nextScratch == 0 ? "r10" : "r9";
            replacement = scratch;
            after.push_back(storeSpill(scratch, spillOffset(temp->num, spillSlots)));
        } else {
            replacement = getTempRegName(temp->num, coloring);
        }
        text = replaceAll(text, "`d" + to_string(i), replacement);
    }

    for (auto &line : before) out.push_back(line);
    out.push_back(text);
    for (auto &line : after) out.push_back(line);
    return out;
}

} // namespace

AsmProg* asmprog2colored(AsmProg* program, const vector<Coloring*>& colorings) {
    AsmProg* colored = new AsmProg();
    
    if (program == nullptr) return colored;

    for (size_t funcIndex = 0; funcIndex < program->functions.size(); ++funcIndex) {
        const auto &func = program->functions[funcIndex];
        const Coloring *coloring = funcIndex < colorings.size() ? colorings[funcIndex] : nullptr;
        AsmFunction coloredFunc(func.name);

        map<int, int> spillSlots;
        int slotIndex = 0;
        if (coloring != nullptr) {
            for (int temp : coloring->spilled) {
                spillSlots[temp] = -40 - slotIndex * 4;
                ++slotIndex;
            }
        }
        int localSize = 4 + static_cast<int>(spillSlots.size()) * 4;

        for (const auto &instr : func.instructions) {
            if (instr.kind == AssemInstr::I_LABEL) {
                coloredFunc.instructions.push_back(instr);
                continue;
            }

            auto lines = coloredInstructionLines(instr, coloring, spillSlots, localSize);
            for (const auto &line : lines) {
                coloredFunc.instructions.push_back(AssemInstr::Oper(line, {}, {}, instr.jumps));
            }
        }

        colored->functions.push_back(coloredFunc);
    }
    
    return colored;
}

} // namespace instr
