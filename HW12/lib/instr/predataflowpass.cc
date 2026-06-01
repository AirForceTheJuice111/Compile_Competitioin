#include "asmprogpass.hh"
#include "temp.hh"

#include <string>

namespace instr {

void preDataFlowPass(AsmProg* prog) {
    if (prog == nullptr) return;

    static tree::Temp r0(0);
    static tree::Temp r1(1);
    static tree::Temp r2(2);
    static tree::Temp r3(3);
    std::vector<tree::Temp*> callerSaved = {&r0, &r1, &r2, &r3};

    for (auto &func : prog->functions) {
        for (auto &instr : func.instructions) {
            bool isCall = instr.kind == AssemInstr::I_CALL ||
                          instr.kind == AssemInstr::I_EXTCALL ||
                          instr.assem.rfind("bl ", 0) == 0 ||
                          instr.assem.rfind("blx ", 0) == 0;
            if (!isCall) continue;

            for (auto *reg : callerSaved) {
                bool exists = false;
                for (auto *dst : instr.dst) {
                    if (dst != nullptr && dst->num == reg->num) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) instr.dst.push_back(reg);
            }
        }
    }
}

} // namespace instr
