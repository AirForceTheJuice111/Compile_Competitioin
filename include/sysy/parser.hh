#pragma once

#include "ast.hh"
#include "lexer.hh"

#include <stdexcept>
#include <string>
#include <vector>

namespace sysy {

class ParseError : public std::runtime_error {
public:
    ParseError(SourceLocation loc, const std::string &message);
    SourceLocation loc() const { return loc_; }

private:
    SourceLocation loc_;
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    NodePtr parseCompUnit();

private:
    const Token &peek(std::size_t ahead = 0) const;
    bool check(TokenKind kind, std::size_t ahead = 0) const;
    bool match(TokenKind kind);
    Token expect(TokenKind kind, const std::string &message);
    bool isBType(TokenKind kind) const;
    bool isFuncType(TokenKind kind) const;
    bool startsExpr(TokenKind kind) const;
    bool isFuncDefAhead() const;

    NodePtr parseDecl();
    NodePtr parseConstDecl();
    NodePtr parseVarDecl();
    NodePtr parseConstDef();
    NodePtr parseVarDef();
    NodePtr parseInitVal();
    NodePtr parseFuncDef();
    NodePtr parseFuncParam();
    NodePtr parseBlock();
    NodePtr parseBlockItem();
    NodePtr parseStmt();
    NodePtr parseLVal();
    NodePtr parsePrimaryExp();
    NodePtr parseUnaryExp();
    NodePtr parseMulExp();
    NodePtr parseAddExp();
    NodePtr parseRelExp();
    NodePtr parseEqExp();
    NodePtr parseLAndExp();
    NodePtr parseLOrExp();
    NodePtr parseExp();
    NodePtr parseBinaryRhs(NodePtr lhs, const std::vector<TokenKind> &ops,
                           NodePtr (Parser::*nextLevel)());

    std::vector<Token> tokens_;
    std::size_t pos_ = 0;
};

NodePtr parseSource(const std::string &source);

} // namespace sysy
