%{
#include "y.tab.h"  /* token definitions from yacc */
extern void yyerror(const char *s);  /* forward declaration */
%}
%%
1       { return ONE; }
2       { return TWO; }
\n      { return EOL; }
[ \t]+ { ; }  /* skip whitespace */
. { yyerror("Unexpected character"); }
%%
