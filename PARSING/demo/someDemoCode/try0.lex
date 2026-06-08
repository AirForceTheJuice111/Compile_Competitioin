%{

int flag;

%}

/* lex definitions */

%%

a {printf("1"); }

^ba {printf("22"); }

^"--"[^\n]*\n { }  /* skip comments starting with -- at line beginning, including newline */

[ \t]+ { ; }       /* skip spaces and tabs */

. {printf("X");}

"\n" {printf("---\n");}

%%

int yywrap() {return 1;}

int main(void)
           {
               yyin = stdin;
               yylex();
	       return 0;
           }

