CC = gcc
SRC_FILES = src/main.c src/tokenData.c src/tokenIdentify.c
HEADER_FILES = include/tokenData.h include/tokenIdentify.h
CFLAGS = -Wall -std=c99 -Iinclude

all: asm

asm: $(SRC_FILES) $(SRC_FILES)
	$(CC) $(CFLAGS) $(SRC_FILES) -o asm

clean:
	rm -f asm