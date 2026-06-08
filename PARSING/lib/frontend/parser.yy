%skeleton "lalr1.cc"
%require  "3.2"
%language "c++"
%locations

%code requires {
  #define DEBUG
  #undef DEBUG
  #include <iostream>
  #include <vector>
  #include <algorithm>
  #include <string>
  #include <variant>
  #include "ast_location.hh"
  #include "ASTLexer.hh"
  #include "ASTheader.hh"
  #include "FDMJAST.hh"

  using namespace std;
  using namespace fdmj;

  //extern Program* root;
}

%define api.namespace {fdmj}
%define api.parser.class {ASTParser}
%define api.value.type {AST_YYSTYPE}
%define api.location.type {ast_location}

%define parse.error detailed
%define parse.trace

%header
%verbose

%parse-param {ASTLexer &lexer}
%parse-param {const bool debug}
%parse-param {AST_YYSTYPE* result}

%initial-action
{
    #if YYDEBUG != 0
        set_debug_level(debug);
    #endif
};

%code {
    namespace fdmj 
    {
        template<typename RHS>
        void calcLocation(location_t &current, const RHS &rhs, const std::size_t n);
    }
    
    #define YYLLOC_DEFAULT(Cur, Rhs, N) calcLocation(Cur, Rhs, N)
    #define yylex lexer.yylex
    Pos *p;
}

//terminals with no value 
%token PUBLIC INT MAIN CLASS EXTENDS IF ELSE WHILE CONTINUE BREAK RETURN 
%token PUTINT PUTCH PUTARRAY STARTTIME STOPTIME THIS LENGTH NEW GETINT GETCH GETARRAY
//terminals with value
%token<i> NONNEGATIVEINT
%token<s> IDENTIFIER
%token '(' ')' '[' ']' '{' '}' '=' ',' ';' '.' 
%token ADD MINUS TIMES DIVIDE EQ NE LT LE GT GE AND OR
//non-terminals, need type information only (not tokens)
%type <program> PROG 
%type <mainMethod> MAINMETHOD 
%type <classDecl> CLASSDECL
%type <classDeclList> CLASSDECLLIST
%type <type> TYPE 
%type <varDecl> VARDECL 
%type <varDeclList> VARDECLLIST
%type <intExp> NUMBER
%type <intExpList> NUMBERLIST NUMBERREST 
%type <methodDecl> METHODDECL
%type <methodDeclList> METHODDECLLIST
%type <formalList> FORMALLIST FORMALREST
%type <stm> STM
%type <stmList> STMLIST
%type <exp> EXP
%type <expList> EXPLIST EXPREST
%type <idExp> ID 

%start PROG
%expect 0

// precedence
%left IF
%left ELSE
%right '='
%left OR
%left AND
%left EQ NE
%left LT LE GT GE
%left ADD MINUS
%left TIMES DIVIDE
%right '!' UMINUS
%left '.' '(' ')' '[' ']'

%%
PROG: MAINMETHOD CLASSDECLLIST 
  { 
#ifdef DEBUG
    cerr << "Program" << endl;
#endif
    result->root = new Program(p, $1, $2);
    //$$ = result->root;
  }
  ;
MAINMETHOD: PUBLIC INT MAIN '(' ')' '{' VARDECLLIST STMLIST '}'
  {
#ifdef DEBUG
    cerr << "MainMethod" << endl;
#endif
    $$ = new MainMethod(p, $7, $8) ;
  }
  ;
TYPE: CLASS ID
  {
#ifdef DEBUG
    cerr << "Type CLASS ID" << endl;
#endif
    $$ = new Type(p, $2);
  }
  |
  INT
  {
#ifdef DEBUG
    cerr << "Type INT" << endl;
#endif
    $$ = new Type(p);
  }
  |
  INT '[' ']'
  {
#ifdef DEBUG
    cerr << "Type INT[]" << endl;
#endif
    Pos *p1 = new Pos(@2.sline, @2.scolumn, @2.eline, @2.ecolumn);
    $$ = new Type(p, new IntExp(p1, 0)); // 0 means array without arity
  }
  |
  INT '[' NUMBER ']'
  {
#ifdef DEBUG
    cerr << "Type [number]" << endl;
#endif
    $$ = new Type(p, $3);
  }
 ; 
VARDECLLIST: // empty
  {
#ifdef DEBUG
    cerr << "VARDECLLIST emtpy" << endl;  
#endif
    $$ = new vector<VarDecl*>();
  }
  |
  VARDECL VARDECLLIST
  {
#ifdef DEBUG
    cerr << "VARDECL VARDECLLIST" << endl;
#endif
    vector<VarDecl*> *v = $2;
    v->push_back($1);
    rotate(v->begin(), v->end()-1, v->end());
    $$ = v;
  }
  ;
