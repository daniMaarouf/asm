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

    /* tokenizes based on whitespace and commas, ignores whitespace
    and commas for comments. Returns linked list of tokens */
    struct LinkedToken * tokens = tokenize(arg);
    if (tokens == NULL) {
        printf("%s could not be processed\n", arg);
        return 1;
    }

    /* does first stage identification of the tokens and ensures 
    that they match a predefined set of allowable patterns */
    if (!identifyTokens(tokens)) {
        printf("Tokens could not be identified\n");
        destroyTokens(tokens);
        return 1;
    }

    /* parses tokens to make sure they are ordered/organized
    correctly and fills fields in data structures representing
    the instructions with decoded data from token text  */
    if (!fillInstructionFields(tokens)) {
        printf("Invalid organization of tokens\n");
        destroyTokens(tokens);
        return 1;
    }

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

