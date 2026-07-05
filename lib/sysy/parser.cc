#include "parser.hh"

#include <sstream>

namespace sysy {

namespace {

NodePtr makeNode(NodeKind kind, const Token &tok, std::string text = {}) {
    return std::make_unique<Node>(kind, tok.loc, text.empty() ? tok.text : std::move(text));
}

NodePtr makeArrayDim(const Token &tok, NodePtr expr = {}) {
    auto node = makeNode(NodeKind::ArrayDim, tok, "[]");
    if (expr != nullptr) {
        node->add(std::move(expr));
    }
    return node;
}

std::string tokenTextForError(const Token &tok) {
    if (tok.kind == TokenKind::End) {
        return "end of file";
    }
    if (!tok.text.empty()) {
        return "'" + tok.text + "'";
    }
    return tokenKindName(tok.kind);
}

} // namespace

ParseError::ParseError(SourceLocation loc, const std::string &message)
    : std::runtime_error(message), loc_(loc) {}

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

const Token &Parser::peek(std::size_t ahead) const {
    std::size_t idx = pos_ + ahead;
    if (idx >= tokens_.size()) {
        return tokens_.back();
    }
    return tokens_[idx];
}

bool Parser::check(TokenKind kind, std::size_t ahead) const {
    return peek(ahead).kind == kind;
}

bool Parser::match(TokenKind kind) {
    if (!check(kind)) {
        return false;
    }
    ++pos_;
    return true;
}

Token Parser::expect(TokenKind kind, const std::string &message) {
    if (!check(kind)) {
        std::ostringstream os;
        os << message << ", got " << tokenTextForError(peek());
        throw ParseError(peek().loc, os.str());
    }
    return tokens_[pos_++];
}

bool Parser::isBType(TokenKind kind) const {
    return kind == TokenKind::KwInt || kind == TokenKind::KwFloat;
}

bool Parser::isFuncType(TokenKind kind) const {
    return isBType(kind) || kind == TokenKind::KwVoid;
}

bool Parser::startsExpr(TokenKind kind) const {
    return kind == TokenKind::Identifier || kind == TokenKind::IntLiteral ||
           kind == TokenKind::FloatLiteral || kind == TokenKind::LParen ||
           kind == TokenKind::Plus || kind == TokenKind::Minus || kind == TokenKind::Bang;
}

bool Parser::isFuncDefAhead() const {
    if (!isFuncType(peek().kind)) {
        return false;
    }
    return check(TokenKind::Identifier, 1) && check(TokenKind::LParen, 2);
}

NodePtr Parser::parseCompUnit() {
    auto root = std::make_unique<Node>(NodeKind::CompUnit, peek().loc, "CompUnit");
    while (!check(TokenKind::End)) {
        if (isFuncDefAhead()) {
            root->add(parseFuncDef());
        } else {
            root->add(parseDecl());
        }
    }
    return root;
}

NodePtr Parser::parseDecl() {
    if (check(TokenKind::KwConst)) {
        return parseConstDecl();
    }
    if (isBType(peek().kind)) {
        return parseVarDecl();
    }
    throw ParseError(peek().loc, "expected declaration");
}

NodePtr Parser::parseConstDecl() {
    Token start = expect(TokenKind::KwConst, "expected const");
    Token type = expect(peek().kind, "expected const base type");
    if (!isBType(type.kind)) {
        throw ParseError(type.loc, "expected int or float after const");
    }
    auto node = makeNode(NodeKind::ConstDecl, start, type.text);
    node->add(parseConstDef());
    while (match(TokenKind::Comma)) {
        node->add(parseConstDef());
    }
    expect(TokenKind::Semicolon, "expected ';' after const declaration");
    return node;
}

NodePtr Parser::parseVarDecl() {
    Token type = expect(peek().kind, "expected variable base type");
    if (!isBType(type.kind)) {
        throw ParseError(type.loc, "expected int or float");
    }
    auto node = makeNode(NodeKind::VarDecl, type, type.text);
    node->add(parseVarDef());
    while (match(TokenKind::Comma)) {
        node->add(parseVarDef());
    }
    expect(TokenKind::Semicolon, "expected ';' after variable declaration");
    return node;
}

NodePtr Parser::parseConstDef() {
    Token ident = expect(TokenKind::Identifier, "expected const name");
    auto node = makeNode(NodeKind::ConstDef, ident);
    while (match(TokenKind::LBracket)) {
        Token bracket = tokens_[pos_ - 1];
        node->add(makeArrayDim(bracket, parseExp()));
        expect(TokenKind::RBracket, "expected ']'");
    }
    expect(TokenKind::Assign, "expected '=' in const definition");
    node->add(parseInitVal());
    return node;
}

NodePtr Parser::parseVarDef() {
    Token ident = expect(TokenKind::Identifier, "expected variable name");
    auto node = makeNode(NodeKind::VarDef, ident);
    while (match(TokenKind::LBracket)) {
        Token bracket = tokens_[pos_ - 1];
        node->add(makeArrayDim(bracket, parseExp()));
        expect(TokenKind::RBracket, "expected ']'");
    }
    if (match(TokenKind::Assign)) {
        node->add(parseInitVal());
    }
    return node;
}

NodePtr Parser::parseInitVal() {
    if (!match(TokenKind::LBrace)) {
        return parseExp();
    }
    Token start = tokens_[pos_ - 1];
    auto node = makeNode(NodeKind::InitList, start, "{}");
    if (!check(TokenKind::RBrace)) {
        node->add(parseInitVal());
        while (match(TokenKind::Comma)) {
            node->add(parseInitVal());
        }
    }
    expect(TokenKind::RBrace, "expected '}' after initializer list");
    return node;
}

NodePtr Parser::parseFuncDef() {
    Token type = expect(peek().kind, "expected function type");
    if (!isFuncType(type.kind)) {
        throw ParseError(type.loc, "expected function type");
    }
    Token ident = expect(TokenKind::Identifier, "expected function name");
    auto node = makeNode(NodeKind::FuncDef, type, type.text + " " + ident.text);
    expect(TokenKind::LParen, "expected '(' after function name");
    if (!check(TokenKind::RParen)) {
        node->add(parseFuncParam());
        while (match(TokenKind::Comma)) {
            node->add(parseFuncParam());
        }
    }
    expect(TokenKind::RParen, "expected ')' after function parameters");
    node->add(parseBlock());
    return node;
}

NodePtr Parser::parseFuncParam() {
    Token type = expect(peek().kind, "expected parameter type");
    if (!isBType(type.kind)) {
        throw ParseError(type.loc, "expected int or float parameter type");
    }
    Token ident = expect(TokenKind::Identifier, "expected parameter name");
    auto node = makeNode(NodeKind::FuncParam, type, type.text + " " + ident.text);
    if (match(TokenKind::LBracket)) {
        Token bracket = tokens_[pos_ - 1];
        node->add(makeArrayDim(bracket));
        expect(TokenKind::RBracket, "expected ']' after omitted first dimension");
        while (match(TokenKind::LBracket)) {
            bracket = tokens_[pos_ - 1];
            node->add(makeArrayDim(bracket, parseExp()));
            expect(TokenKind::RBracket, "expected ']'");
        }
    }
    return node;
}

NodePtr Parser::parseBlock() {
    Token start = expect(TokenKind::LBrace, "expected '{'");
    auto node = makeNode(NodeKind::Block, start, "{}");
    while (!check(TokenKind::RBrace) && !check(TokenKind::End)) {
        node->add(parseBlockItem());
    }
    expect(TokenKind::RBrace, "expected '}' after block");
    return node;
}

NodePtr Parser::parseBlockItem() {
    if (check(TokenKind::KwConst) || isBType(peek().kind)) {
        return parseDecl();
    }
    return parseStmt();
}

NodePtr Parser::parseStmt() {
    if (match(TokenKind::LBrace)) {
        --pos_;
        return parseBlock();
    }
    if (match(TokenKind::KwIf)) {
        Token start = tokens_[pos_ - 1];
        auto node = makeNode(NodeKind::IfStmt, start, "if");
        expect(TokenKind::LParen, "expected '(' after if");
        node->add(parseExp());
        expect(TokenKind::RParen, "expected ')' after if condition");
        node->add(parseStmt());
        if (match(TokenKind::KwElse)) {
            node->add(parseStmt());
        }
        return node;
    }
    if (match(TokenKind::KwWhile)) {
        Token start = tokens_[pos_ - 1];
        auto node = makeNode(NodeKind::WhileStmt, start, "while");
        expect(TokenKind::LParen, "expected '(' after while");
        node->add(parseExp());
        expect(TokenKind::RParen, "expected ')' after while condition");
        node->add(parseStmt());
        return node;
    }
    if (match(TokenKind::KwBreak)) {
        Token start = tokens_[pos_ - 1];
        expect(TokenKind::Semicolon, "expected ';' after break");
        return makeNode(NodeKind::BreakStmt, start, "break");
    }
    if (match(TokenKind::KwContinue)) {
        Token start = tokens_[pos_ - 1];
        expect(TokenKind::Semicolon, "expected ';' after continue");
        return makeNode(NodeKind::ContinueStmt, start, "continue");
    }
    if (match(TokenKind::KwReturn)) {
        Token start = tokens_[pos_ - 1];
        auto node = makeNode(NodeKind::ReturnStmt, start, "return");
        if (!check(TokenKind::Semicolon)) {
            node->add(parseExp());
        }
        expect(TokenKind::Semicolon, "expected ';' after return");
        return node;
    }
    if (match(TokenKind::Semicolon)) {
        return makeNode(NodeKind::ExprStmt, tokens_[pos_ - 1], ";");
    }

    if (check(TokenKind::Identifier)) {
        std::size_t save = pos_;
        NodePtr lval = parseLVal();
        if (match(TokenKind::Assign)) {
            auto node = std::make_unique<Node>(NodeKind::AssignStmt, lval->loc, "=");
            node->add(std::move(lval));
            node->add(parseExp());
            expect(TokenKind::Semicolon, "expected ';' after assignment");
            return node;
        }
        pos_ = save;
    }

    if (startsExpr(peek().kind)) {
        auto node = std::make_unique<Node>(NodeKind::ExprStmt, peek().loc, "expr");
        node->add(parseExp());
        expect(TokenKind::Semicolon, "expected ';' after expression");
        return node;
    }
    throw ParseError(peek().loc, "expected statement");
}

NodePtr Parser::parseLVal() {
    Token ident = expect(TokenKind::Identifier, "expected identifier");
    auto node = makeNode(NodeKind::LVal, ident);
    while (match(TokenKind::LBracket)) {
        node->add(parseExp());
        expect(TokenKind::RBracket, "expected ']'");
    }
    return node;
}

NodePtr Parser::parsePrimaryExp() {
    if (match(TokenKind::LParen)) {
        auto node = parseExp();
        expect(TokenKind::RParen, "expected ')'");
        return node;
    }
    if (check(TokenKind::Identifier)) {
        return parseLVal();
    }
    if (match(TokenKind::IntLiteral) || match(TokenKind::FloatLiteral)) {
        return makeNode(NodeKind::Number, tokens_[pos_ - 1]);
    }
    if (match(TokenKind::StringLiteral)) {
        return makeNode(NodeKind::StringLiteral, tokens_[pos_ - 1]);
    }
    throw ParseError(peek().loc, "expected primary expression");
}

NodePtr Parser::parseUnaryExp() {
    if (check(TokenKind::Identifier) && check(TokenKind::LParen, 1)) {
        Token ident = expect(TokenKind::Identifier, "expected function name");
        auto node = makeNode(NodeKind::CallExpr, ident);
        expect(TokenKind::LParen, "expected '(' in call");
        if (!check(TokenKind::RParen)) {
            node->add(parseExp());
            while (match(TokenKind::Comma)) {
                node->add(parseExp());
            }
        }
        expect(TokenKind::RParen, "expected ')' after call arguments");
        return node;
    }
    if (match(TokenKind::Plus) || match(TokenKind::Minus) || match(TokenKind::Bang)) {
        Token op = tokens_[pos_ - 1];
        auto node = makeNode(NodeKind::UnaryExpr, op);
        node->add(parseUnaryExp());
        return node;
    }
    return parsePrimaryExp();
}

NodePtr Parser::parseBinaryRhs(NodePtr lhs, const std::vector<TokenKind> &ops,
                               NodePtr (Parser::*nextLevel)()) {
    for (;;) {
        bool found = false;
        for (TokenKind op : ops) {
            if (check(op)) {
                found = true;
                break;
            }
        }
        if (!found) {
            return lhs;
        }
        Token op = tokens_[pos_++];
        auto node = makeNode(NodeKind::BinaryExpr, op);
        node->add(std::move(lhs));
        node->add((this->*nextLevel)());
        lhs = std::move(node);
    }
}

NodePtr Parser::parseMulExp() {
    return parseBinaryRhs(parseUnaryExp(),
                          {TokenKind::Star, TokenKind::Slash, TokenKind::Percent},
                          &Parser::parseUnaryExp);
}

NodePtr Parser::parseAddExp() {
    return parseBinaryRhs(parseMulExp(), {TokenKind::Plus, TokenKind::Minus},
                          &Parser::parseMulExp);
}

NodePtr Parser::parseRelExp() {
    return parseBinaryRhs(parseAddExp(),
                          {TokenKind::Less, TokenKind::Greater, TokenKind::LessEqual,
                           TokenKind::GreaterEqual},
                          &Parser::parseAddExp);
}

NodePtr Parser::parseEqExp() {
    return parseBinaryRhs(parseRelExp(), {TokenKind::EqualEqual, TokenKind::BangEqual},
                          &Parser::parseRelExp);
}

NodePtr Parser::parseLAndExp() {
    return parseBinaryRhs(parseEqExp(), {TokenKind::AndAnd}, &Parser::parseEqExp);
}

NodePtr Parser::parseLOrExp() {
    return parseBinaryRhs(parseLAndExp(), {TokenKind::OrOr}, &Parser::parseLAndExp);
}

NodePtr Parser::parseExp() {
    return parseLOrExp();
}

NodePtr parseSource(const std::string &source) {
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.lexAll();
    for (const Token &tok : tokens) {
        if (tok.kind == TokenKind::Invalid) {
            throw ParseError(tok.loc, "invalid token '" + tok.text + "'");
        }
    }
    Parser parser(std::move(tokens));
    return parser.parseCompUnit();
}

std::string nodeKindName(NodeKind kind) {
    switch (kind) {
    case NodeKind::CompUnit: return "CompUnit";
    case NodeKind::ConstDecl: return "ConstDecl";
    case NodeKind::VarDecl: return "VarDecl";
    case NodeKind::ConstDef: return "ConstDef";
    case NodeKind::VarDef: return "VarDef";
    case NodeKind::FuncDef: return "FuncDef";
    case NodeKind::FuncParam: return "FuncParam";
    case NodeKind::ArrayDim: return "ArrayDim";
    case NodeKind::Block: return "Block";
    case NodeKind::AssignStmt: return "AssignStmt";
    case NodeKind::ExprStmt: return "ExprStmt";
    case NodeKind::IfStmt: return "IfStmt";
    case NodeKind::WhileStmt: return "WhileStmt";
    case NodeKind::BreakStmt: return "BreakStmt";
    case NodeKind::ContinueStmt: return "ContinueStmt";
    case NodeKind::ReturnStmt: return "ReturnStmt";
    case NodeKind::InitList: return "InitList";
    case NodeKind::BinaryExpr: return "BinaryExpr";
    case NodeKind::UnaryExpr: return "UnaryExpr";
    case NodeKind::CallExpr: return "CallExpr";
    case NodeKind::LVal: return "LVal";
    case NodeKind::Number: return "Number";
    case NodeKind::StringLiteral: return "StringLiteral";
    case NodeKind::Identifier: return "Identifier";
    case NodeKind::Empty: return "Empty";
    }
    return "Unknown";
}

void dumpAst(const Node &node, std::string &out, int indent) {
    out.append(static_cast<std::size_t>(indent), ' ');
    out += nodeKindName(node.kind);
    if (!node.text.empty()) {
        out += " ";
        out += node.text;
    }
    out += "\n";
    for (const auto &child : node.children) {
        dumpAst(*child, out, indent + 2);
    }
}

} // namespace sysy
