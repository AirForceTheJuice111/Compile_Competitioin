#include <iostream>
#include <cstdint>
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

vector<string> loadImmediateIntoIp(uint32_t value);

bool isMoveInstruction(const AssemInstr &instr) {
    return instr.kind == AssemInstr::I_MOVE && instr.dst.size() == 1 && instr.src.size() == 1 &&
           instr.dst[0] != nullptr && instr.src[0] != nullptr;
}

int spillOffset(int tempNum, const map<int, int> &spillSlots) {
    auto found = spillSlots.find(tempNum);
    return found == spillSlots.end() ? -40 : found->second;
}

bool isArmAddressImmediate(int offset) {
    return offset >= -4095 && offset <= 4095;
}

vector<string> addressSpillSlot(int offset) {
    uint32_t magnitude = static_cast<uint32_t>(offset < 0 ? -offset : offset);
    vector<string> out = loadImmediateIntoIp(magnitude);
    out.push_back(string(offset < 0 ? "sub" : "add") + " ip, fp, ip");
    return out;
}

vector<string> loadSpill(const string &reg, int offset) {
    if (isArmAddressImmediate(offset)) {
        return {"ldr " + reg + ", [fp, #" + to_string(offset) + "]"};
    }

    vector<string> out = addressSpillSlot(offset);
    out.push_back("ldr " + reg + ", [ip]");
    return out;
}

vector<string> storeSpill(const string &reg, int offset) {
    if (isArmAddressImmediate(offset)) {
        return {"str " + reg + ", [fp, #" + to_string(offset) + "]"};
    }

    vector<string> out = addressSpillSlot(offset);
    out.push_back("str " + reg + ", [ip]");
    return out;
}

string patchFrameInstruction(const string &assem, int localSize) {
    int fpOffset = localSize + 32;
    if (assem == "sub sp, sp, #4") return "sub sp, sp, #" + to_string(localSize);
    if (assem == "add fp, sp, #36") return "add fp, sp, #" + to_string(fpOffset);
    if (assem == "sub sp, fp, #36") return "sub sp, fp, #" + to_string(fpOffset);
    if (assem == "add sp, sp, #4") return "add sp, sp, #" + to_string(localSize);
    return assem;
}

uint32_t rotateLeft(uint32_t value, int amount) {
    amount &= 31;
    if (amount == 0) return value;
    return (value << amount) | (value >> (32 - amount));
}

bool isArmDataImmediate(uint32_t value) {
    for (int rotate = 0; rotate < 32; rotate += 2) {
        if ((rotateLeft(value, rotate) & ~0xffu) == 0) {
            return true;
        }
    }
    return false;
}

vector<string> loadImmediateIntoIp(uint32_t value) {
    vector<string> out;
    uint32_t low = value & 0xffffu;
    uint32_t high = (value >> 16) & 0xffffu;
    out.push_back("movw ip, #" + to_string(low));
    if (high != 0) {
        out.push_back("movt ip, #" + to_string(high));
    }
    return out;
}

vector<string> expandLargeArmImmediate(const string &line) {
    static const regex addSubImm(R"(^\s*(add|sub)\s+(\w+),\s*(\w+),\s*#([0-9]+)\s*$)");
    smatch match;
    if (!regex_match(line, match, addSubImm)) {
        return {line};
    }

    uint32_t value = static_cast<uint32_t>(stoul(match[4].str()));
    if (isArmDataImmediate(value)) {
        return {line};
    }
    vector<string> out = loadImmediateIntoIp(value);
    out.push_back(match[1].str() + " " + match[2].str() + ", " + match[3].str() + ", ip");
    return out;
}

vector<string> coloredInstructionLines( // returns colored instruction lines, including multiple lines for spill code
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
            auto spillLoad = loadSpill(scratch, spillOffset(temp->num, spillSlots));
            before.insert(before.end(), spillLoad.begin(), spillLoad.end());
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
            replacement = "r10";
            auto spillStore = storeSpill("r10", spillOffset(temp->num, spillSlots));
            after.insert(after.end(), spillStore.begin(), spillStore.end());
        } else {
            replacement = getTempRegName(temp->num, coloring);
        }
        text = replaceAll(text, "`d" + to_string(i), replacement);
    }

    for (auto &line : before) out.push_back(line);
    for (const auto &line : expandLargeArmImmediate(text)) out.push_back(line);
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
                spillSlots[temp] = -40 - slotIndex * 4; // stack memory offset for spilled temp
                ++slotIndex;
            }
        }
        int localSize = 4 + static_cast<int>(spillSlots.size()) * 4;
        // The fixed prologue saves 9 registers (36 bytes), so SP is 4 mod 8
        // before local allocation. AAPCS requires SP to be 8-byte aligned at
        // calls; keep the local area 4 mod 8 to restore alignment.
        if (localSize % 8 != 4) {
            localSize += 4;
        }

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
