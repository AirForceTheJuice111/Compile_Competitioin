#include "aarch64_peephole.hh"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

namespace backend {
namespace {

enum class LineKind { Empty, Instruction, Label, Directive, Other };

struct AsmLine {
    LineKind kind = LineKind::Other;
    std::string original;
    std::string opcode;
    std::vector<std::string> operands;
    bool removed = false;
    bool rewritten = false;
};

std::string trim(const std::string &text) {
    std::size_t begin = text.find_first_not_of(" \t\r");
    if (begin == std::string::npos) return {};
    std::size_t end = text.find_last_not_of(" \t\r");
    return text.substr(begin, end - begin + 1);
}

std::vector<std::string> splitOperands(const std::string &text) {
    std::vector<std::string> result;
    std::size_t begin = 0;
    int brackets = 0;
    for (std::size_t i = 0; i <= text.size(); ++i) {
        char ch = i < text.size() ? text[i] : ',';
        if (ch == '[' || ch == '{') ++brackets;
        else if (ch == ']' || ch == '}') --brackets;
        if (ch == ',' && brackets == 0) {
            result.push_back(trim(text.substr(begin, i - begin)));
            begin = i + 1;
        }
    }
    if (result.size() == 1 && result.front().empty()) result.clear();
    return result;
}

AsmLine parseLine(const std::string &source) {
    AsmLine line;
    line.original = source;
    std::string text = trim(source);
    if (text.empty()) {
        line.kind = LineKind::Empty;
        return line;
    }
    if (text.back() == ':') {
        line.kind = LineKind::Label;
        return line;
    }
    if (text.front() == '.') {
        line.kind = LineKind::Directive;
        return line;
    }
    if (text.front() == '/' || text.front() == '#') {
        line.kind = LineKind::Other;
        return line;
    }
    std::size_t split = text.find_first_of(" \t");
    line.opcode = split == std::string::npos ? text : text.substr(0, split);
    std::string operandText = split == std::string::npos ? std::string{} : trim(text.substr(split + 1));
    line.operands = splitOperands(operandText);
    line.kind = LineKind::Instruction;
    return line;
}

bool isRegister(const std::string &operand) {
    if (operand == "sp" || operand == "wzr" || operand == "xzr") return true;
    if (operand.size() < 2 || (operand[0] != 'w' && operand[0] != 'x')) return false;
    return std::all_of(operand.begin() + 1, operand.end(),
                       [](unsigned char ch) { return std::isdigit(ch) != 0; });
}

bool sameNumberedRegister(const std::string &wide, const std::string &narrow) {
    return wide.size() > 1 && narrow.size() > 1 && wide[0] == 'x' &&
           narrow[0] == 'w' && wide.substr(1) == narrow.substr(1);
}

bool isScratchRegister(const std::string &operand) {
    if (!isRegister(operand) || operand.size() < 2 ||
        (operand[0] != 'w' && operand[0] != 'x')) return false;
    int number = std::stoi(operand.substr(1));
    return number >= 9 && number <= 17;
}

std::string labelText(const AsmLine &line) {
    std::string text = trim(line.original);
    return !text.empty() && text.back() == ':' ? text.substr(0, text.size() - 1) : text;
}

std::size_t nextNonEmpty(const std::vector<AsmLine> &lines, std::size_t index) {
    for (std::size_t i = index + 1; i < lines.size(); ++i) {
        if (!lines[i].removed && lines[i].kind != LineKind::Empty) return i;
    }
    return lines.size();
}

void rewrite(AsmLine &line, const std::string &opcode,
             std::vector<std::string> operands) {
    line.opcode = opcode;
    line.operands = std::move(operands);
    line.rewritten = true;
}

std::string render(const AsmLine &line) {
    if (!line.rewritten) return line.original;
    std::string result = "\t" + line.opcode;
    if (!line.operands.empty()) {
        result += " ";
        for (std::size_t i = 0; i < line.operands.size(); ++i) {
            if (i != 0) result += ", ";
            result += line.operands[i];
        }
    }
    return result;
}

} // namespace

std::string optimizeAarch64Assembly(const std::string &assembly,
                                    Aarch64PeepholeStats *statsOut) {
    Aarch64PeepholeStats stats;
    std::vector<AsmLine> lines;
    std::istringstream input(assembly);
    std::string source;
    while (std::getline(input, source)) lines.push_back(parseLine(source));

    for (int round = 0; round < 4; ++round) {
        bool changed = false;
        for (std::size_t i = 0; i < lines.size(); ++i) {
            AsmLine &line = lines[i];
            if (line.removed || line.kind != LineKind::Instruction) continue;
            if (line.opcode == "mov" && line.operands.size() == 2 &&
                line.operands[0] == line.operands[1]) {
                line.removed = true;
                ++stats.redundantMoves;
                changed = true;
                continue;
            }
            if ((line.opcode == "add" || line.opcode == "sub") &&
                line.operands.size() == 3 && line.operands[2] == "#0" &&
                isRegister(line.operands[0]) && isRegister(line.operands[1])) {
                if (line.operands[0] == line.operands[1]) line.removed = true;
                else rewrite(line, "mov", {line.operands[0], line.operands[1]});
                ++stats.zeroArithmetic;
                changed = true;
                continue;
            }
            if (line.opcode == "b" && line.operands.size() == 1) {
                std::size_t next = nextNonEmpty(lines, i);
                if (next < lines.size() && lines[next].kind == LineKind::Label &&
                    line.operands[0] == labelText(lines[next])) {
                    line.removed = true;
                    ++stats.branchesToNextLabel;
                    changed = true;
                    continue;
                }
            }
            if (line.opcode == "sxtw" && line.operands.size() == 2 &&
                sameNumberedRegister(line.operands[0], line.operands[1])) {
                std::size_t next = nextNonEmpty(lines, i);
                if (next == i + 1 && next < lines.size()) {
                    AsmLine &add = lines[next];
                    if (!add.removed && add.kind == LineKind::Instruction &&
                        add.opcode == "add" && add.operands.size() == 3 &&
                        add.operands[2] == line.operands[0] &&
                        !add.operands[0].empty() && add.operands[0][0] == 'x' &&
                        !add.operands[1].empty() && add.operands[1][0] == 'x') {
                        rewrite(add, "add", {add.operands[0], add.operands[1],
                                             line.operands[1], "sxtw"});
                        line.removed = true;
                        ++stats.extendedAdds;
                        changed = true;
                        continue;
                    }
                }
            }
            if ((line.opcode == "mov" || line.opcode == "movz") &&
                line.operands.size() == 2 && line.operands[1] == "#0" &&
                isScratchRegister(line.operands[0])) {
                std::size_t compareIndex = nextNonEmpty(lines, i);
                std::size_t branchIndex = compareIndex < lines.size()
                    ? nextNonEmpty(lines, compareIndex) : lines.size();
                if (compareIndex == i + 1 && branchIndex == compareIndex + 1 &&
                    branchIndex < lines.size()) {
                    AsmLine &compare = lines[compareIndex];
                    AsmLine &branch = lines[branchIndex];
                    if (compare.kind == LineKind::Instruction &&
                        compare.opcode == "cmp" && compare.operands.size() == 2 &&
                        compare.operands[1] == line.operands[0] &&
                        branch.kind == LineKind::Instruction &&
                        branch.opcode.rfind("b.", 0) == 0) {
                        compare.operands[1] = "#0";
                        compare.rewritten = true;
                        line.removed = true;
                        ++stats.zeroCompares;
                        changed = true;
                    }
                }
            }
        }
        if (!changed) break;
    }

    std::ostringstream output;
    for (const auto &line : lines) {
        if (!line.removed) output << render(line) << "\n";
    }
    if (statsOut != nullptr) *statsOut = stats;
    return output.str();
}

} // namespace backend
