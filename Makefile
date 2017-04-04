CC = gcc
SRC_FILES = src/main.c src/tokenData.c src/tokenIdentify.c src/codeGeneration.c
HEADER_FILES = include/tokenData.h include/tokenIdentify.h include/codeGeneration.h
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -Iinclude

all: assembler

assembler: $(SRC_FILES) $(HEADER_FILES)
	$(CC) $(CFLAGS) $(SRC_FILES) -o asm

win: $(SRC_FILES) $(HEADER_FILES)
	x86_64-w64-mingw32-gcc $(CFLAGS) $(SRC_FILES) -o asm.exe


clean:
	rm -f asm