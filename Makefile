jeval:	jeval_main.c ops.c variable.c
	gcc -o jeval jeval_main.c ops.c variable.c -lncurses -lpthread -g
