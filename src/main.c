#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "tokenData.h"
#include "tokenIdentify.h"

static void printUsageInfo(const char * binName);

/*
    when tokens reach this point they can be a
    linear sequence of tokens which have valid
    forms. This function will ensure that the
    way the tokens are arranged with respect
    to each other is valid and will fill in
    fields for the data structure representing
    each token
*/
bool fillTokenFields(struct LinkedToken * tokens) {
    if (tokens == NULL) {
        return false;
    }

    while (tokens != NULL) {
        if (tokens->tokenText == NULL) {
            break;
        }

        switch(tokens->tokenType) {

            case COMMENT: {
                if (tokens->next == NULL || (tokens->next->tokenType == NONE
                    && tokens->next->next == NULL)) {
                    /* comment then end of file */
                    return true;
                } else if (tokens->next->tokenType == LABEL
                    || tokens->next->tokenType == INSTRUCTION) {
                    tokens = tokens->next;
                    break;
                } else {
                    if (tokens->next->tokenText != NULL) {
                        printf("This type of token: %s\nshould not appear after a comment\n", 
                            tokens->next->tokenText);
                        printf("Preceding comment: %s\n", tokens->tokenText);
                    } else {
                        printf("Invalid token after comment: %s\n", tokens->tokenText);
                    }
                    return false;
                }
                break;
            }

            case LABEL: {
                if (tokens->next == NULL || (tokens->next->tokenType == NONE
                    && tokens->next->next == NULL)) {
                    printf("Can't have label at end of file. It should precede instruction\n");
                    return false;
                } else if (tokens->next->tokenType == LABEL
                    || tokens->next->tokenType == INSTRUCTION) {
                    tokens = tokens->next;
                    break;
                } else {
                    if (tokens->next->tokenText != NULL) {
                        printf("This type of token: %s\nshould not appear after a label\n", 
                            tokens->next->tokenText);
                        printf("Preceding label: %s\n", tokens->tokenText);
                    } else {
                        printf("Invalid token after label: %s\n", tokens->tokenText);
                    }
                    return false;
                }

                break;
            }

            case INSTRUCTION: {

                /* now make sure that surrounding tokens are organized
                in way that is valid for the specific instruction */

                switch(tokens->instructionType) {

                    /* no operands */
                    case I_NOP:

                    break;

                    /* one register */
                    case I_CLEAR:
                    case I_INC:
                    case I_DEC:

                    break;

                    /* register then register or literal or offset */
                    case I_LW:
                    case I_SW:
                    case I_SLL:
                    case I_SRL:
                    case I_OUT:
                    

                    break;


                    /* one register or int literal or label operand */
                    case I_LLO:
                    case I_LHI:
                    case I_LOAD:
                    case I_JMP:
                    case I_PUSH:
                    case I_POP:
                    case I_NOT:

                    break;

                    /* register then register or literal
                    then register or literal */
                    case I_BEQ:
                    case I_BNE:
                    case I_BLT:
                    case I_BGT:
                    case I_BGE:
                    case I_BLE:

                    break;

                    /* register then register then register or literal */
                    case I_AND:
                    case I_OR:
                    case I_XOR:
                    case I_SLT:
                    case I_UADD:
                    case I_SADD:
                    case I_SSUB:
                    case I_USUB:
                    case I_MUL:
                    case I_DIV:
                    case I_REM:

                    break;

                    default:
                    printf("Fatal error. Malformed instruction %s\n", tokens->tokenText);
                    return false;
                }

                break;
            }

            case REGISTER:
            case DECIMAL_LITERAL:
            case HEX_LITERAL:
            case BIN_LITERAL:
            case OCTAL_LITERAL:
            case OFFSET:
            printf("Lone operand '%s'\n", tokens->tokenText);
            return false;
            
            case STRING_LITERAL:
            case NONE:
            default:
            printf("Illegal token '%s'\n", tokens->tokenText);
            return false;

        }
    }

    return true;
}

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

    if (!fillTokenFields(tokens)) {
        printf("Invalid organization of tokens\n");
        destroyTokens(tokens);
        return 1;
    }

    destroyTokens(tokens);
    return 0;
}

static void printUsageInfo(const char * binName) {
    printf("\nUsage:\n");
    printf("\t%s *.asm\n", binName);
    return;
}