VARDECL: TYPE ID ';'
  {
#ifdef DEBUG
    cerr << "VarDecl TYPE ID" << endl;
#endif
    $$ = new VarDecl(p, $1, $2);
  }
  |
  TYPE ID '=' '{' NUMBERLIST '}' ';'
  {
#ifdef DEBUG
    cerr << "VarDecl TYPE ID = {NUMBERLIST}" << endl;
#endif
    $$ = new VarDecl(p, $1, $2, $5); 
  }
  ;
NUMBER: NONNEGATIVEINT
  {
#ifdef DEBUG
    cerr << "Number: NONNEGATIVEINT: " << $1 << endl;
#endif
    $$ = new IntExp(p, $1);
  }
  |
  MINUS NONNEGATIVEINT
  {
#ifdef DEBUG
    cerr << "Number: -NONNEGATIVEINT: " << -$2 <<  endl;  
#endif
    $$ = new IntExp(p, -$2);
  }
  ;
NUMBERLIST: // empty 
  {
#ifdef DEBUG
    cerr << "NUMBERLIST empty" << endl;
#endif
    $$ = new vector<IntExp*>();
  }
  |
  NUMBER NUMBERREST
  {
#ifdef DEBUG
    cerr << "NUMBER NUMBERREST" << endl;
#endif
    vector<IntExp*> *v = $2;
    v->push_back($1);
    rotate(v->begin(), v->end() - 1, v->end());
    $$ = v;
  }
  ;
NUMBERREST: // empty
  {
#ifdef DEBUG
    cerr << "NUMBERREST empty" << endl;
#endif
    $$ = new vector<IntExp*>();
  }
  |
  ',' NUMBER NUMBERREST
  {
#ifdef DEBUG
    cerr << "NUMBER NUMBERREST" << endl;
#endif
    vector<IntExp*> *v = $3;
    v->push_back($2);
    rotate(v->begin(), v->end() - 1, v->end());
    $$ = v;
  }
  ;
  CLASSDECLLIST: // empty 
  {
#ifdef DEBUG
    cerr << "CLASSDECLLIST empty" << endl;
#endif
    $$ = new vector<ClassDecl*>();
  }
  |
  CLASSDECL CLASSDECLLIST
  {
#ifdef DEBUG
    cerr << "CLASSDECL CLASSDECLLIST" << endl;
#endif
    vector<ClassDecl*> *v = $2;
    v->push_back($1);
    rotate(v->begin(), v->end() - 1, v->end());
    $$ = v;
  }
  ;
CLASSDECL: PUBLIC CLASS ID '{' VARDECLLIST METHODDECLLIST '}'
  {
#ifdef DEBUG
    cerr << "ClassDecl: public class id { vlist, mlist }" << endl;
#endif
    $$ = new ClassDecl(p, $3, $5, $6);
  }
  |
  PUBLIC CLASS ID EXTENDS ID '{' VARDECLLIST METHODDECLLIST '}'
  {
#ifdef DEBUG
    cerr << "ClassDecl: public class id extends id { vlist, mlist }" << endl;
#endif
    $$ = new ClassDecl(p, $3, $5, $7, $8);
  }
  ;
METHODDECLLIST: // empty 
  {
#ifdef DEBUG
    cerr << "METHODDECLLIST empty" << endl;
#endif
    $$ = new vector<MethodDecl*>();
  }
  |
  METHODDECL METHODDECLLIST
  {
#ifdef DEBUG
    cerr << "METHODDECL METHODDECLLIST" << endl;
#endif
    vector<MethodDecl*> *v = $2;
    v->push_back($1);
    rotate(v->begin(), v->end() - 1, v->end());
    $$ = v;
  }
  ;
METHODDECL: PUBLIC TYPE ID '(' FORMALLIST ')' '{' VARDECLLIST STMLIST '}'
  {
#ifdef DEBUG
    cerr << "MethodDecl: public type id (formallist) { vlist, slist }" << endl;
#endif
    $$ = new MethodDecl(p, $2, $3, $5, $8, $9);
  }
  ;
FORMALLIST: // empty
  {
#ifdef  DEBUG
    cerr << "FORMALLIST empty" << endl;
#endif
    $$ = new vector<Formal*>();
  }
  |
  TYPE ID FORMALREST
  {
#ifdef DEBUG
    cerr << "TYPE ID FORMALREST" << endl;
#endif
    vector<Formal*> *v = $3;
    Pos *p1 = new Pos(@1.sline, @1.scolumn, @2.eline, @2.ecolumn);
    v->push_back(new Formal(p1, $1, $2));
    rotate(v->begin(), v->end() - 1, v->end());
    $$ = v;
  }
  ;
