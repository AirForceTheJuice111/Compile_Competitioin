#ifndef BACKEND_AARCH64_PEEPHOLE_HH
#define BACKEND_AARCH64_PEEPHOLE_HH

#include <string>

namespace backend {

struct Aarch64PeepholeStats {
    int redundantMoves = 0;
    int zeroArithmetic = 0;
    int branchesToNextLabel = 0;
    int extendedAdds = 0;
    int zeroCompares = 0;
};

std::string optimizeAarch64Assembly(
    const std::string &assembly,
    Aarch64PeepholeStats *stats = nullptr);

} // namespace backend

#endif
