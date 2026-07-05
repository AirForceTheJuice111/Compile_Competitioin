#include "lexer.hh"

#include <cctype>
#include <stdexcept>
#include <unordered_map>

namespace sysy {

namespace {

bool isIdentStart(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool isIdentChar(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

bool isHexDigit(char c) {
    return std::isxdigit(static_cast<unsigned char>(c));
}

bool isOctDigit(char c) {
    return c >= '0' && c <= '7';
}

TokenKind keywordKind(const std::string &text) {
    static const std::unordered_map<std::string, TokenKind> keywords = {
        {"const", TokenKind::KwConst},
        {"int", TokenKind::KwInt},
        {"float", TokenKind::KwFloat},
        {"void", TokenKind::KwVoid},
        {"if", TokenKind::KwIf},
        {"else", TokenKind::KwElse},
        {"while", TokenKind::KwWhile},
        {"break", TokenKind::KwBreak},
        {"continue", TokenKind::KwContinue},
        {"return", TokenKind::KwReturn},
    };
    auto it = keywords.find(text);
    return it == keywords.end() ? TokenKind::Identifier : it->second;
}

} // namespace

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

bool Lexer::eof() const {
    return pos_ >= source_.size();
}

char Lexer::peek(std::size_t ahead) const {
    std::size_t p = pos_ + ahead;
    if (p >= source_.size()) {
        return '\0';
    }
    return source_[p];
}

char Lexer::advance() {
    if (eof()) {
        return '\0';
    }
    char c = source_[pos_++];
    if (c == '\n') {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }
    return c;
}

bool Lexer::consume(char c) {
    if (peek() != c) {
        return false;
    }
    advance();
    return true;
}

Token Lexer::make(TokenKind kind, SourceLocation loc, std::size_t begin, std::size_t end) const {
    Token tok;
    tok.kind = kind;
    tok.loc = loc;
    tok.text = source_.substr(begin, end - begin);
    return tok;
}

void Lexer::skipWhitespaceAndComments() {
    for (;;) {
        while (std::isspace(static_cast<unsigned char>(peek()))) {
            advance();
        }
        if (peek() == '/' && peek(1) == '/') {
            while (!eof() && peek() != '\n') {
                advance();
            }
            continue;
        }
        if (peek() == '/' && peek(1) == '*') {
            advance();
            advance();
            while (!eof() && !(peek() == '*' && peek(1) == '/')) {
                advance();
            }
            if (!eof()) {
                advance();
                advance();
            }
            continue;
        }
        return;
    }
}

Token Lexer::lexIdentifierOrKeyword() {
    SourceLocation loc{pos_, line_, column_};
    std::size_t begin = pos_;
    advance();
    while (isIdentChar(peek())) {
        advance();
    }
    std::size_t end = pos_;
    std::string text = source_.substr(begin, end - begin);
    return make(keywordKind(text), loc, begin, end);
}

Token Lexer::lexNumber() {
    SourceLocation loc{pos_, line_, column_};
    std::size_t begin = pos_;
    bool isFloat = false;

    if (peek() == '0' && (peek(1) == 'x' || peek(1) == 'X')) {
        advance();
        advance();
        while (isHexDigit(peek())) {
            advance();
        }
        if (peek() == '.') {
            isFloat = true;
            advance();
            while (isHexDigit(peek())) {
                advance();
            }
        }
        if (peek() == 'p' || peek() == 'P') {
            isFloat = true;
            advance();
            if (peek() == '+' || peek() == '-') {
                advance();
            }
            while (std::isdigit(static_cast<unsigned char>(peek()))) {
                advance();
            }
        }
    } else {
        if (peek() == '0') {
            advance();
            while (isOctDigit(peek())) {
                advance();
            }
        } else {
            while (std::isdigit(static_cast<unsigned char>(peek()))) {
                advance();
            }
        }
        if (peek() == '.') {
            isFloat = true;
            advance();
            while (std::isdigit(static_cast<unsigned char>(peek()))) {
                advance();
            }
        }
        if (peek() == 'e' || peek() == 'E') {
            isFloat = true;
            advance();
            if (peek() == '+' || peek() == '-') {
                advance();
            }
            while (std::isdigit(static_cast<unsigned char>(peek()))) {
                advance();
            }
        }
    }

    while (std::isalpha(static_cast<unsigned char>(peek()))) {
        advance();
    }
    return make(isFloat ? TokenKind::FloatLiteral : TokenKind::IntLiteral, loc, begin, pos_);
}

Token Lexer::lexString() {
    SourceLocation loc{pos_, line_, column_};
    std::size_t begin = pos_;
    advance();
    while (!eof()) {
        if (peek() == '\\') {
            advance();
            if (!eof()) {
                advance();
            }
            continue;
        }
        if (peek() == '"') {
            advance();
            break;
        }
        advance();
    }
    return make(TokenKind::StringLiteral, loc, begin, pos_);
}

Token Lexer::next() {
    skipWhitespaceAndComments();
    SourceLocation loc{pos_, line_, column_};
    std::size_t begin = pos_;

    if (eof()) {
        return make(TokenKind::End, loc, begin, begin);
    }
    if (isIdentStart(peek())) {
        return lexIdentifierOrKeyword();
    }
    if (std::isdigit(static_cast<unsigned char>(peek())) ||
        (peek() == '.' && std::isdigit(static_cast<unsigned char>(peek(1))))) {
        return lexNumber();
    }
    if (peek() == '"') {
        return lexString();
    }

    char c = advance();
    switch (c) {
    case '+': return make(TokenKind::Plus, loc, begin, pos_);
    case '-': return make(TokenKind::Minus, loc, begin, pos_);
    case '*': return make(TokenKind::Star, loc, begin, pos_);
    case '/': return make(TokenKind::Slash, loc, begin, pos_);
    case '%': return make(TokenKind::Percent, loc, begin, pos_);
    case ',': return make(TokenKind::Comma, loc, begin, pos_);
    case ';': return make(TokenKind::Semicolon, loc, begin, pos_);
    case '(': return make(TokenKind::LParen, loc, begin, pos_);
    case ')': return make(TokenKind::RParen, loc, begin, pos_);
    case '{': return make(TokenKind::LBrace, loc, begin, pos_);
    case '}': return make(TokenKind::RBrace, loc, begin, pos_);
    case '[': return make(TokenKind::LBracket, loc, begin, pos_);
    case ']': return make(TokenKind::RBracket, loc, begin, pos_);
    case '!':
        if (consume('=')) {
            return make(TokenKind::BangEqual, loc, begin, pos_);
        }
        return make(TokenKind::Bang, loc, begin, pos_);
    case '=':
        if (consume('=')) {
            return make(TokenKind::EqualEqual, loc, begin, pos_);
        }
        return make(TokenKind::Assign, loc, begin, pos_);
    case '<':
        if (consume('=')) {
            return make(TokenKind::LessEqual, loc, begin, pos_);
        }
        return make(TokenKind::Less, loc, begin, pos_);
    case '>':
        if (consume('=')) {
            return make(TokenKind::GreaterEqual, loc, begin, pos_);
        }
        return make(TokenKind::Greater, loc, begin, pos_);
    case '&':
        if (consume('&')) {
            return make(TokenKind::AndAnd, loc, begin, pos_);
        }
        return make(TokenKind::Invalid, loc, begin, pos_);
    case '|':
        if (consume('|')) {
            return make(TokenKind::OrOr, loc, begin, pos_);
        }
        return make(TokenKind::Invalid, loc, begin, pos_);
    default:
        return make(TokenKind::Invalid, loc, begin, pos_);
    }
}

std::vector<Token> Lexer::lexAll() {
    std::vector<Token> tokens;
    for (;;) {
        Token tok = next();
        tokens.push_back(tok);
        if (tok.kind == TokenKind::End) {
            break;
        }
    }
    return tokens;
}

std::string tokenKindName(TokenKind kind) {
    switch (kind) {
    case TokenKind::End: return "end";
    case TokenKind::Invalid: return "invalid";
    case TokenKind::Identifier: return "identifier";
    case TokenKind::IntLiteral: return "int-literal";
    case TokenKind::FloatLiteral: return "float-literal";
    case TokenKind::StringLiteral: return "string-literal";
    case TokenKind::KwConst: return "const";
    case TokenKind::KwInt: return "int";
    case TokenKind::KwFloat: return "float";
    case TokenKind::KwVoid: return "void";
    case TokenKind::KwIf: return "if";
    case TokenKind::KwElse: return "else";
    case TokenKind::KwWhile: return "while";
    case TokenKind::KwBreak: return "break";
    case TokenKind::KwContinue: return "continue";
    case TokenKind::KwReturn: return "return";
    case TokenKind::Plus: return "+";
    case TokenKind::Minus: return "-";
    case TokenKind::Star: return "*";
    case TokenKind::Slash: return "/";
    case TokenKind::Percent: return "%";
    case TokenKind::Bang: return "!";
    case TokenKind::Assign: return "=";
    case TokenKind::Less: return "<";
    case TokenKind::Greater: return ">";
    case TokenKind::LessEqual: return "<=";
    case TokenKind::GreaterEqual: return ">=";
    case TokenKind::EqualEqual: return "==";
    case TokenKind::BangEqual: return "!=";
    case TokenKind::AndAnd: return "&&";
    case TokenKind::OrOr: return "||";
    case TokenKind::Comma: return ",";
    case TokenKind::Semicolon: return ";";
    case TokenKind::LParen: return "(";
    case TokenKind::RParen: return ")";
    case TokenKind::LBrace: return "{";
    case TokenKind::RBrace: return "}";
    case TokenKind::LBracket: return "[";
    case TokenKind::RBracket: return "]";
    }
    return "unknown";
}

} // namespace sysy
