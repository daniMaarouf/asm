all: asm

asm: main.c
	gcc -std=c89 -Wall main.c
