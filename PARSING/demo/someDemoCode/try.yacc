%{
#include <stdio.h>

extern int yylex(void);
extern void yyerror(const char *s);
extern int yywrap(void);

%}

%start list
%token ONE TWO EOL

%%                   /* beginning of rules section */

list: 
|
ONE ONE EOL
         {
           printf("Got it!\n"); 
         }

;

%%
int main(void)
{
  return yyparse();
}

void yyerror(const char *s)
{
  fprintf(stderr, "%s\n", s);
}

int yywrap(void)
{
  return 1;
}