FORMALREST: // empty
  {
#ifdef DEBUG
    cerr << "FORMALREST empty" << endl;
#endif
    $$ = new vector<Formal*>();
  }
  |
  ',' TYPE ID FORMALREST
  {
#ifdef DEBUG
    cerr << "',' TYPE ID FORMALREST" << endl;
#endif
    vector<Formal*> *v = $4;
    Pos *p1 = new Pos(@2.sline, @3.scolumn, @3.eline, @3.ecolumn);
    v->push_back(new Formal(p1, $2, $3));
    rotate(v->begin(), v->end() - 1, v->end());
    $$ = v;
  }
  ;
STMLIST: // empty
  {
#ifdef DEBUG
    cerr << "STMLIST empty" << endl;
#endif
    $$ = new vector<Stm*>();
  }
  |
  STM STMLIST
  {
#ifdef DEBUG
    cerr << "STM STMLIST" << endl;
#endif
    vector<Stm*> *v = $2;
    v->push_back($1);
    rotate(v->begin(), v->end() - 1, v->end());
    $$ = v;
  }
  ;
STM: '{' STMLIST '}'
  {
#ifdef DEBUG
    cerr << "{ sl }" << endl;
#endif
    $$ = new Nested(p, $2);
  }
  |
  IF '(' EXP ')' STM ELSE STM
  {
#ifdef DEBUG
    cerr << "If else" << endl;
#endif
    $$ = new If(p, $3, $5, $7);
  }
  |
  IF '(' EXP ')' STM %prec IF
  {
#ifdef DEBUG
    cerr << "If no else" << endl;
#endif
    $$ = new If(p, $3, $5);
  }
  |
  WHILE '(' EXP ')' STM
  {
#ifdef DEBUG
    cerr << "While stm" << endl;
#endif
    $$ = new While(p, $3, $5);
  }
  |
  WHILE '(' EXP ')' ';'
  {
#ifdef DEBUG
    cerr << "While no stm" << endl;
#endif
    $$ = new While(p, $3);
  }
  |
  EXP '=' EXP ';'
  {
#ifdef DEBUG
    cerr << "Assign" << endl;
#endif
    $$ = new Assign(p, $1, $3);
  }
  |
  EXP '.' ID '(' EXPLIST ')' ';'
  {
#ifdef DEBUG
    cerr << "CallStm" << endl;
#endif
    $$ = new CallStm(p, $1, $3, $5);
  }
  |
  CONTINUE ';'
  {
#ifdef DEBUG
    cerr << "Continue" << endl;
#endif
    $$ = new Continue(p);
  }
  |
  BREAK ';'
  {
#ifdef DEBUG
    cerr << "Break" << endl;
#endif
    $$ = new Break(p);
  }
  |
  RETURN EXP ';'
  {
#ifdef DEBUG
    cerr << "Return" << endl;
#endif
    $$ = new Return(p, $2);
  }
  |
  PUTINT '(' EXP ')' ';'
  {
#ifdef DEBUG
    cerr << "PutInt" << endl;
#endif
    $$ = new PutInt(p, $3);
  }
  |
  PUTCH '(' EXP ')' ';'
  {
#ifdef DEBUG
    cerr << "PutCh" << endl;
#endif
    $$ = new PutCh(p, $3);
  }
  |
  PUTARRAY '(' EXP ',' EXP ')' ';'
  {
#ifdef DEBUG
    cerr << "PutArray" << endl;
#endif
    $$ = new PutArray(p, $3, $5);
  }
  |
  STARTTIME '(' ')' ';'
  {
#ifdef DEBUG
    cerr << "StartTime" << endl;
#endif
    $$ = new Starttime(p);
  }
  |
  STOPTIME '(' ')' ';'
  {
#ifdef DEBUG
    cerr << "StopTime" << endl;
#endif
    $$ = new Stoptime(p);
  }
  ;
