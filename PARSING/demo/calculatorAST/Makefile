# Default target
calc: lex.yy.o y.tab.o calc.o util.o printast.o
	cc -o calc calc.o printast.o util.o y.tab.o lex.yy.o

# Version-specific targets
calc0: YACC_FILE = calc0.yacc
calc0: calc0_lex.yy.o calc0_y.tab.o calc.o util.o printast.o
	cc -o calc0 calc.o printast.o util.o calc0_y.tab.o calc0_lex.yy.o

calc1: YACC_FILE = calc1.yacc
calc1: calc1_lex.yy.o calc1_y.tab.o calc.o util.o printast.o
	cc -o calc1 calc.o printast.o util.o calc1_y.tab.o calc1_lex.yy.o

calc2: YACC_FILE = calc2.yacc
calc2: calc2_lex.yy.o calc2_y.tab.o calc.o util.o printast.o
	cc -o calc2 calc.o printast.o util.o calc2_y.tab.o calc2_lex.yy.o

calc3: YACC_FILE = calc3.yacc
calc3: calc3_lex.yy.o calc3_y.tab.o calc.o util.o printast.o
	cc -o calc3 calc.o printast.o util.o calc3_y.tab.o calc3_lex.yy.o

calc4: YACC_FILE = calc4.yacc
calc4: calc4_lex.yy.o calc4_y.tab.o calc.o util.o printast.o
	cc -o calc4 calc.o printast.o util.o calc4_y.tab.o calc4_lex.yy.o

calc5: YACC_FILE = calc5.yacc
calc5: calc5_lex.yy.o calc5_y.tab.o calc.o util.o printast.o
	cc -o calc5 calc.o printast.o util.o calc5_y.tab.o calc5_lex.yy.o

calc6: YACC_FILE = calc6.yacc
calc6: calc6_lex.yy.o calc6_y.tab.o calc.o util.o printast.o
	cc -o calc6 calc.o printast.o util.o calc6_y.tab.o calc6_lex.yy.o

# Build all versions
all: calc calc0 calc1 calc2 calc3 calc4 calc5 calc6

# Object files compilation
calc.o: calc.c calc.h util.h
	cc -g -c calc.c

printast.o: printast.c calc.h
	cc -g -c printast.c

util.o: util.c util.h
	cc -g -c util.c

# Default lex and yacc
lex.yy.c: calc.lex y.tab.h y.tab.c
	lex calc.lex
	
y.tab.c: calc.yacc calc.h 
	yacc -d -v calc.yacc
	
y.tab.h: calc.yacc
	yacc -d -v calc.yacc

lex.yy.o: lex.yy.c y.tab.h
	cc -g -c lex.yy.c
	
y.tab.o: y.tab.c
	cc -g -c y.tab.c

# Version-specific lex files
calc0_lex.yy.c: calc.lex calc0_y.tab.h calc0_y.tab.c
	lex -Pcalc0_ calc.lex && mv lex.yy.c calc0_lex.yy.c

calc1_lex.yy.c: calc.lex calc1_y.tab.h calc1_y.tab.c
	lex -Pcalc1_ calc.lex && mv lex.yy.c calc1_lex.yy.c

calc2_lex.yy.c: calc.lex calc2_y.tab.h calc2_y.tab.c
	lex -Pcalc2_ calc.lex && mv lex.yy.c calc2_lex.yy.c

calc3_lex.yy.c: calc.lex calc3_y.tab.h calc3_y.tab.c
	lex -Pcalc3_ calc.lex && mv lex.yy.c calc3_lex.yy.c

calc4_lex.yy.c: calc.lex calc4_y.tab.h calc4_y.tab.c
	lex -Pcalc4_ calc.lex && mv lex.yy.c calc4_lex.yy.c

calc5_lex.yy.c: calc.lex calc5_y.tab.h calc5_y.tab.c
	lex -Pcalc5_ calc.lex && mv lex.yy.c calc5_lex.yy.c

calc6_lex.yy.c: calc.lex calc6_y.tab.h calc6_y.tab.c
	lex -Pcalc6_ calc.lex && mv lex.yy.c calc6_lex.yy.c

# Version-specific yacc files
calc0_y.tab.c: calc0.yacc calc.h
	yacc -pcalc0 -d -v calc0.yacc && mv y.tab.c calc0_y.tab.c && mv y.tab.h calc0_y.tab.h && mv y.output calc0_y.output

calc0_y.tab.h: calc0.yacc
	yacc -pcalc0 -d -v calc0.yacc && mv y.tab.c calc0_y.tab.c && mv y.tab.h calc0_y.tab.h && mv y.output calc0_y.output

calc1_y.tab.c: calc1.yacc calc.h
	yacc -pcalc1 -d -v calc1.yacc && mv y.tab.c calc1_y.tab.c && mv y.tab.h calc1_y.tab.h && mv y.output calc1_y.output

