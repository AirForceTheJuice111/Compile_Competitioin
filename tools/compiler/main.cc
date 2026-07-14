#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "backend_driver.hh"
#include "lexer.hh"
#include "lower_tree.hh"
#include "parallel_plan.hh"
#include "parser.hh"
#include "semantics.hh"

namespace {

struct Options {
    bool emitAssembly = false;
    bool showHelp = false;
    bool dumpTokens = false;
    bool dumpAst = false;
    bool dumpParallelPlan = false;
    bool checkSysY = false;
    bool parallelNative = false;
    bool parallelNativeExplicit = false;
    bool disableParallelNative = false;
    bool emitDebugFiles = false;
    std::string output;
    std::string input;
    std::string optLevel = "-O0";
    std::string debugPrefix;
};

void printUsage(std::ostream &os) {
    os << "Usage: compiler -S -o <output.s> <input.sy> [options]\n"
       << "\n"
       << "Contest-compatible SysY entry. By default this uses -O0; performance\n"
       << "testing passes -O1 to enable the full optimizer and native AArch64 backend.\n"
       << "\n"
       << "Debugging:\n"
       << "  compiler --dump-tokens <input.sy>\n"
       << "  compiler --dump-ast <input.sy>\n"
       << "  compiler --dump-parallel-plan <input.sy>\n"
       << "  compiler --check-sysy <input.sy>\n"
       << "  compiler --debug-prefix /tmp/case -S -o <output.s> <input.sy>\n"
       << "  compiler --opt-mode none|const|loop1|loop2|allloop|allopt -S -o <output.s> <input.sy>\n"
       << "  compiler --target aarch64 -S -o <output.s> <input.sy>\n"
       << "  compiler --parallel-native -S -o <output.s> <input.sy>\n"
       << "  compiler --no-parallel-native -S -o <output.s> <input.sy>\n";
}

bool isNativeOptModeName(const std::string &arg) {
    return arg == "none" || arg == "no" || arg == "noopt" ||
           arg == "const" || arg == "sccp" ||
           arg == "loop1" || arg == "licm" ||
           arg == "loop2" || arg == "iv" || arg == "strength" ||
           arg == "allloop" || arg == "loops" ||
           arg == "allopt";
}

std::string lowerAscii(std::string s) {
    for (char &ch : s) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return s;
}

std::string readFile(const std::string &path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("cannot open input file: " + path);
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void writeFile(const std::string &path, const std::string &content) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("cannot write temporary file: " + path);
    }
    out << content;
}

int dumpTokens(const std::string &path, const std::string &source) {
    sysy::Lexer lexer(source);
    bool ok = true;
    for (;;) {
        sysy::Token tok = lexer.next();
        std::cout << path << ":" << tok.loc.line << ":" << tok.loc.column << " "
                  << sysy::tokenKindName(tok.kind);
        if (!tok.text.empty()) {
            std::cout << " " << tok.text;
        }
        std::cout << "\n";
        if (tok.kind == sysy::TokenKind::Invalid) {
            ok = false;
        }
        if (tok.kind == sysy::TokenKind::End) {
            break;
        }
    }
    return ok ? 0 : 1;
}

int dumpAst(const std::string &source) {
    sysy::NodePtr root = sysy::parseSource(source);
    std::string out;
    sysy::dumpAst(*root, out);
    std::cout << out;
    return 0;
}

int checkSysY(const std::string &source) {
    sysy::NodePtr root = sysy::parseSource(source);
    sysy::checkSemantics(*root);
    std::cout << "ok\n";
    return 0;
}

int dumpParallelPlan(const std::string &source) {
    sysy::NodePtr root = sysy::parseSource(source);
    sysy::checkSemantics(*root);
    std::cout << sysy::dumpParallelPlansJsonl(*root);
    return 0;
}

int compileWithNativeBackend(const Options &opt, const std::string &source) {
    sysy::NodePtr root = sysy::parseSource(source);
    sysy::checkSemantics(*root);
    sysy::LoweringOptions loweringOptions;
    loweringOptions.parallelLoops = opt.parallelNative;
    loweringOptions.pointerBytes = 8;
    tree::Program *ir = sysy::lowerToTree(*root, loweringOptions);

    backend::BackendOptions backendOptions;
    backendOptions.optMode = backend::optModeFromCompilerFlag(opt.optLevel);
    backendOptions.emitDebugFiles = opt.emitDebugFiles;
    backendOptions.debugBase = opt.debugPrefix;
    backend::BackendResult result = backend::compileTreeToAarch64(ir, backendOptions);
    if (!result.ok) {
        throw std::runtime_error("native backend failed: " + result.error);
    }
    writeFile(opt.output, result.assembly + sysy::emitGlobalDataSection(*root));
    return 0;
}