EXP: EXP ADD EXP
  {
#ifdef DEBUG
    cerr << "EXP ADD EXP" << endl;
#endif
    Pos *p1 = new Pos(@2.sline, @2.scolumn, @2.eline, @2.ecolumn);
    $$ = new BinaryOp(p, $1, new OpExp(p1, "+"), $3);
  }
  |
  EXP MINUS EXP
  {
#ifdef DEBUG
    cerr << "EXP MINUS EXP" << endl;
#endif
    Pos *p1 = new Pos(@2.sline, @2.scolumn, @2.eline, @2.ecolumn);
    $$ = new BinaryOp(p, $1, new OpExp(p1, "-"), $3);
  }
  |
  EXP TIMES EXP
  {
#ifdef DEBUG
    cerr << "EXP TIMES EXP" << endl;
#endif
    Pos *p1 = new Pos(@2.sline, @2.scolumn, @2.eline, @2.ecolumn);
    $$ = new BinaryOp(p, $1, new OpExp(p1, "*"), $3);
  }
  |
  EXP DIVIDE EXP
  {
#ifdef DEBUG
    cerr << "EXP DIVIDE EXP" << endl;
#endif
    Pos *p1 = new Pos(@2.sline, @2.scolumn, @2.eline, @2.ecolumn);
    $$ = new BinaryOp(p, $1, new OpExp(p1, "/"), $3);
  }
  |
  EXP OR EXP
  {
#ifdef DEBUG
    cerr << "EXP OR EXP" << endl;
#endif
    Pos *p1 = new Pos(@2.sline, @2.scolumn, @2.eline, @2.ecolumn);
    $$ = new BinaryOp(p, $1, new OpExp(p1, "||"), $3);
  }
  |
  EXP AND EXP
  {
#ifdef DEBUG
    cerr << "EXP AND EXP" << endl;
#endif
    Pos *p1 = new Pos(@2.sline, @2.scolumn, @2.eline, @2.ecolumn);
    $$ = new BinaryOp(p, $1, new OpExp(p1, "&&"), $3);
  }
  |
  EXP LT EXP
  {
#ifdef DEBUG
    cerr << "EXP LT EXP" << endl;
#endif
    Pos *p1 = new Pos(@2.sline, @2.scolumn, @2.eline, @2.ecolumn);
    $$ = new BinaryOp(p, $1, new OpExp(p1, "<"), $3);
  }
  |
  EXP LE EXP
  {
#ifdef DEBUG
    cerr << "EXP LE EXP" << endl;
#endif
    Pos *p1 = new Pos(@2.sline, @2.scolumn, @2.eline, @2.ecolumn);
    $$ = new BinaryOp(p, $1, new OpExp(p1, "<="), $3);
  }
  |
  EXP GT EXP
  {
#ifdef DEBUG
    cerr << "EXP GT EXP" << endl;
#endif
    Pos *p1 = new Pos(@2.sline, @2.scolumn, @2.eline, @2.ecolumn);
    $$ = new BinaryOp(p, $1, new OpExp(p1, ">"), $3);
  }
  |
  EXP GE EXP
  {
#ifdef DEBUG
    cerr << "EXP GE EXP" << endl;
#endif
    Pos *p1 = new Pos(@2.sline, @2.scolumn, @2.eline, @2.ecolumn);
    $$ = new BinaryOp(p, $1, new OpExp(p1, ">="), $3);
  }
  |
  EXP EQ EXP
  {
#ifdef DEBUG
    cerr << "EXP EQ EXP" << endl;
#endif
    Pos *p1 = new Pos(@2.sline, @2.scolumn, @2.eline, @2.ecolumn);
    $$ = new BinaryOp(p, $1, new OpExp(p1, "=="), $3);
  }
  |
  EXP NE EXP
  {
#ifdef DEBUG
    cerr << "EXP NE EXP" << endl;
#endif
    Pos *p1 = new Pos(@2.sline, @2.scolumn, @2.eline, @2.ecolumn);
    $$ = new BinaryOp(p, $1, new OpExp(p1, "!="), $3);
  }
  |
  EXP '[' EXP ']'
  {
#ifdef DEBUG
    cerr << "EXP [ EXP ]" << endl;
#endif
    $$ = new ArrayExp(p, $1, $3);
  }
  |
  EXP '.' ID '(' EXPLIST ')'
  {
#ifdef DEBUG
    cerr << "EXP . ID ( EXPLIST )" << endl;
#endif
    $$ = new CallExp(p, $1, $3, $5);
  }
  |
  EXP '.' ID
  {
#ifdef DEBUG
    cerr << "EXP . ID" << endl;
#endif
    $$ = new ClassVar(p, $1, $3);
  }
  |
  NONNEGATIVEINT
  {
#ifdef DEBUG
    cerr << "NonNegativeInt: " << $1 << endl;
#endif
    $$ = new IntExp(p, $1);
  }
  |
  LENGTH '(' EXP ')'
  {
#ifdef DEBUG
    cerr << "Length" << endl;
#endif
    $$ = new Length(p, $3);
  }
  |
  THIS
  {
#ifdef DEBUG
    cerr << "This" << endl;
#endif
    $$ = new This(p);
  }
  |
  '!' EXP
  {
#ifdef DEBUG
    cerr << "! EXP" << endl;
#endif
    Pos *p1 = new Pos(@1.sline, @1.scolumn, @1.eline, @1.ecolumn);
    $$ = new UnaryOp(p, new OpExp(p1, "!"), $2);
  }
  |
  MINUS EXP %prec UMINUS
  {
#ifdef DEBUG
    cerr << "- EXP" << endl;
#endif
    Pos *p1 = new Pos(@1.sline, @1.scolumn, @1.eline, @1.ecolumn);
    $$ =  new UnaryOp(p, new OpExp(p1, "-"), $2);
  }
  |
  '(' EXP ')'
  {
#ifdef DEBUG
    cerr << "( EXP )" << endl;
#endif
    $$ = $2;
  }
  |
  NEW INT '[' EXP ']'
  {
#ifdef DEBUG
    cerr << "NEW INT [ EXP ]" << endl;
#endif
    $$ = new NewArray(p, $4);
  }
  |
  NEW ID '(' ')'
  {
#ifdef DEBUG
    cerr << "NEW ID ( )" << endl;
#endif
    $$ = new NewObject(p, $2);
  }
  |
  GETINT '(' ')'
  {
#ifdef DEBUG
    cerr << "GetInt" << endl;
#endif
    $$ = new GetInt(p);
  }
  |
  GETCH '(' ')'
  {
#ifdef DEBUG
    cerr << "GetCh" << endl;
#endif
    $$ = new GetCh(p);
  }
  |
  GETARRAY '(' EXP ')'
  {
#ifdef DEBUG
    cerr << "GetArray" << endl;
#endif
    $$ = new GetArray(p, $3);
  }
  |
  ID
  {
#ifdef DEBUG
    cerr << "ID" << endl;
#endif
    $$ = $1;
  }
  ;
