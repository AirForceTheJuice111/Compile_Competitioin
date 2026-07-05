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
    int registerCount = 9;
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
BackendResult compileTreeToArm(tree::Program *program, const BackendOptions &options = {});

} // namespace backend
