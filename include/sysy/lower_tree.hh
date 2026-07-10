#pragma once

#include "ast.hh"
#include "treep.hh"

#include <stdexcept>
#include <string>

namespace sysy {

struct LoweringOptions {
    bool parallelLoops = false;
    int pointerBytes = 4;
};

class LoweringError : public std::runtime_error {
public:
    LoweringError(SourceLocation loc, const std::string &message);
    SourceLocation loc() const { return loc_; }

private:
    SourceLocation loc_;
};

tree::Program *lowerToTree(const Node &root, const LoweringOptions &options = {});
std::string emitGlobalDataSection(const Node &root);

} // namespace sysy
