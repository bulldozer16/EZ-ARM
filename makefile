ezarm: lex.yy.c asm.tab.c sim.tab.c ezarm_gui.c
	gcc -g -w lex.yy.c asm.tab.c -o ezasm
	gcc -g -w lex.yy.c sim.tab.c -o ezsim
	gcc -g -w -Wall ezarm_gui.c -o ezarm `pkg-config --cflags --libs gtk+-2.0`

asm.tab.c: ezasm.y
	yacc -b asm -d ezasm.y

sim.tab.c: ezsim.y
	yacc -b sim -d ezsim.y

lex.yy.c: asm.tab.c ezarm.l
	lex ezarm.l

clean: 
	rm -f lex.yy.c asm.tab.c sim.tab.c asm.tab.h sim.tab.h ezasm ezsim out.txt ezarm
