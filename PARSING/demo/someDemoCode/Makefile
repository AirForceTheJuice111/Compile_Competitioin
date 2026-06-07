try0: try0.lex
	lex -o try0.c try0.lex
	cc -o try0 try0.c

t: t.lex t.yacc
	yacc t.yacc
	lex -o t.c t.lex
	cc -o t t.c y.tab.c

try: try.lex try.yacc
	yacc -d try.yacc
	lex -o try.c try.lex
	cc -o try try.c y.tab.c

all: t try try0

clean: 
	rm -f try0 try0.c t try t.c try.c y.tab.* y.output

