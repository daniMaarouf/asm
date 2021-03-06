#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "tokenData.h"
#include "tokenIdentify.h"
#include "codeGeneration.h"

static void printUsageInfo(const char * binName);

int main(int argc, char ** argv) {
    if (argc != 2) {
        printf("You entered %d arguments\n", argc-1);
        printf("Please call program with one argument\n");
        printUsageInfo(argv[0]);
        return 1;
    }

    const char * arg = argv[1];
    int argLen = strlen(argv[1]);

    if (argLen < 5 
            || arg[argLen - 4] != '.'
            || arg[argLen - 3] != 'a'
            || arg[argLen - 2] != 's'
            || arg[argLen - 1] != 'm') {
        printf("Argument must have .asm suffix\n");
        printUsageInfo(argv[0]);
        return 1;
    }

    /* PHASE 1: tokenizes based on whitespace and commas, ignores whitespace
    and commas for comments. Returns a linked list of tokens which require
    further processing */
    struct LinkedToken * tokens = tokenize(arg);
    if (tokens == NULL) {
        printf("%s could not be processed\n", arg);
        return 1;
    }

    /* PHASE 2: does basic identification of tokens and checks that
    all the tokens have valid forms */
    if (!identifyTokens(tokens)) {
        printf("Tokens could not be identified\n");
        destroyTokens(tokens);
        return 1;
    }

    /* PHASE 3: analyzes the sequence of tokens to determine
    whether they are ordered in a valid way. For example for an
    instruction which expects 3 operands the next 3 tokens in
    the sequence must be tokens which can be used as operands.
    Labels must precede other labels or instructions, etc.

    Simultaneously data in the tokens is being decoded (hence
    the name of the function: fillInstructionFields), the 
    nybble for register tokens is decoded and integer literal
    values are decoded and read into the appropriate LinkedToken struct */
    if (!fillInstructionFields(tokens)) {
        printf("Invalid organization of tokens\n");
        destroyTokens(tokens);
        return 1;
    }

    /* PHASE 4: See comments in generateCode() for more detailed breakdown
    First gets addresses in memory for each instruction, then resolves
    references to labels, then finally generates code and prints it to file */
    if (!generateCode(tokens, arg, 0x4000)) {
        printf("Could not generate binary\n");
        destroyTokens(tokens);
        return 1;
    }
    printf("Assembly was successful\n");
    destroyTokens(tokens);
    return 0;
}

static void printUsageInfo(const char * binName) {
    printf("\nUsage:\n");
    printf("\t%s *.asm\n", binName);
    return;
}

