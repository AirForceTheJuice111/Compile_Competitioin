#include <cerrno>
#include <cctype>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "backend_driver.hh"
#include "lexer.hh"
#include "lower_tree.hh"
#include "parser.hh"
#include "semantics.hh"

namespace {

struct Options {
    bool emitAssembly = false;
    bool showHelp = false;
    bool dumpTokens = false;
    bool dumpAst = false;
    bool checkSysY = false;
    bool nativeBackend = false;
    std::string output;
    std::string input;
    std::string optLevel = "-O0";
    std::vector<std::string> passthrough;
};

void printUsage(std::ostream &os) {
    os << "Usage: compiler -S -o <output.s> <input.sy> [options]\n"
       << "\n"
       << "Contest-compatible SysY entry. The current contest branch lowers\n"
       << "SysY2022 source through the system ARM GCC frontend while the native\n"
       << "SysY frontend, IR, and backend are being migrated.\n"
       << "\n"
       << "Debugging:\n"
       << "  compiler --dump-tokens <input.sy>\n"
       << "  compiler --dump-ast <input.sy>\n"
       << "  compiler --check-sysy <input.sy>\n"
       << "  compiler --native-backend -S -o <output.s> <input.sy>\n";
}

bool isIdentifierStart(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool isIdentifierChar(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

std::string shellQuote(const std::string &s) {
    std::string out = "'";
    for (char c : s) {
        if (c == '\'') {
            out += "'\\''";
        } else {
            out.push_back(c);
        }
    }
    out.push_back('\'');
    return out;
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

bool hasWordAt(const std::string &s, size_t pos, const std::string &word) {
    if (pos + word.size() > s.size() || s.compare(pos, word.size(), word) != 0) {
        return false;
    }
    if (pos > 0 && isIdentifierChar(s[pos - 1])) {
        return false;
    }
    if (pos + word.size() < s.size() && isIdentifierChar(s[pos + word.size()])) {
        return false;
    }
    return true;
}

size_t skipSpaces(const std::string &s, size_t pos) {
    while (pos < s.size() && std::isspace(static_cast<unsigned char>(s[pos]))) {
        ++pos;
    }
    return pos;
}

size_t findStatementEnd(const std::string &s, size_t pos) {
    int paren = 0;
    int bracket = 0;
    int brace = 0;
    for (size_t i = pos; i < s.size(); ++i) {
        char c = s[i];
        if (c == '/' && i + 1 < s.size() && s[i + 1] == '/') {
            i += 2;
            while (i < s.size() && s[i] != '\n') {
                ++i;
            }
            if (i >= s.size()) {
                return std::string::npos;
            }
            continue;
        }
        if (c == '/' && i + 1 < s.size() && s[i + 1] == '*') {
            i += 2;
            while (i + 1 < s.size() && !(s[i] == '*' && s[i + 1] == '/')) {
                ++i;
            }
            if (i + 1 >= s.size()) {
                return std::string::npos;
            }
            ++i;
            continue;
        }
        if (c == '"') {
            ++i;
            while (i < s.size()) {
                if (s[i] == '\\') {
                    ++i;
                } else if (s[i] == '"') {
                    break;
                }
                ++i;
            }
            continue;
        }
        if (c == '\'') {
            ++i;
            while (i < s.size()) {
                if (s[i] == '\\') {
                    ++i;
                } else if (s[i] == '\'') {
                    break;
                }
                ++i;
            }
            continue;
        }
        if (c == '(') {
            ++paren;
        } else if (c == ')') {
            --paren;
        } else if (c == '[') {
            ++bracket;
        } else if (c == ']') {
            --bracket;
        } else if (c == '{') {
            ++brace;
        } else if (c == '}') {
            --brace;
        } else if (c == ';' && paren == 0 && bracket == 0 && brace == 0) {
            return i;
        }
    }
    return std::string::npos;
}

std::vector<std::string> splitTopLevelComma(const std::string &s) {
    std::vector<std::string> pieces;
    size_t start = 0;
    int paren = 0;
    int bracket = 0;
    int brace = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '(') {
            ++paren;
        } else if (c == ')') {
            --paren;
        } else if (c == '[') {
            ++bracket;
        } else if (c == ']') {
            --bracket;
        } else if (c == '{') {
            ++brace;
        } else if (c == '}') {
            --brace;
        } else if (c == ',' && paren == 0 && bracket == 0 && brace == 0) {
            pieces.push_back(s.substr(start, i - start));
            start = i + 1;
        }
    }
    pieces.push_back(s.substr(start));
    return pieces;
}

std::string trim(const std::string &s) {
    size_t b = 0;
    while (b < s.size() && std::isspace(static_cast<unsigned char>(s[b]))) {
        ++b;
    }
    size_t e = s.size();
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) {
        --e;
    }
    return s.substr(b, e - b);
}

bool tryParseScalarConstItem(const std::string &item, std::string &name, std::string &expr) {
    std::string t = trim(item);
    if (t.empty() || !isIdentifierStart(t[0])) {
        return false;
    }
    size_t pos = 1;
    while (pos < t.size() && isIdentifierChar(t[pos])) {
        ++pos;
    }
    name = t.substr(0, pos);
    pos = skipSpaces(t, pos);
    if (pos < t.size() && t[pos] == '[') {
        return false;
    }
    if (pos >= t.size() || t[pos] != '=') {
        return false;
    }
    expr = trim(t.substr(pos + 1));
    return !expr.empty();
}

bool isFloatyExpression(const std::string &expr) {
    for (size_t i = 0; i < expr.size(); ++i) {
        char c = expr[i];
        if (c == '.') {
            return true;
        }
        if ((c == 'e' || c == 'E' || c == 'p' || c == 'P')) {
            bool prevCanBeNumber = (i > 0 && (std::isdigit(static_cast<unsigned char>(expr[i - 1])) ||
                                              expr[i - 1] == '.'));
            bool nextCanBeExp = (i + 1 < expr.size() &&
                                 (std::isdigit(static_cast<unsigned char>(expr[i + 1])) ||
                                  expr[i + 1] == '+' || expr[i + 1] == '-'));
            if (prevCanBeNumber && nextCanBeExp) {
                return true;
            }
        }
    }
    return false;
}

std::string transformConstIntDeclarations(const std::string &src) {
    std::string out;
    out.reserve(src.size() + 4096);
    size_t i = 0;
    while (i < src.size()) {
        if (src[i] == '/' && i + 1 < src.size() && src[i + 1] == '/') {
            size_t end = src.find('\n', i + 2);
            if (end == std::string::npos) {
                out.append(src.substr(i));
                break;
            }
            out.append(src.substr(i, end + 1 - i));
            i = end + 1;
            continue;
        }
        if (src[i] == '/' && i + 1 < src.size() && src[i + 1] == '*') {
            size_t end = src.find("*/", i + 2);
            if (end == std::string::npos) {
                out.append(src.substr(i));
                break;
            }
            out.append(src.substr(i, end + 2 - i));
            i = end + 2;
            continue;
        }
        if (src[i] == '"') {
            size_t start = i++;
            while (i < src.size()) {
                if (src[i] == '\\') {
                    i += 2;
                } else if (src[i++] == '"') {
                    break;
                }
            }
            out.append(src.substr(start, i - start));
            continue;
        }
        if (src[i] == '\'') {
            size_t start = i++;
            while (i < src.size()) {
                if (src[i] == '\\') {
                    i += 2;
                } else if (src[i++] == '\'') {
                    break;
                }
            }
            out.append(src.substr(start, i - start));
            continue;
        }

        if (!hasWordAt(src, i, "const")) {
            out.push_back(src[i++]);
            continue;
        }
        size_t pos = skipSpaces(src, i + 5);
        if (!hasWordAt(src, pos, "int")) {
            out.push_back(src[i++]);
            continue;
        }
        size_t bodyStart = skipSpaces(src, pos + 3);
        size_t stmtEnd = findStatementEnd(src, bodyStart);
        if (stmtEnd == std::string::npos) {
            out.push_back(src[i++]);
            continue;
        }
        std::string body = src.substr(bodyStart, stmtEnd - bodyStart);
        std::vector<std::string> items = splitTopLevelComma(body);
        std::vector<std::pair<std::string, std::string>> scalars;
        bool ok = !items.empty();
        for (const std::string &item : items) {
            std::string name;
            std::string expr;
            if (!tryParseScalarConstItem(item, name, expr)) {
                ok = false;
                break;
            }
            if (isFloatyExpression(expr)) {
                expr = "((int)(" + expr + "))";
            }
            scalars.push_back({name, expr});
        }
        if (!ok) {
            out.append(src.substr(i, stmtEnd + 1 - i));
        } else {
            out += "enum { ";
            for (size_t k = 0; k < scalars.size(); ++k) {
                if (k != 0) {
                    out += ", ";
                }
                out += scalars[k].first;
                out += " = ";
                out += scalars[k].second;
            }
            out += " };";
        }
        i = stmtEnd + 1;
    }
    return out;
}

std::string injectSysYPrelude(const std::string &src) {
    std::ostringstream out;
    out << "#define starttime() _sysy_starttime(__LINE__)\n"
        << "#define stoptime() _sysy_stoptime(__LINE__)\n"
        << "extern int getint(void);\n"
        << "extern int getch(void);\n"
        << "extern int getarray(int a[]);\n"
        << "extern float getfloat(void);\n"
        << "extern int getfarray(float a[]);\n"
        << "extern void putint(int a);\n"
        << "extern void putch(int a);\n"
        << "extern void putarray(int n, int a[]);\n"
        << "extern void putfloat(float a);\n"
        << "extern void putfarray(int n, float a[]);\n"
        << "extern void putf(char a[], ...);\n"
        << "extern void _sysy_starttime(int lineno);\n"
        << "extern void _sysy_stoptime(int lineno);\n"
        << "#line 1 \"<sysy-input>\"\n"
        << transformConstIntDeclarations(src);
    return out.str();
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

void runNativeFrontendChecks(const std::string &source) {
    sysy::NodePtr root = sysy::parseSource(source);
    sysy::checkSemantics(*root);
}

int compileWithNativeBackend(const Options &opt, const std::string &source) {
    sysy::NodePtr root = sysy::parseSource(source);
    sysy::checkSemantics(*root);
    tree::Program *ir = sysy::lowerToTree(*root);

    backend::BackendOptions backendOptions;
    backendOptions.optMode = backend::optModeFromCompilerFlag(opt.optLevel);
    backend::BackendResult result = backend::compileTreeToArm(ir, backendOptions);
    if (!result.ok) {
        throw std::runtime_error("native backend failed: " + result.error);
    }
    writeFile(opt.output, result.assembly);
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
        } else if (arg == "--check-sysy") {
            opt.checkSysY = true;
        } else if (arg == "--native-backend") {
            opt.nativeBackend = true;
        } else if (arg == "-S") {
            opt.emitAssembly = true;
        } else if (arg == "-o") {
            if (i + 1 >= argc) {
                throw std::runtime_error("-o requires an output path");
            }
            opt.output = argv[++i];
        } else if (arg.rfind("-O", 0) == 0) {
            opt.optLevel = arg;
        } else if (arg == "-I" || arg == "-D" || arg == "-U" || arg == "-include") {
            if (i + 1 >= argc) {
                throw std::runtime_error(arg + " requires an argument");
            }
            opt.passthrough.push_back(arg);
            opt.passthrough.push_back(argv[++i]);
        } else if (arg.rfind("-I", 0) == 0 || arg.rfind("-D", 0) == 0 ||
                   arg.rfind("-U", 0) == 0) {
            opt.passthrough.push_back(arg);
        } else if (!arg.empty() && arg[0] == '-') {
            opt.passthrough.push_back(arg);
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
    if (!opt.dumpTokens && !opt.dumpAst && !opt.checkSysY && !opt.emitAssembly) {
        throw std::runtime_error("contest invocation must include -S");
    }
    if (opt.input.empty()) {
        throw std::runtime_error("missing input .sy file");
    }
    if (opt.output.empty()) {
        opt.output = "a.s";
    }
    return opt;
}

std::string getEnvOrDefault(const char *name, const std::string &fallback) {
    const char *value = std::getenv(name);
    if (value == nullptr || value[0] == '\0') {
        return fallback;
    }
    return value;
}

int runCommand(const std::string &cmd) {
    int rc = std::system(cmd.c_str());
    if (rc == -1) {
        std::cerr << "compiler: failed to launch ARM compiler: " << std::strerror(errno) << "\n";
        return 1;
    }
    if (WIFEXITED(rc)) {
        return WEXITSTATUS(rc);
    }
    if (WIFSIGNALED(rc)) {
        std::cerr << "compiler: ARM compiler terminated by signal " << WTERMSIG(rc) << "\n";
        return 128 + WTERMSIG(rc);
    }
    return 1;
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
        if (opt.checkSysY) {
            return checkSysY(source);
        }
        if (opt.nativeBackend) {
            return compileWithNativeBackend(opt, source);
        }
        runNativeFrontendChecks(source);
        std::string lowered = injectSysYPrelude(source);

        std::string tempPath = "/tmp/sysycc_" + std::to_string(static_cast<long long>(getpid())) + ".c";
        writeFile(tempPath, lowered);

        std::string armcc = getEnvOrDefault("SYSY_CC", getEnvOrDefault("ARM_CC", "arm-linux-gnueabihf-gcc"));
        std::ostringstream cmd;
        cmd << shellQuote(armcc)
            << " -x c -std=gnu99 -S"
            << " " << opt.optLevel
            << " -mcpu=cortex-a72 -fno-stack-protector -fno-pic -fno-pie -fno-common"
            << " -fsingle-precision-constant";
        for (const std::string &arg : opt.passthrough) {
            cmd << " " << shellQuote(arg);
        }
        cmd << " -o " << shellQuote(opt.output)
            << " " << shellQuote(tempPath);

        int rc = runCommand(cmd.str());
        std::remove(tempPath.c_str());
        return rc;
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
