#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace sysy {

struct SourceLocation {
    std::size_t offset = 0;
    int line = 1;
    int column = 1;
};

enum class TokenKind {
    End,
    Invalid,

    Identifier,
    IntLiteral,
    FloatLiteral,
    StringLiteral,

    KwConst,
    KwInt,
    KwFloat,
    KwVoid,
    KwIf,
    KwElse,
    KwWhile,
    KwBreak,
    KwContinue,
    KwReturn,

    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    Bang,
    Assign,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    EqualEqual,
    BangEqual,
    AndAnd,
    OrOr,

    Comma,
    Semicolon,
    LParen,
    RParen,
    LBrace,
    RBrace,
    LBracket,
    RBracket
};

struct Token {
    TokenKind kind = TokenKind::Invalid;
    SourceLocation loc;
    std::string text;
};

class Lexer {
public:
    explicit Lexer(std::string source);

    Token next();
    std::vector<Token> lexAll();

private:
    bool eof() const;
    char peek(std::size_t ahead = 0) const;
    char advance();
    bool consume(char c);
    void skipWhitespaceAndComments();
    Token make(TokenKind kind, SourceLocation loc, std::size_t begin, std::size_t end) const;
    Token lexIdentifierOrKeyword();
    Token lexNumber();
    Token lexString();

    std::string source_;
    std::size_t pos_ = 0;
    int line_ = 1;
    int column_ = 1;
};

std::string tokenKindName(TokenKind kind);

} // namespace sysy
