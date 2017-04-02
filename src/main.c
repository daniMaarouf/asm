#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "tokenData.h"
#include "tokenIdentify.h"

static void printUsageInfo(const char * binName);

int regNumber(const char * regString) {
    if (regString == NULL) {
        return -1;
    }
    if (strcmp(regString, "$r0") == 0) {
        return 0;
    } else if (strcmp(regString, "$r1") == 0) {
        return 1;
    } else if (strcmp(regString, "$r2") == 0) {
        return 2;
    } else if (strcmp(regString, "$r3") == 0) {
        return 3;
    } else if (strcmp(regString, "$r4") == 0) {
        return 4;
    } else if (strcmp(regString, "$r5") == 0) {
        return 5;
    } else if (strcmp(regString, "$r6") == 0) {
        return 6;
    } else if (strcmp(regString, "$r7") == 0) {
        return 7;
    } else if (strcmp(regString, "$zero") == 0) {
        return 8;
    } else if (strcmp(regString, "$sw") == 0) {
        return 9;
    } else if (strcmp(regString, "$led") == 0) {
        return 10;
    } else if (strcmp(regString, "$psw") == 0) {
        return 11;
    } else {
        return -1;
    }
}

int decodeNum(const char * numStr, bool * valid) {
    if (numStr == NULL || valid == NULL) {
        return -1;
    }

    *valid = true;

    int numLen = strlen(numStr);
    if (numLen == 0) {
        *valid = false;
        return -1;
    }

    bool neg = (numStr[0] == '-');
    if (neg && numLen <= 1) {
        *valid = false;
        return -1;
    }

    if (numLen < 3 + neg) {
        int total = 0;
        int weight = 1;
        int i;
        for (i = numLen - 1; i >= neg; i--) {
            if (!(numStr[i] >= '0' && numStr[i] <= '9')) {
                *valid = false;
                return -1;
            }
            total += weight * (numStr[i] - '0');
            weight *= 10;
        }
        if (neg) {
            return -total;
        } else {
            return total;
        }
    } else {

        int total = 0;
        int weight = 0;
        int i;

        if (numStr[neg + 1] == 'x') {
            weight = 16;
            for (i = numLen - 1; i >= 2 + neg; i--) {
                if (!((numStr[i] >= '0' && numStr[i] <= '9') 
                    || (numStr[i] >= 'a' && numStr[i] <= 'f'))) {
                    *valid = false;
                    return -1;
                }
                if (numStr[i] >= '0' && numStr[i] <= '9') {
                    total += weight * (numStr[i] - '0');
                } else {
                    total += weight * ((numStr[i] - 'a') + 10);
                }
                weight *= 16;
            }
        } else if (numStr[neg + 1] == 'o') {
            weight = 8;
            for (i = numLen - 1; i >= 2 + neg; i--) {
                if (!(numStr[i] >= '0' && numStr[i] <= '7')) {
                    *valid = false;
                    return -1;
                }
                total += weight * (numStr[i] - '0');
                weight *= 8;
            }
        } else if (numStr[neg + 1] == 'b') {
            weight = 2;
            for (i = numLen - 1; i >= 2 + neg; i--) {
                if (!(numStr[i] >= '0' && numStr[i] <= '1')) {
                    *valid = false;
                    return -1;
                }
                total += weight * (numStr[i] - '0');
                weight *= 2;
            }
            
        } else {
            weight = 10;
            for (i = numLen - 1; i >= neg; i--) {
                if (!(numStr[i] >= '0' && numStr[i] <= '9')) {
                    *valid = false;
                    return -1;
                }
                total += weight * (numStr[i] - '0');
                weight *= 10;
            }
        }

        if (neg) {
            return -total;
        } else {
            return total;
        }
    }
}

