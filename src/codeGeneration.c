#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codeGeneration.h"

static int regNumber(const char * regString);
static int decodeNum(const char * numStr, bool * valid);

static void printBinary(uint16_t num) {
    static const char * nybbleStrings[] = {
        "0000","0001","0010","0011","0100","0101","0110","0111",
        "1000","1001","1010","1011","1100","1101","1110","1111"
    };

    printf("%s%s%s%s", 
        nybbleStrings[(num & 0xF000) >> 12], 
        nybbleStrings[(num & 0x0F00) >> 8],
        nybbleStrings[(num & 0x00F0) >> 4],
        nybbleStrings[(num & 0x000F)]);
    return;
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
                        printf("Preceding comment: %s on line %d\n", tokens->tokenText, tokens->lineNum);
                    } else {
                        printf("Invalid token after comment: %s on line %d\n", tokens->tokenText, tokens->lineNum);
                    }
                    return false;
                }
                break;
            }

            case LABEL: {
                if (tokens->next == NULL || (tokens->next->tokenType == NONE
                    && tokens->next->next == NULL)) {
                    printf("Can't have label at end of file. It should precede instruction\n");
                    printf("Label %s on line number %d is invalid\n", tokens->tokenText, tokens->lineNum);
                    return false;
                } else if (tokens->next->tokenType == LABEL
                    || tokens->next->tokenType == INSTRUCTION) {

                    struct LinkedToken * nextInstr = tokens;
                    bool found = false;
                    while (nextInstr != NULL && !found) {
                        if (nextInstr->tokenText == NULL) {
                            printf("Label %s on line number %d is invalid\n",
                                tokens->tokenText, tokens->lineNum);
                            return false;
                        }
                        switch(nextInstr->tokenType) {
                            case INSTRUCTION:
                            tokens->nextInstruction = nextInstr;
                            found = true;
                            break;

                            case LABEL:
                            nextInstr = nextInstr->next;
                            break;

                            default:
                            printf("Invalid token %s after label token %s on line %d\n", 
                                nextInstr->tokenText, tokens->tokenText, tokens->lineNum);
                            return false;
                        } 
                    }

                    if (nextInstr == NULL || found == false) {
                        printf("Label %s on line %d is invalid\n", tokens->tokenText, tokens->lineNum);
                        return false;
                    }

                    tokens = tokens->next;
                    break;
                } else {
                    if (tokens->next->tokenText != NULL) {
                        printf("This type of token: %s\nshould not appear after a label\n", 
                            tokens->next->tokenText);
                        printf("Preceding label: %s on line %d\n", tokens->tokenText, tokens->lineNum);
                    } else {
                        printf("Invalid token after label: %s on line %d\n", tokens->tokenText, tokens->lineNum);
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
                    case I_NOP: {
                        tokens = tokens->next;
                        break;
                    }

                    /* one register */
                    case I_CLEAR: case I_INC: case I_DEC: case I_POP: 
                    case I_NOT: case I_SLL: case I_SRL: 
                    /* one register or int literal */
                    case I_PUSH: case I_OUT:
                    /* one register or int literal or label */
                    case I_JMP: {
                        if (tokens->next == NULL || (tokens->next->tokenType == NONE
                    && tokens->next->next == NULL)) {
                            printf("Instruction %s on line %d is missing its operand\n", tokens->tokenText, tokens->lineNum);
                            return false;
                        } else if (tokens->next->tokenType == REGISTER) {

                            int regNum = regNumber(tokens->next->tokenText);
                            if (regNum == -1) {
                                printf("Could not get register number for %s instruction on line %d\n", tokens->tokenText, tokens->lineNum);
                                return false;
                            }
                            tokens->next->registerNum = regNum;
                            tokens->operandOne = tokens->next;
                            tokens = tokens->next->next;
                            break;
                        } else if ((tokens->instructionType == I_JMP
                            || tokens->instructionType == I_PUSH
                            || tokens->instructionType == I_OUT) 
                            && 
                            (tokens->next->tokenType == DECIMAL_LITERAL
                                || tokens->next->tokenType == HEX_LITERAL
                                || tokens->next->tokenType == BIN_LITERAL
                                || tokens->next->tokenType == OCTAL_LITERAL)) {

                            bool valid = false;
                            int literalNum = decodeNum(tokens->next->tokenText, &valid);
                            if (!valid) {
                                printf("Could not decode operand for %s instruction on line %d\n", tokens->tokenText, tokens->lineNum);
                                return false;
                            }
                            tokens->next->intValue = literalNum;
                            tokens->operandOne = tokens->next;
                            tokens = tokens->next->next;
                            break;
                        } else if ((tokens->instructionType == I_JMP) 
                            && tokens->next->tokenType == IDENTIFIER) {
                            tokens->operandOne = tokens->next;
                            tokens = tokens->next->next;
                            break;
                        } else {
                            printf("Instruction %s on line %d has an invalid operand\n", tokens->tokenText, tokens->lineNum);
                            return false;
                        }

                        break;
                    }

                    /* register then register or literal */
                    case I_LLO: case I_LHI: case I_LOAD: 
                    /* register then register or literal or offset */
                    case I_LW: case I_SW:  {
                        if ((tokens->next == NULL || (tokens->next->tokenType == NONE
                    && tokens->next->next == NULL))) {
                            printf("Instruction %s on line %d is missing its first operand\n", tokens->tokenText, tokens->lineNum);
                            return false;
                        } else if ((tokens->next->next == NULL || (tokens->next->next->tokenType == NONE
                    && tokens->next->next->next == NULL))) {
                            printf("Instruction %s on line %d is missing its second operand\n", tokens->tokenText, tokens->lineNum);
                            return false;
                        } else if (tokens->next->tokenType == REGISTER) {

                            int regNum = regNumber(tokens->next->tokenText);
                            if (regNum == -1) {
                                printf("Could not get register number for first operand in %s instruction on line %d\n", tokens->tokenText, tokens->lineNum);
                                return false;
                            }
                            tokens->next->registerNum = regNum;
                            tokens->operandOne = tokens->next;

                            if (tokens->next->next->tokenType == REGISTER) {

                                regNum = regNumber(tokens->next->next->tokenText);
                                if (regNum == -1) {
                                    printf("Could not get register number for second operand in %s instruction on line %d\n", tokens->tokenText, tokens->lineNum);
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
                                    printf("Could not decode second operand for %s instruction on line %d\n", tokens->tokenText, tokens->lineNum);
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
                                    printf("Instruction %s on line %d has a malformed second operand\n", tokens->tokenText, tokens->lineNum);
                                    return false;
                                }

                                char * decimalPart = tokens->next->next->tokenText;
                                char * registerPart = &(tokens->next->next->tokenText[opening + 1]);
                                decimalPart[opening] = '\0';
                                decimalPart[tokens->next->next->textSize - 1] = '\0';

                                int regNum = regNumber(registerPart);
                                if (regNum == -1) {
                                    printf("Instruction %s on line %d has an invalid second operand (offset)\n", tokens->tokenText, tokens->lineNum);
                                    return false;
                                }
                                tokens->next->next->registerNum = regNum;

                                bool valid = false;
                                int literalNum = decodeNum(decimalPart, &valid);
                                if (!valid) {
                                    printf("Could not decode second operand for %s instruction on line %d (offset)\n", tokens->tokenText, tokens->lineNum);
                                    return false;
                                }
                                tokens->next->next->intValue = literalNum;
                                tokens->operandTwo = tokens->next->next;
                                tokens = tokens->next->next->next;
                                break;
                            } else {
                                printf("Instruction %s on line %d has an invalid second operand\n", tokens->tokenText, tokens->lineNum);
                                return false;
                            }

                        } else {
                            printf("Instruction %s on line %d has an invalid first operand\n", tokens->tokenText, tokens->lineNum);
                            return false;
                        }

                        break;
                    }

                    /* register then register then register or literal */
                    case I_AND: case I_OR: case I_XOR: case I_SLT: case I_UADD: case I_SADD:
                    case I_SSUB: case I_USUB: case I_MUL: case I_DIV: case I_REM: 
                    /* register then register or literal then register or literal or label */
                    case I_BEQ: case I_BNE: case I_BLT: case I_BGT: case I_BGE: case I_BLE: {

                        if (tokens->next == NULL || (tokens->next->tokenType == NONE
                    && tokens->next->next == NULL)) {
                            printf("Instruction %s on line %d is missing its first operand\n", tokens->tokenText, tokens->lineNum);
                            return false;
                        } else if (tokens->next->next == NULL || (tokens->next->next->tokenType == NONE
                    && tokens->next->next->next == NULL)) {
                            printf("Instruction %s on line %d is missing its second operand\n", tokens->tokenText, tokens->lineNum);
                            return false;
                        } else if (tokens->next->next->next == NULL || (tokens->next->next->next->tokenType == NONE
                    && tokens->next->next->next->next == NULL)) {
                            printf("Instruction %s on line %d is missing its third operand\n", tokens->tokenText, tokens->lineNum);
                            return false;
                        } else if (tokens->next->tokenType == REGISTER) {

                            int regNum = regNumber(tokens->next->tokenText);
                            if (regNum == -1) {
                                printf("Could not get register number for first operand of %s instruction on line %d\n", tokens->tokenText, tokens->lineNum);
                                return false;
                            }
                            tokens->next->registerNum = regNum;
                            tokens->operandOne = tokens->next;

                        } else {
                            printf("Instruction %s on line %d has an invalid first operand\n", tokens->tokenText, tokens->lineNum);
                            return false;
                        }

                        if (tokens->next->next->tokenType == REGISTER) {

                            int regNum = regNumber(tokens->next->next->tokenText);
                            if (regNum == -1) {
                                printf("Could not get register number for second operand of %s instructionon line %d\n", tokens->tokenText, tokens->lineNum);
                                return false;
                            }
                            tokens->next->next->registerNum = regNum;
                            tokens->operandTwo = tokens->next->next;

                        } else if ((tokens->instructionType == I_BEQ
                            || tokens->instructionType == I_BNE
                            || tokens->instructionType == I_BLT
                            || tokens->instructionType == I_BGT
                            || tokens->instructionType == I_BGE
                            || tokens->instructionType == I_BLE)
                        && (tokens->next->next->tokenType == DECIMAL_LITERAL
                            || tokens->next->next->tokenType == HEX_LITERAL
                            || tokens->next->next->tokenType == BIN_LITERAL
                            || tokens->next->next->tokenType == OCTAL_LITERAL)) {

                            bool valid = false;
                            int literalNum = decodeNum(tokens->next->next->tokenText, &valid);
                            if (!valid) {
                                printf("Could not decode second operand for %s instruction on line %d\n", tokens->tokenText, tokens->lineNum);
                                return false;
                            }
                            tokens->next->next->intValue = literalNum;
                            tokens->operandTwo = tokens->next->next;

                        } else {
                            printf("Instruction %s on line %d has an invalid second operand\n", tokens->tokenText, tokens->lineNum);
                            return false;
                        }

                        if (tokens->next->next->next->tokenType == REGISTER) {

                            int regNum = regNumber(tokens->next->next->next->tokenText);
                            if (regNum == -1) {
                                printf("Could not get register number for third operand of %s instruction on line %d\n", tokens->tokenText, tokens->lineNum);
                                return false;
                            }
                            tokens->next->next->next->registerNum = regNum;
                            tokens->operandThree = tokens->next->next->next;

                        } else if (tokens->next->next->next->tokenType == DECIMAL_LITERAL
                            || tokens->next->next->next->tokenType == HEX_LITERAL
                            || tokens->next->next->next->tokenType == BIN_LITERAL
                            || tokens->next->next->next->tokenType == OCTAL_LITERAL) {

                            bool valid = false;
                            int literalNum = decodeNum(tokens->next->next->next->tokenText, &valid);
                            if (!valid) {
                                printf("Could not decode third operand for %s instruction on line %d\n", tokens->tokenText, tokens->lineNum);
                                return false;
                            }
                            tokens->next->next->next->intValue = literalNum;
                            tokens->operandThree = tokens->next->next->next;

                        } else if ((tokens->instructionType == I_BEQ
                            || tokens->instructionType == I_BNE
                            || tokens->instructionType == I_BLT
                            || tokens->instructionType == I_BGT
                            || tokens->instructionType == I_BGE
                            || tokens->instructionType == I_BLE)
                        && tokens->next->next->next->tokenType == IDENTIFIER) {
                            tokens->operandThree = tokens->next->next->next;
                        } else {
                            printf("Instruction %s on line %d has an invalid third operand\n", tokens->tokenText, tokens->lineNum);
                            return false;
                        }

                        tokens = tokens->next->next->next->next;
                        break;
                    }

                    default:
                    printf("Fatal error. Malformed instruction %s on line %d\n", tokens->tokenText, tokens->lineNum);
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

bool calculateNuminstrs(struct LinkedToken * tokens, uint16_t startAddress) {
    if (tokens == NULL || startAddress >= 0x7000 || startAddress < 0x4000) {
        return false;
    }

    while (tokens != NULL) {

        if (tokens->tokenText == NULL || tokens->tokenType == NONE) {
            break;
        }

        switch(tokens->tokenType) {
            case COMMENT:
            tokens = tokens->next;
            break;

            case LABEL: {
                if (tokens->nextInstruction == NULL) {
                    printf("Label %s on line %d is not pointing to an instruction\n",
                        tokens->tokenText, tokens->lineNum);
                    return false;
                } else {
                    tokens = tokens->nextInstruction;
                }
                break;
            }

            case INSTRUCTION: {

                switch(tokens->instructionType) {

                    case I_NOP:
                    tokens->numPrimitives = 1;
                    tokens = tokens->next;
                    break;

                    case I_CLEAR: case I_INC: case I_DEC: case I_POP: case I_NOT: case I_SLL: case I_SRL: {
                        if (tokens->operandOne == NULL) {
                            printf("Invalid first operand for %s instruction on line %d\n",
                                tokens->tokenText, tokens->lineNum);
                            return false;
                        }
                        if (tokens->operandOne->tokenType != REGISTER) {
                            printf("%s instruction on line %d must have register operand\n",
                                tokens->tokenText, tokens->lineNum);
                            return false;
                        }
                        if (tokens->instructionType == I_CLEAR) {
                            tokens->numPrimitives = 1;
                        } else if (tokens->instructionType == I_POP) {
                            tokens->numPrimitives = 4;
                        } else if (tokens->instructionType == I_NOT){
                            tokens->numPrimitives = 3;
                        } else if (tokens->instructionType == I_SLL){
                            tokens->numPrimitives = 1;
                        } else if (tokens->instructionType == I_SRL){
                            tokens->numPrimitives = 13;
                        } else {
                            tokens->numPrimitives = 3;
                        }
                        tokens = tokens->operandOne->next;
                        break;
                    }

                    /* pop actually a bit different from push, handle this later */
                    case I_PUSH: case I_OUT: {
                        if (tokens->operandOne == NULL) {
                            printf("Invalid first operand for %s instruction on line %d\n",
                                tokens->tokenText, tokens->lineNum);
                            return false;
                        }
                        if (tokens->operandOne->tokenType != REGISTER
                            && tokens->operandOne->tokenType != DECIMAL_LITERAL
                            && tokens->operandOne->tokenType != HEX_LITERAL
                            && tokens->operandOne->tokenType != OCTAL_LITERAL
                            && tokens->operandOne->tokenType != BIN_LITERAL) {
                            printf("%s instruction on line %d must have register or int literal operand\n",
                                tokens->tokenText, tokens->lineNum);
                            return false;
                        }

                        if (tokens->instructionType == I_PUSH) {
                            if (tokens->operandOne->tokenType == REGISTER) {
                                tokens->numPrimitives = 4;
                            } else {
                                tokens->numPrimitives = 6;
                            }
                        } else {
                            if (tokens->operandOne->tokenType == REGISTER) {
                                tokens->numPrimitives = 1;
                            } else {
                                tokens->numPrimitives = 3;
                            }
                        }

                        tokens = tokens->operandOne->next;
                        break;
                    }

                    case I_JMP: {
                        if (tokens->operandOne == NULL) {
                            printf("Invalid first operand for %s instruction on line %d\n",
                                tokens->tokenText, tokens->lineNum);
                            return false;
                        }

                        if (tokens->operandOne->tokenType != REGISTER
                            && tokens->operandOne->tokenType != IDENTIFIER
                            && tokens->operandOne->tokenType != DECIMAL_LITERAL
                            && tokens->operandOne->tokenType != HEX_LITERAL
                            && tokens->operandOne->tokenType != OCTAL_LITERAL
                            && tokens->operandOne->tokenType != BIN_LITERAL) {
                            printf("%s instruction on line %d must have register, int literal or label operand\n",
                                tokens->tokenText, tokens->lineNum);
                            return false;
                        }

                        if (tokens->operandOne->tokenType == REGISTER) {
                            tokens->numPrimitives = 1;
                        } else if (tokens->operandOne->tokenType == IDENTIFIER) {
                            tokens->numPrimitives = 3;
                        } else {
                            tokens->numPrimitives = 3;
                        }

                        tokens = tokens->operandOne->next;
                        break;
                    }

                    case I_LLO: case I_LHI: case I_LOAD: {
                        if (tokens->operandOne == NULL) {
                            printf("Invalid first operand for %s instruction on line %d\n",
                                tokens->tokenText, tokens->lineNum);
                            return false;
                        }
                        if (tokens->operandTwo == NULL) {
                            printf("Invalid second operand for %s instruction on line %d\n",
                                tokens->tokenText, tokens->lineNum);
                            return false;
                        }

                        if (tokens->operandOne->tokenType != REGISTER
                            || (tokens->operandTwo->tokenType != REGISTER
                                && tokens->operandTwo->tokenType != DECIMAL_LITERAL
                            && tokens->operandTwo->tokenType != HEX_LITERAL
                            && tokens->operandTwo->tokenType != OCTAL_LITERAL
                            && tokens->operandTwo->tokenType != BIN_LITERAL)) {
                            printf("%s instruction on line %d must have register then register or int literal\n", 
                                tokens->tokenText, tokens->lineNum);
                            return false;
                        }

                        if (tokens->instructionType == I_LOAD) {
                            if (tokens->operandTwo->tokenType == REGISTER) {
                                tokens->numPrimitives = 1;
                            } else {
                                tokens->numPrimitives = 2;
                            }
                        } else if (tokens->instructionType == I_LHI) {
                            if (tokens->operandTwo->tokenType == REGISTER) {
                                tokens->numPrimitives = 7;
                            } else {
                                tokens->numPrimitives = 1;
                            }
                        } else {
                            if (tokens->operandTwo->tokenType == REGISTER) {
                                tokens->numPrimitives = 7;
                            } else {
                                tokens->numPrimitives = 1;
                            }
                        }

                        tokens = tokens->operandTwo->next;
                        break;
                    }

                    case I_LW: case I_SW:  {
                        if (tokens->operandOne == NULL) {
                            printf("Invalid first operand for %s instruction on line %d\n",
                                tokens->tokenText, tokens->lineNum);
                            return false;
                        }
                        if (tokens->operandTwo == NULL) {
                            printf("Invalid second operand for %s instruction on line %d\n",
                                tokens->tokenText, tokens->lineNum);
                            return false;
                        }

                        if (tokens->operandOne->tokenType != REGISTER
                            || (tokens->operandTwo->tokenType != REGISTER
                                && tokens->operandTwo->tokenType != DECIMAL_LITERAL
                            && tokens->operandTwo->tokenType != HEX_LITERAL
                            && tokens->operandTwo->tokenType != OCTAL_LITERAL
                            && tokens->operandTwo->tokenType != BIN_LITERAL
                            && tokens->operandTwo->tokenType != OFFSET)) {
                            printf("%s instruction on line %d must have register then register or int literal or offset\n", 
                                tokens->tokenText, tokens->lineNum);
                            return false;
                        }

                        if (tokens->operandTwo->tokenType == REGISTER) {
                            tokens->numPrimitives = 1;
                        } else if (tokens->operandTwo->tokenType == OFFSET) {
                            tokens->numPrimitives = 4;
                        } else {
                            tokens->numPrimitives = 3;
                        }
                        tokens = tokens->operandTwo->next;
                        break;
                    }

                    
                    
                    

                    default:
                    printf("Instruction %s on line %d not recognized\n",
                        tokens->tokenText, tokens->lineNum);
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
            case STRING_LITERAL:
            case IDENTIFIER:
            case NONE:
            default:
            printf("Location is token %s on line %d is incorrect\n", 
                tokens->tokenText, tokens->lineNum);
            return false;

        }
    }

    return true;
}

bool generateCode(struct LinkedToken * tokens, const char * fileLoc, uint16_t startAddress) {
    if (tokens == NULL || fileLoc == NULL || startAddress >= 0x7000 || startAddress < 0x4000) {
        return false;
    }

    if (!calculateNuminstrs(tokens, startAddress)) {
        printf("Problem generating code. Some of your instructions may be malformed\n");
        return false;
    }

    int fileNameLen = strlen(fileLoc);

    char * textFileName = malloc(sizeof(char) * (fileNameLen + 1));
    char * binaryName = malloc(sizeof(char) * (fileNameLen + 1));
    char * tempName = malloc(sizeof(char) * (fileNameLen + 1));
    if (textFileName == NULL || binaryName == NULL || tempName == NULL) {
        free(textFileName);
        free(binaryName);
        free(tempName);
        return false;
    }
    strcpy(textFileName, fileLoc);
    textFileName[fileNameLen - 3] = 't';
    textFileName[fileNameLen - 2] = 'x';
    textFileName[fileNameLen - 1] = 't';

    strcpy(binaryName, fileLoc);
    binaryName[fileNameLen - 4] = '\0';

    strcpy(tempName, fileLoc);
    tempName[fileNameLen - 3] = 't';
    tempName[fileNameLen - 2] = 'm';
    tempName[fileNameLen - 1] = 'p';

    FILE * textFile = fopen(textFileName, "w");
    FILE * binaryFile = fopen(binaryName, "wb");
    FILE * tempFile = fopen(tempName, "w");


    fclose(textFile);
    fclose(binaryFile);
    fclose(tempFile);
    free(textFileName);
    free(binaryName);
    free(tempName);
    return true;
}

static int regNumber(const char * regString) {
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

static int decodeNum(const char * numStr, bool * valid) {
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
