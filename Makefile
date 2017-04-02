CC = gcc
SRC_FILES = main.c tokenData.c tokenIdentify.c
HEADER_FILES = include/tokenData.h include/tokenIdentify.h
CFLAGS = -Wall -std=c99 -Iinclude

all: asm

asm: $(SRC_FILES) $(SRC_FILES)
	$(CC) $(CFLAGS) $(SRC_FILES) -o asm