/*
    when tokens reach this point they can be a
    linear sequence of tokens which have valid
    forms. This function will ensure that the
    way the tokens are arranged with respect
    to each other is valid and will fill in
    fields for the data structure representing
    each token
*/
bool fillInstructionFields(struct LinkedToken * tokens) {
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

                /* TODO: point preceeding labels to instruction */

                /* now make sure that surrounding tokens are organized
                in way that is valid for the specific instruction */
                switch(tokens->instructionType) {

                    /* no operands */
                    case I_NOP: {
                        tokens = tokens->next;
                        break;
                    }

                    /* one register */
                    case I_CLEAR: case I_INC: case I_DEC: 
                    /* one register or int literal or label operand */
                    case I_JMP: case I_PUSH: case I_POP: {
                        if (tokens->next == NULL || (tokens->next->tokenType == NONE
                    && tokens->next->next == NULL)) {
                            printf("Instruction %s is missing its operand\n", tokens->tokenText);
                            return false;
                        } else if (tokens->next->tokenType == REGISTER) {

                            int regNum = regNumber(tokens->next->tokenText);
                            if (regNum == -1) {
                                printf("Could not get register number for %s instruction\n", tokens->tokenText);
                                return false;
                            }
                            tokens->next->registerNum = regNum;
                            tokens->operandOne = tokens->next;
                            tokens = tokens->next->next;
                            break;
                        } else if ((tokens->instructionType == I_JMP
                            || tokens->instructionType == I_PUSH
                            || tokens->instructionType == I_POP) 
                            && 
                            (tokens->next->tokenType == DECIMAL_LITERAL
                                || tokens->next->tokenType == HEX_LITERAL
                                || tokens->next->tokenType == BIN_LITERAL
                                || tokens->next->tokenType == OCTAL_LITERAL)) {

                            bool valid = false;
                            int literalNum = decodeNum(tokens->next->tokenText, &valid);
                            if (!valid) {
                                printf("Could not decode operand for %s instruction\n", tokens->tokenText);
                                return false;
                            }
                            tokens->next->intValue = literalNum;
                            tokens->operandOne = tokens->next;
                            tokens = tokens->next->next;
                            break;
                        } else {
                            printf("Instruction %s has an invalid operand\n", tokens->tokenText);
                            return false;
                        }

                        break;
                    }

                    /* register then register or literal or offset */
                    case I_LW: case I_SW: case I_SLL: case I_SRL: case I_OUT: 
                    case I_LLO: case I_LHI: case I_LOAD: case I_NOT: {
                        if ((tokens->next == NULL || (tokens->next->tokenType == NONE
                    && tokens->next->next == NULL))) {
                            printf("Instruction %s is missing its first operand\n", tokens->tokenText);
                            return false;
                        } else if ((tokens->next->next == NULL || (tokens->next->next->tokenType == NONE
                    && tokens->next->next->next == NULL))) {
                            printf("Instruction %s is missing its second operand\n", tokens->tokenText);
                            return false;
                        } else if (tokens->next->tokenType == REGISTER) {

                            int regNum = regNumber(tokens->next->tokenText);
                            if (regNum == -1) {
                                printf("Could not get register number for first operand in %s instruction\n", tokens->tokenText);
                                return false;
                            }
                            tokens->next->registerNum = regNum;
                            tokens->operandOne = tokens->next;

                            if (tokens->next->next->tokenType == REGISTER) {

                                regNum = regNumber(tokens->next->next->tokenText);
                                if (regNum == -1) {
                                    printf("Could not get register number for second operand in %s instruction\n", tokens->tokenText);
                                    return false;
                                }
                                tokens->next->next->registerNum = regNum;
                                tokens->operandTwo = tokens->next->next;
                                tokens = tokens->next->next->next;
                                break;

                            } else if (tokens->next->next->tokenType == DECIMAL_LITERAL
                                || tokens->next->next->tokenType == HEX_LITERAL
                                || tokens->next->next->tokenType == BIN_LITERAL
                                || tokens->next->next->tokenType == OCTAL_LITERAL) {

                                bool valid = false;
                                int literalNum = decodeNum(tokens->next->next->tokenText, &valid);
                                if (!valid) {
                                    printf("Could not decode second operand for %s instruction\n", tokens->tokenText);
                                    return false;
                                }
                                tokens->next->next->intValue = literalNum;
                                tokens->operandTwo = tokens->next->next;
                                tokens = tokens->next->next->next;
                                break;

                            } else if (((tokens->instructionType == I_LW 
                                || tokens->instructionType == I_SW))
                                && (tokens->next->next->tokenType == OFFSET)) {

                                int opening = -1;
                                int i;
                                for (i = 0; i < tokens->next->next->textSize; i++) {
                                    if (tokens->next->next->tokenText[i] == '(') {
                                        opening = i;
                                        break;
                                    }
                                }

                                if (opening == -1 || opening + 1 >= tokens->next->next->textSize) {
                                    printf("Instruction %s has a malformed second operand\n", tokens->tokenText);
                                    return false;
                                }

                                char * decimalPart = tokens->next->next->tokenText;
                                char * registerPart = &(tokens->next->next->tokenText[opening + 1]);
                                decimalPart[opening] = '\0';
                                decimalPart[tokens->next->next->textSize - 1] = '\0';

                                int regNum = regNumber(registerPart);
                                if (regNum == -1) {
                                    printf("Instruction %s has an invalid second operand (offset)\n", tokens->tokenText);
                                    return false;
                                }
                                tokens->next->next->registerNum = regNum;

                                bool valid = false;
                                int literalNum = decodeNum(decimalPart, &valid);
                                if (!valid) {
                                    printf("Could not decode second operand for %s instruction (offset)\n", tokens->tokenText);
                                    return false;
                                }
                                tokens->next->next->intValue = literalNum;
                                tokens->operandTwo = tokens->next->next;
                                tokens = tokens->next->next->next;
                                break;
                            } else {
                                printf("Instruction %s has an invalid second operand\n", tokens->tokenText);
                                return false;
                            }

                        } else {
                            printf("Instruction %s has an invalid first operand\n", tokens->tokenText);
                            return false;
                        }

                        break;
                    }

                    

                    /* register then register then register or literal */
                    case I_AND: case I_OR: case I_XOR: case I_SLT: case I_UADD: case I_SADD:
                    case I_SSUB: case I_USUB: case I_MUL: case I_DIV: case I_REM: 
                    /* register then register or literal then register or literal */
                    case I_BEQ: case I_BNE: case I_BLT: case I_BGT: case I_BGE: case I_BLE: {



                        break;
                    }

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

    if (!fillInstructionFields(tokens)) {
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