EXPLIST: // empty 
  {
#ifdef DEBUG
    cerr << "EXPLIST empty" << endl;
#endif
    $$ = new vector<Exp*>();
  }
  |
  EXP EXPREST
  {
#ifdef DEBUG
    cerr << "EXP EXPREST" << endl;
#endif
    vector<Exp*> *v = $2;
    v->push_back($1);
        rotate(v->begin(), v->end() - 1, v->end());
    $$ = v;
  }
  ;
EXPREST: // empty 
  {
#ifdef DEBUG
    cerr << "EXPREST empty" << endl;
#endif
    $$ = new vector<Exp*>();
  }
  |
  ',' EXP EXPREST
  {
#ifdef DEBUG
    cerr << "EXP EXPREST" << endl;
#endif
    vector<Exp*> *v = $3;
    v->push_back($2);
    rotate(v->begin(), v->end() - 1, v->end());
    $$ = v;
  }
  ;
ID: IDENTIFIER
  {
#ifdef DEBUG
    cerr << "Identifier: " << $1 << endl;
#endif
    $$ = new IdExp(p, $1);
  }
  ;

%%
/*
void yyerror(char *s) {
  fprintf(stderr, "%s\n",s);
}

int yywrap() {
  return(1);
}
*/

//%code 
namespace fdmj 
{
    template<typename RHS>
    inline void calcLocation(location_t &current, const RHS &rhs, const std::size_t n)
    {
        current = location_t(YYRHSLOC(rhs, 1).sline, YYRHSLOC(rhs, 1).scolumn, 
                                    YYRHSLOC(rhs, n).eline, YYRHSLOC(rhs, n).ecolumn);
        p = new Pos(current.sline, current.scolumn, current.eline, current.ecolumn);
    }
    
    void ASTParser::error(const location_t &location, const std::string &message)
    {
        std::cerr << "Error at lines " << location << ": " << message << std::endl;
    }

  Program* fdmjParser(ifstream &fp, const bool debug) {
    fdmj:AST_YYSTYPE result; 
    result.root = nullptr;
    fdmj::ASTLexer lexer(fp, debug);
    fdmj::ASTParser parser(lexer, debug, &result); //set up the parser
    if (parser() ) { //call the parser
      cout << "Error: parsing failed" << endl;
      return nullptr;
    }
    if (debug) cout << "Parsing successful" << endl;
    return result.root;
  }

  Program*  fdmjParser(const string &filename, const bool debug) {
    std::ifstream fp(filename);
    if (!fp) {
      cout << "Error: cannot open file " << filename << endl;
      return nullptr;
    }
    return fdmjParser(fp, debug);
  }
}
