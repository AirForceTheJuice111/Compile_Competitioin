#pragma once

#include "treep.hh"

#include <string>

namespace backend {

enum class OptMode {
    None,
    Const,
    Loop1,
    Loop2,
    AllLoop,
    AllOpt
};

struct BackendOptions {
    OptMode optMode = OptMode::AllOpt;
    bool emitDebugFiles = false;
    std::string debugBase;
};

struct BackendResult {
    bool ok = false;
    std::string assembly;
    std::string error;
};

OptMode optModeFromCompilerFlag(const std::string &flag);
BackendResult compileTreeToAarch64(tree::Program *program, const BackendOptions &options = {});

} // namespace backend
