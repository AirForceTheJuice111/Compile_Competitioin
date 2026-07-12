#ifndef QUAD_OPT_METADATA_HH
#define QUAD_OPT_METADATA_HH

#include <map>
#include <string>

#include "flowinfo.hh"
#include "quad.hh"

namespace quad {

// Rebuild structural def/use sets, block successor labels, and temp/label
// extents after an optimization rewrites Quad operands or removes statements.
void rebuildQuadFunctionMetadata(QuadFuncDecl *func);
void rebuildQuadProgramMetadata(QuadProgram *program);

// Validate the invariants required by SSA value substitution: one definition
// per temp, every use dominated by its definition, and phi inputs associated
// with real predecessor edges. Metadata must be rebuilt before calling this.
bool verifySsaFunction(QuadFuncDecl *func, ControlFlowInfo *cfi,
                       std::string *reason = nullptr);

// Rewrite every structural temp use, including nested call operands and phi
// edge inputs. Definitions are intentionally never rewritten.
void rewriteQuadUses(QuadFuncDecl *func, const std::map<int, int> &replacements);

// Rewrite uses in one detached/cloned statement. This is used by structural
// transforms while assembling a new block, before it belongs to a function.
void rewriteQuadStatementUses(QuadStm *statement,
                              const std::map<int, int> &replacements);

// Iterative DCE restricted to side-effect-free scalar/address computations.
// Loads and all calls remain conservative even when their result is unused.
int eliminateDeadPureQuadDefs(QuadFuncDecl *func);

} // namespace quad

#endif