Options parseArgs(int argc, char **argv) {
    Options opt;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            opt.showHelp = true;
        } else if (arg == "--dump-tokens") {
            opt.dumpTokens = true;
        } else if (arg == "--dump-ast") {
            opt.dumpAst = true;
        } else if (arg == "--dump-parallel-plan") {
            opt.dumpParallelPlan = true;
        } else if (arg == "--check-sysy") {
            opt.checkSysY = true;
        } else if (arg == "--native-backend") {
            // The native AArch64 backend is the only assembly backend.
        } else if (arg == "--gcc-bridge") {
            throw std::runtime_error("--gcc-bridge was removed with the ARM32 backend; use native AArch64 output");
        } else if (arg == "--aarch64-gcc-bridge") {
            throw std::runtime_error("--aarch64-gcc-bridge was removed; use native AArch64 output");
        } else if (arg == "--target") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--target requires aarch64");
            }
            std::string target = lowerAscii(argv[++i]);
            if (target != "aarch64" && target != "arm64" && target != "armv8-a") {
                throw std::runtime_error("only AArch64 is supported; ARM32 backend has been removed");
            }
        } else if (arg.rfind("--target=", 0) == 0) {
            std::string target = lowerAscii(arg.substr(std::string("--target=").size()));
            if (target != "aarch64" && target != "arm64" && target != "armv8-a") {
                throw std::runtime_error("only AArch64 is supported; ARM32 backend has been removed");
            }
        } else if (arg == "--emit-c" || arg == "--parallel-c" ||
                   arg == "--parallel-c-abi" ||
                   arg == "--parallel-c-abi-inline-runtime") {
            throw std::runtime_error(arg + " was removed with the C++ prototype path; use native AArch64 output");
        } else if (arg == "--parallel-asm-bridge") {
            throw std::runtime_error("--parallel-asm-bridge was removed; use --parallel-native");
        } else if (arg == "--parallel-native") {
            opt.parallelNative = true;
            opt.parallelNativeExplicit = true;
            opt.disableParallelNative = false;
        } else if (arg == "--no-parallel-native") {
            opt.parallelNative = false;
            opt.parallelNativeExplicit = true;
            opt.disableParallelNative = true;
        } else if (arg == "--debug-prefix") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--debug-prefix requires a path prefix");
            }
            opt.emitDebugFiles = true;
            opt.debugPrefix = argv[++i];
        } else if (arg == "--opt-mode") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--opt-mode requires a mode");
            }
            opt.optLevel = argv[++i];
        } else if (arg == "-S") {
            opt.emitAssembly = true;
        } else if (arg == "-o") {
            if (i + 1 >= argc) {
                throw std::runtime_error("-o requires an output path");
            }
            opt.output = argv[++i];
        } else if (arg.rfind("-O", 0) == 0) {
            opt.optLevel = arg;
        } else if (isNativeOptModeName(arg)) {
            opt.optLevel = arg;
        } else if (!arg.empty() && arg[0] == '-') {
            throw std::runtime_error("unsupported option for native AArch64 backend: " + arg);
        } else {
            if (!opt.input.empty()) {
                throw std::runtime_error("multiple input files are not supported");
            }
            opt.input = arg;
        }
    }
    if (opt.showHelp) {
        return opt;
    }
    bool targetApplies = opt.emitAssembly && !opt.dumpTokens && !opt.dumpAst &&
                         !opt.dumpParallelPlan && !opt.checkSysY;
    if (targetApplies && !opt.parallelNativeExplicit &&
        !opt.disableParallelNative &&
        backend::optModeFromCompilerFlag(opt.optLevel) != backend::OptMode::None) {
        opt.parallelNative = true;
    }
    if (!opt.dumpTokens && !opt.dumpAst && !opt.dumpParallelPlan && !opt.checkSysY &&
        !opt.emitAssembly) {
        throw std::runtime_error("contest invocation must include -S or a dump/check option");
    }
    if (opt.input.empty()) {
        throw std::runtime_error("missing input .sy file");
    }
    if (opt.output.empty()) {
        opt.output = "a.s";
    }
    return opt;
}

} // namespace

int main(int argc, char **argv) {
    try {
        Options opt = parseArgs(argc, argv);
        if (opt.showHelp) {
            printUsage(std::cout);
            return 0;
        }

        std::string source = readFile(opt.input);
        if (opt.dumpTokens) {
            return dumpTokens(opt.input, source);
        }
        if (opt.dumpAst) {
            return dumpAst(source);
        }
        if (opt.dumpParallelPlan) {
            return dumpParallelPlan(source);
        }
        if (opt.checkSysY) {
            return checkSysY(source);
        }
        return compileWithNativeBackend(opt, source);
    } catch (const sysy::ParseError &ex) {
        std::cerr << "compiler: " << ex.loc().line << ":" << ex.loc().column << ": "
                  << ex.what() << "\n";
        return 1;
    } catch (const sysy::SemanticError &ex) {
        std::cerr << "compiler: " << ex.loc().line << ":" << ex.loc().column << ": "
                  << ex.what() << "\n";
        return 1;
    } catch (const sysy::LoweringError &ex) {
        std::cerr << "compiler: " << ex.loc().line << ":" << ex.loc().column << ": "
                  << ex.what() << "\n";
        return 1;
    } catch (const std::exception &ex) {
        std::cerr << "compiler: " << ex.what() << "\n";
        printUsage(std::cerr);
        return 1;
    }
}