calc1_y.tab.h: calc1.yacc
	yacc -pcalc1 -d -v calc1.yacc && mv y.tab.c calc1_y.tab.c && mv y.tab.h calc1_y.tab.h && mv y.output calc1_y.output

calc2_y.tab.c: calc2.yacc calc.h
	yacc -pcalc2 -d -v calc2.yacc && mv y.tab.c calc2_y.tab.c && mv y.tab.h calc2_y.tab.h && mv y.output calc2_y.output

calc2_y.tab.h: calc2.yacc
	yacc -pcalc2 -d -v calc2.yacc && mv y.tab.c calc2_y.tab.c && mv y.tab.h calc2_y.tab.h && mv y.output calc2_y.output

calc3_y.tab.c: calc3.yacc calc.h
	yacc -pcalc3 -d -v calc3.yacc && mv y.tab.c calc3_y.tab.c && mv y.tab.h calc3_y.tab.h && mv y.output calc3_y.output

calc3_y.tab.h: calc3.yacc
	yacc -pcalc3 -d -v calc3.yacc && mv y.tab.c calc3_y.tab.c && mv y.tab.h calc3_y.tab.h && mv y.output calc3_y.output

calc4_y.tab.c: calc4.yacc calc.h
	yacc -pcalc4 -d -v calc4.yacc && mv y.tab.c calc4_y.tab.c && mv y.tab.h calc4_y.tab.h && mv y.output calc4_y.output

calc4_y.tab.h: calc4.yacc
	yacc -pcalc4 -d -v calc4.yacc && mv y.tab.c calc4_y.tab.c && mv y.tab.h calc4_y.tab.h && mv y.output calc4_y.output

calc5_y.tab.c: calc5.yacc calc.h
	yacc -pcalc5 -d -v calc5.yacc && mv y.tab.c calc5_y.tab.c && mv y.tab.h calc5_y.tab.h && mv y.output calc5_y.output

calc5_y.tab.h: calc5.yacc
	yacc -pcalc5 -d -v calc5.yacc && mv y.tab.c calc5_y.tab.c && mv y.tab.h calc5_y.tab.h && mv y.output calc5_y.output

calc6_y.tab.c: calc6.yacc calc.h
	yacc -pcalc6 -d -v calc6.yacc && mv y.tab.c calc6_y.tab.c && mv y.tab.h calc6_y.tab.h && mv y.output calc6_y.output

calc6_y.tab.h: calc6.yacc
	yacc -pcalc6 -d -v calc6.yacc && mv y.tab.c calc6_y.tab.c && mv y.tab.h calc6_y.tab.h && mv y.output calc6_y.output

# Object file compilation for version-specific files
calc0_lex.yy.o: calc0_lex.yy.c calc0_y.tab.h
	cc -g -c calc0_lex.yy.c

calc0_y.tab.o: calc0_y.tab.c
	cc -g -c calc0_y.tab.c

calc1_lex.yy.o: calc1_lex.yy.c calc1_y.tab.h
	cc -g -c calc1_lex.yy.c

calc1_y.tab.o: calc1_y.tab.c
	cc -g -c calc1_y.tab.c

calc2_lex.yy.o: calc2_lex.yy.c calc2_y.tab.h
	cc -g -c calc2_lex.yy.c

calc2_y.tab.o: calc2_y.tab.c
	cc -g -c calc2_y.tab.c

calc3_lex.yy.o: calc3_lex.yy.c calc3_y.tab.h
	cc -g -c calc3_lex.yy.c

calc3_y.tab.o: calc3_y.tab.c
	cc -g -c calc3_y.tab.c

calc4_lex.yy.o: calc4_lex.yy.c calc4_y.tab.h
	cc -g -c calc4_lex.yy.c

calc4_y.tab.o: calc4_y.tab.c
	cc -g -c calc4_y.tab.c

calc5_lex.yy.o: calc5_lex.yy.c calc5_y.tab.h
	cc -g -c calc5_lex.yy.c

calc5_y.tab.o: calc5_y.tab.c
	cc -g -c calc5_y.tab.c

calc6_lex.yy.o: calc6_lex.yy.c calc6_y.tab.h
	cc -g -c calc6_lex.yy.c

calc6_y.tab.o: calc6_y.tab.c
	cc -g -c calc6_y.tab.c

clean: 
	rm -f calc calc0 calc1 calc2 calc3 calc4 calc5 calc6
	rm -f lex.yy.o lex.yy.c y.tab.o y.tab.c y.tab.h y.output
	rm -f calc*_lex.yy.o calc*_lex.yy.c calc*_y.tab.o calc*_y.tab.c calc*_y.tab.h calc*_y.output
	rm -f calc.o util.o printast.o

.PHONY: all clean
