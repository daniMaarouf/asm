#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codeGeneration.h"

static int regNumber(const char * regString);
static int decodeNum(const char * numStr, bool * valid);

/*
    prints ascii binary representation of 16 bit num
*/
static void printBinary(uint16_t num, FILE * stream) {
    if (stream == NULL) {
        return;
    }
    static const char * nybbleStrings[] = {
        "0000","0001","0010","0011","0100","0101","0110","0111",
        "1000","1001","1010","1011","1100","1101","1110","1111"
    };

    fprintf(stream, "%s%s%s%s", 
        nybbleStrings[(num & 0xF000) >> 12], 
        nybbleStrings[(num & 0x0F00) >> 8],
        nybbleStrings[(num & 0x00F0) >> 4],
        nybbleStrings[(num & 0x000F)]);
    return;
}   

static void printWord(FILE * stream, uint16_t word, uint16_t address) {
    if (stream == NULL) {
        return;
    }

    fprintf(stream, "%d => \"", address);
    printBinary(word, stream);
    fprintf(stream, "\",\n");
    return;
}

/*
    when tokens reach this point they can be a
    linear sequence of tokens which have valid
    'token forms'. 
    
    This function will ensure that the
    way the tokens are arranged with respect
    to each other is valid.
    
    It will also decode things like register numbers,
    and int literal values, and it will read these
    values into data structures which represent
    each token so that the information can be used
    in later stages of assembling
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
                } else if (tokens->next->tokenType == REGISTER
                        || tokens->next->tokenType == OFFSET
                        || tokens->next->tokenType == DECIMAL_LITERAL
                        || tokens->next->tokenType == HEX_LITERAL
                        || tokens->next->tokenType == BIN_LITERAL
                        || tokens->next->tokenType == OCTAL_LITERAL
                        || tokens->next->tokenType == IDENTIFIER) {
                    
                    if (tokens->next->tokenText != NULL) {
                        printf("This type of token: %s\nshould not appear after a comment\n", 
                            tokens->next->tokenText);
                        printf("Preceding comment: %s on line %d\n", tokens->tokenText, tokens->lineNum);
                    } else {
                        printf("Invalid token after comment: %s on line %d\n", tokens->tokenText, tokens->lineNum);
                    }
                    return false;
                } else {
                    tokens = tokens->next;
                    break;
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
                    case I_NOP: case I_RET: {
                        tokens = tokens->next;
                        break;
                    }

                    /* one register */
                    case I_CLEAR: case I_INC: case I_DEC: case I_POP: case I_IN: 
                    case I_NOT:
                    /* one register or int literal */
                    case I_PUSH: case I_OUT: case I_WAIT:
                    /* one register or int literal or label */
                    case I_JMP: case I_CALL: {
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
                        } else if (((tokens->instructionType == I_JMP || tokens->instructionType == I_CALL)
                            || tokens->instructionType == I_PUSH
                            || tokens->instructionType == I_OUT
                            || tokens->instructionType == I_WAIT) 
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
                        } else if (((tokens->instructionType == I_JMP || tokens->instructionType == I_CALL)) 
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
                    case I_LLO: case I_LHI: case I_LOAD: case I_SLL: case I_SRL: 
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
                                unsigned int i;
                                for (i = 0; i < tokens->next->next->textSize; i++) {
                                    if (tokens->next->next->tokenText[i] == '(') {
                                        opening = i;
                                        break;
                                    }
                                }

                                if (opening == -1 || opening + 1 >= (int) tokens->next->next->textSize) {
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
            printf("Lone operand '%s' on line %d\n", tokens->tokenText, tokens->lineNum);
            return false;
            
            case STRING_LITERAL:
            case NONE:
            default:
            printf("Illegal token '%s' one line %d\n", tokens->tokenText, tokens->lineNum);
            return false;

        }
    }

    return true;
}

/*
    first time this function is called (with writeCode argument
    equal to 0) no code will be emitted, only the number of primitive
    instructions that each 'pseudoinstruction / assembler instruction'
    will take is calculated and stored in data structure.
    This is necessary to resolve labels to
    actual addresses. Second time this function is called code
    will actually be emitted to file

*/
bool evaluateInstructions(struct LinkedToken * tokens, uint16_t startAddress, bool writeCode, FILE * fp) {
    if (tokens == NULL || startAddress >= 0x7000 || startAddress < 0x4000) {
        return false;
    }
    if (writeCode && fp == NULL) {
        return false;
    }

    if (writeCode) {
        /* load stack pointer register B with 0x7FEF */
        fprintf(fp, "--ENGG3380 Assembler - v0.01, by Dani Maarouf\n");
        printWord(fp, 0x2BEF, startAddress);
        printWord(fp, 0x3B7F, startAddress + 1);
    }

    uint16_t lastAddress = startAddress + 1;

    while (tokens != NULL) {

        if (tokens->tokenText == NULL || tokens->tokenType == NONE) {
            break;
        }

        uint16_t instrs[30];
        unsigned int i;
        for (i = 0; i < sizeof(instrs)/sizeof(instrs[0]); i++) {
            instrs[i] = 0;
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

                    case I_NOP: {
                        tokens->numPrimitives = 1;
                        instrs[0] = 0x0;
                        break;
                    }

                    case I_RET: {
                        /* pop address off stack and jump to it */
                        instrs[0] = 0x2C01;
                        instrs[1] = 0x3C00;
                        instrs[2] = 0xCBBC;
                        instrs[3] = 0x4DB0;
                        instrs[4] = 0x10D0;

                        tokens->numPrimitives = 5;
                        break;
                    }

                    /* must have exactly one register operand */
                    case I_CLEAR: case I_INC: case I_DEC: case I_POP: case I_NOT: case I_IN: {
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

                            instrs[0] = 0xC000;
                            instrs[0] |= (tokens->operandOne->registerNum << 8);
                            instrs[0] |= 0x88;  /* give zero register as 2nd and 3rd operand */
                            tokens->numPrimitives = 1;

                        } else if (tokens->instructionType == I_POP) {
                            instrs[0] = 0x2C01;
                            instrs[1] = 0x3C00;
                            instrs[2] = 0xCBBC; /* increment stack pointer */

                            instrs[3] = 0x4000;
                            instrs[3] |= (tokens->operandOne->registerNum << 8);
                            instrs[3] |= 0xB0;

                            tokens->numPrimitives = 4;
                        } else if (tokens->instructionType == I_NOT){

                            instrs[0] = 0x2CFF;
                            instrs[1] = 0x3CFF;
                            instrs[2] = 0xA000;
                            instrs[2] |= (tokens->operandOne->registerNum << 8);
                            instrs[2] |= (tokens->operandOne->registerNum << 4);
                            instrs[2] |= 0xC;

                            tokens->numPrimitives = 3;

                        } else if (tokens->instructionType == I_INC) {
                            instrs[0] = 0x2C01;
                            instrs[1] = 0x3C00;
                            instrs[2] = 0xC000;
                            instrs[2] |= (tokens->operandOne->registerNum << 8);
                            instrs[2] |= (tokens->operandOne->registerNum << 4);
                            instrs[2] |= 0xC;
                            tokens->numPrimitives = 3; 
                        } else if (tokens->instructionType == I_DEC) {
                            instrs[0] = 0x2CFF;
                            instrs[1] = 0x3CFF;
                            instrs[2] = 0xC000;
                            instrs[2] |= (tokens->operandOne->registerNum << 8);
                            instrs[2] |= (tokens->operandOne->registerNum << 4);
                            instrs[2] |= 0xC;
                            tokens->numPrimitives = 3;
                        } else if (tokens->instructionType == I_IN) {
                            instrs[0] = 0xC000;
                            instrs[0] |= (tokens->operandOne->registerNum) << 8;
                            instrs[0] |= 0x89;
                            tokens->numPrimitives = 1;
                        }

                        break;
                    }

                    /* pop actually a bit different from push, handle this later */
                    case I_PUSH: case I_OUT: case I_WAIT: {
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

                                instrs[0] = 0x50B0;
                                instrs[0] |= tokens->operandOne->registerNum;

                                instrs[1] = 0x2CFF;
                                instrs[2] = 0x3CFF;
                                instrs[3] = 0xCBBC;

                                tokens->numPrimitives = 4;
                            } else {

                                instrs[0] = 0x2C00;
                                instrs[0] |= (tokens->operandOne->intValue & 0xFF);

                                instrs[1] = 0x3C00;
                                instrs[1] |= (((tokens->operandOne->intValue & 0xFF00)) >> 8);

                                instrs[2] = 0x50BC;

                                instrs[3] = 0x2CFF;
                                instrs[4] = 0x3CFF;
                                instrs[5] = 0xCBBC;

                                tokens->numPrimitives = 6;
                            }
                        } else if (tokens->instructionType == I_OUT){
                            if (tokens->operandOne->tokenType == REGISTER) {
                                tokens->numPrimitives = 1;
                                instrs[0] = 0xCA80;
                                instrs[0] |= tokens->operandOne->registerNum;
                            } else {

                                instrs[0] = 0x2C00;
                                instrs[0] |= (tokens->operandOne->intValue & 0xFF);
                                instrs[1] = 0x3C00;
                                instrs[1] |= (((tokens->operandOne->intValue & 0xFF00)) >> 8);
                                instrs[2] = 0xCA8C;
                                tokens->numPrimitives = 3;
                            }
                        } else if (tokens->instructionType == I_WAIT) {
                            if (tokens->operandOne->tokenType == REGISTER) {
                          
                                instrs[0] = 0xCC88;
                                instrs[1] = 0xCD88;

                                /* magic number 2702 for inner loop (happens to take about 1ms to execute) */
                                instrs[2] = 0x2EA3;
                                instrs[3] = 0x3E02;

                                instrs[4] = 0x2F06;
                                instrs[5] = 0x3F00;

                                instrs[6] = 0x6CEF;

                                instrs[7] = 0x2E01;
                                instrs[8] = 0x3E00;

                                instrs[9] = 0xCCCE;

                                instrs[10] = 0x2F00;
                                instrs[10] |= ((tokens->address + 2) & 0xFF);

                                instrs[11] = 0x3F00;
                                instrs[11] |= ((tokens->address + 2) & 0xFF00) >> 8;

                                instrs[12] = 0x10F0;
                                
                                instrs[13] = 0xCE80;
                                instrs[13] |= (tokens->operandOne->registerNum);

                                instrs[14] = 0x0;

                                instrs[15] = 0x2F07;
                                instrs[16] = 0x3F00;

                                instrs[17] = 0x6DEF;

                                instrs[18] = 0x2E01;
                                instrs[19] = 0x3E00;

                                instrs[20] = 0xCDDE;

                                instrs[21] = 0x2F00;
                                instrs[21] |= ((tokens->address + 2) & 0xFF);

                                instrs[22] = 0x3F00;
                                instrs[22] |= ((tokens->address + 2) & 0xFF00) >> 8;
                                instrs[23] = 0xCC88;
                                instrs[24] = 0x10F0;

                                instrs[25] = 0x0;
                                
                                tokens->numPrimitives = 26;
 

                            } else {
                          
                                instrs[0] = 0xCC88;
                                instrs[1] = 0xCD88;

                                /* magic number 2702 for inner loop (happens to take about 1ms to execute) */
                                instrs[2] = 0x2EA3;
                                instrs[3] = 0x3E02;

                                instrs[4] = 0x2F06;
                                instrs[5] = 0x3F00;

                                instrs[6] = 0x6CEF;

                                instrs[7] = 0x2E01;
                                instrs[8] = 0x3E00;

                                instrs[9] = 0xCCCE;

                                instrs[10] = 0x2F00;
                                instrs[10] |= ((tokens->address + 2) & 0xFF);

                                instrs[11] = 0x3F00;
                                instrs[11] |= ((tokens->address + 2) & 0xFF00) >> 8;

                                instrs[12] = 0x10F0;
                                
                                instrs[13] = 0x2E00;
                                instrs[13] |= (tokens->operandOne->intValue & 0xFF);

                                instrs[14] = 0x3E00;
                                instrs[14] |= (tokens->operandOne->intValue & 0xFF00) >> 8;

                                instrs[15] = 0x2F07;
                                instrs[16] = 0x3F00;

                                instrs[17] = 0x6DEF;

                                instrs[18] = 0x2E01;
                                instrs[19] = 0x3E00;

                                instrs[20] = 0xCDDE;

                                instrs[21] = 0x2F00;
                                instrs[21] |= ((tokens->address + 2) & 0xFF);

                                instrs[22] = 0x3F00;
                                instrs[22] |= ((tokens->address + 2) & 0xFF00) >> 8;
                                instrs[23] = 0xCC88;
                                instrs[24] = 0x10F0;

                                instrs[25] = 0x0;
                                
                                tokens->numPrimitives = 26;
                            }

                        }

                        break;
                    }

                    case I_JMP: case I_CALL: {
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

                        if (tokens->instructionType == I_JMP) {
                            if (tokens->operandOne->tokenType == REGISTER) {

                                instrs[0] = 0x1000;
                                instrs[0] |= (tokens->operandOne->registerNum << 4);

                                tokens->numPrimitives = 1;
                            } else {
                                /* same code for label identifiers and immediate values */
                                instrs[0] = 0x2C00;
                                instrs[0] |= (tokens->operandOne->intValue & 0xFF);
                                instrs[1] = 0x3C00;
                                instrs[1] |= (((tokens->operandOne->intValue & 0xFF00)) >> 8);
                                instrs[2] = 0x10C0;

                                tokens->numPrimitives = 3;
                            }
                        } else {
                            if (tokens->operandOne->tokenType == REGISTER) {
                                instrs[0] = 0x2C00;
                                instrs[0] |= ((tokens->address + 7) & 0xFF);
                                instrs[1] = 0x3C00;
                                instrs[1] |= (((tokens->address + 7) & 0xFF00) >> 8);

                                instrs[2] = 0x50BC;

                                instrs[3] = 0x2CFF;
                                instrs[4] = 0x3CFF;
                                instrs[5] = 0xCBBC;
                                instrs[6] = 0x1000;
                                instrs[6] |= (tokens->operandOne->registerNum << 4);

                                tokens->numPrimitives = 7;

                            } else {

                                instrs[0] = 0x2C00;
                                instrs[0] |= ((tokens->address + 9) & 0xFF);

                                instrs[1] = 0x3C00;
                                instrs[1] |= (((tokens->address + 9) & 0xFF00) >> 8);

                                instrs[2] = 0x50BC;

                                instrs[3] = 0x2CFF;
                                instrs[4] = 0x3CFF;
                                instrs[5] = 0xCBBC;

                                instrs[6] = 0x2C00;
                                instrs[6] |= (tokens->operandOne->intValue & 0xFF);

                                instrs[7] = 0x3C00;
                                instrs[7] |= (((tokens->operandOne->intValue & 0xFF00)) >> 8);

                                instrs[8] = 0x10C0;

                                tokens->numPrimitives = 9;
                            }
                        }

                        break;
                    }

                    /* register then reg or int literal */
                    case I_LLO: case I_LHI: case I_LOAD: case I_SLL: case I_SRL: {
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
                                instrs[0] = 0xC000;
                                instrs[0] |= (tokens->operandOne->registerNum << 8);
                                instrs[0] |= (tokens->operandTwo->registerNum << 4);
                                instrs[0] |= 0x8;
                                tokens->numPrimitives = 1;

                            } else {
                                instrs[0] = 0x2000;
                                instrs[0] |= (tokens->operandOne->registerNum << 8);
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);

                                instrs[1] = 0x3000;
                                instrs[1] |= (tokens->operandOne->registerNum << 8);
                                instrs[1] |= (((tokens->operandTwo->intValue & 0xFF00)) >> 8);

                                tokens->numPrimitives = 2;
                            }
                        } else if (tokens->instructionType == I_LHI) {
                            if (tokens->operandTwo->tokenType == REGISTER) {
                                tokens->numPrimitives = 7;

                                instrs[0] = 0x2C00;
                                instrs[1] = 0x3CFF;

                                instrs[2] = 0x8EC0;
                                instrs[2] |= tokens->operandTwo->registerNum;

                                instrs[3] = 0x2CFF;
                                instrs[4] = 0x3C00;

                                instrs[5] = 0x8FC0;
                                instrs[5] |= (tokens->operandOne->registerNum);

                                instrs[6] = 0x9000;
                                instrs[6] |= (tokens->operandOne->registerNum << 8);
                                instrs[6] |= 0xEF;

                            } else {
                                instrs[0] = 0x3000;
                                instrs[0] |= (tokens->operandOne->registerNum << 8);
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);
                                tokens->numPrimitives = 1;
                            }
                        } else if (tokens->instructionType == I_LLO) {
                            if (tokens->operandTwo->tokenType == REGISTER) {

                                instrs[0] = 0x2CFF;
                                instrs[1] = 0x3C00;

                                instrs[2] = 0x8EC0;
                                instrs[2] |= (tokens->operandTwo->registerNum);

                                instrs[3] = 0x2C00;
                                instrs[4] = 0x3CFF;

                                instrs[5] = 0x8FC0;
                                instrs[5] |= (tokens->operandOne->registerNum);

                                instrs[6] = 0x9000;
                                instrs[6] |= (tokens->operandOne->registerNum << 8);
                                instrs[6] |= 0xEF;

                                tokens->numPrimitives = 7;
                            } else {
                                instrs[0] = 0x2000;
                                instrs[0] |= (tokens->operandOne->registerNum << 8);
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);

                                tokens->numPrimitives = 1;
                            }
                        } else if (tokens->instructionType == I_SLL) {

                            if (tokens->operandTwo->tokenType == REGISTER) {
                                instrs[0] = 0xC000;
                                instrs[0] |= (tokens->operandOne->registerNum << 8);
                                instrs[0] |= (tokens->operandTwo->registerNum << 4);
                                instrs[0] |= (tokens->operandTwo->registerNum);
                                tokens->numPrimitives = 1;

                            } else {

                                instrs[0] = 0x2C00;
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);

                                instrs[1] = 0x3C00;
                                instrs[1] |= (tokens->operandTwo->intValue & 0xFF00) >> 8;

                                instrs[2] = 0xC000;
                                instrs[2] |= (tokens->operandOne->registerNum << 8);
                                instrs[2] |= 0xCC;
                                tokens->numPrimitives = 3;
                            }

                        } else if (tokens->instructionType == I_SRL) {
                            if (tokens->operandTwo->tokenType == REGISTER) {

                                /* $t1 = $ry */
                                instrs[0] = 0xCC00;
                                instrs[0] |= (tokens->operandTwo->registerNum << 4);
                                instrs[0] |= 0x8;

                                /* clear $rx */
                                instrs[1] = 0xC000;
                                instrs[1] |= (tokens->operandOne->registerNum << 8);
                                instrs[1] |= 0x88;

                                /* $t2 = 2 */
                                instrs[2] = 0x2D02;
                                instrs[3] = 0x3D00;
                                
                                /* $t1 -= $t2 */
                                instrs[4] = 0xECCD;

                                /* $t3 = ($t1 < 0) */
                                instrs[5] = 0xBEC8;


                                instrs[6] = 0x2F06;
                                instrs[7] = 0x3F00;

                                instrs[8] = 0x7E8F;

                                instrs[9] = 0x2E01;
                                instrs[10] = 0x3E00;

                                instrs[11] = 0xC000;
                                instrs[11] |= (tokens->operandOne->registerNum << 8);
                                instrs[11] |= (tokens->operandOne->registerNum << 4);
                                instrs[11] |= 0xE;

                                instrs[12] = 0x2E00;
                                instrs[12] |= (tokens->address + 4) & 0xFF;

                                instrs[13] = 0x3E00;
                                instrs[13] |= ((tokens->address + 4) & 0xFF00) >> 8;

                                instrs[14] = 0x10E0;

                                instrs[15] = 0x0;
                                tokens->numPrimitives = 16;                               


                            } else {

                                /*  */
                                instrs[0] = 0x2C00;
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);

                                instrs[1] = 0x3C00;
                                instrs[1] |= (tokens->operandTwo->intValue & 0xFF00) >> 8;
                                
                                /* clear $rx */
                                instrs[2] = 0xC000;
                                instrs[2] |= (tokens->operandOne->registerNum << 8);
                                instrs[2] |= 0x88;

                                /* $t2 = 2 */
                                instrs[3] = 0x2D02;
                                instrs[4] = 0x3D00;
                                
                                /* $t1 -= $t2 */
                                instrs[5] = 0xECCD;

                                /* $t3 = ($t1 < 0) */
                                instrs[6] = 0xBEC8;


                                instrs[7] = 0x2F06;
                                instrs[8] = 0x3F00;

                                instrs[9] = 0x7E8F;

                                instrs[10] = 0x2E01;
                                instrs[11] = 0x3E00;

                                instrs[12] = 0xC000;
                                instrs[12] |= (tokens->operandOne->registerNum << 8);
                                instrs[12] |= (tokens->operandOne->registerNum << 4);
                                instrs[12] |= 0xE;

                                instrs[13] = 0x2E00;
                                instrs[13] |= (tokens->address + 5) & 0xFF;

                                instrs[14] = 0x3E00;
                                instrs[14] |= ((tokens->address + 5) & 0xFF00) >> 8;

                                instrs[15] = 0x10E0;

                                instrs[16] = 0x0;
                                tokens->numPrimitives = 17;                               

                                
                            }
                        }

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

                        if (tokens->instructionType == I_LW) {
                            if (tokens->operandTwo->tokenType == REGISTER) {

                                instrs[0] = 0x4000;
                                instrs[0] |= (tokens->operandOne->registerNum << 8);
                                instrs[0] |= (tokens->operandTwo->registerNum << 4);
                                tokens->numPrimitives = 1;

                            } else if (tokens->operandTwo->tokenType == OFFSET) {

                                instrs[0] = 0x2C00;
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);
                                instrs[1] = 0x3C00;
                                instrs[1] |= ((tokens->operandTwo->intValue & 0xFF00) >> 8);

                                instrs[2] = 0xCDC0;
                                instrs[2] |= (tokens->operandTwo->registerNum);

                                instrs[3] = 0x4000;
                                instrs[3] |= (tokens->operandOne->registerNum << 8);
                                instrs[3] |= 0xD0;

                                tokens->numPrimitives = 4;

                            } else {
                                instrs[0] = 0x2C00;
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);

                                instrs[1] = 0x3C00;
                                instrs[1] |= ((tokens->operandTwo->intValue & 0xFF00) >> 8);

                                instrs[2] = 0x4000;
                                instrs[2] |= (tokens->operandOne->registerNum << 8);
                                instrs[2] |= 0xC0;
                                tokens->numPrimitives = 3;
                            }
                        } else {
                            if (tokens->operandTwo->tokenType == REGISTER) {

                                instrs[0] = 0x5000;
                                instrs[0] |= (tokens->operandTwo->registerNum << 4);
                                instrs[0] |= (tokens->operandOne->registerNum << 0);

                                tokens->numPrimitives = 1;
                            } else if (tokens->operandTwo->tokenType == OFFSET) {
                                tokens->numPrimitives = 4;

                                instrs[0] = 0x2C00;
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);
                                instrs[1] = 0x3C00;
                                instrs[1] |= ((tokens->operandTwo->intValue & 0xFF00) >> 8);

                                instrs[2] = 0xCDC0;
                                instrs[2] |= (tokens->operandTwo->registerNum);

                                instrs[3] = 0x50D0;
                                instrs[3] |= (tokens->operandOne->registerNum);

                            } else {

                                instrs[0] = 0x2C00;
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);
                                instrs[1] = 0x3C00;
                                instrs[1] |= (((tokens->operandTwo->intValue & 0xFF00)) >> 8);

                                instrs[2] = 0x50C0;
                                instrs[2] |= (tokens->operandOne->registerNum);

                                tokens->numPrimitives = 3;
                            }
                        }
                    
                        break;
                    }

                    case I_AND: case I_OR: case I_XOR: case I_SLT: case I_UADD: case I_SADD:
                    case I_SSUB: case I_USUB: case I_MUL: case I_DIV: case I_REM: {
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
                        if (tokens->operandThree == NULL) {
                            printf("Invalid third operand for %s instruction on line %d\n",
                                tokens->tokenText, tokens->lineNum);
                            return false;
                        }

                        if (tokens->operandOne->tokenType != REGISTER
                            || tokens->operandTwo->tokenType != REGISTER
                            || (tokens->operandThree->tokenType != REGISTER
                                && tokens->operandThree->tokenType != DECIMAL_LITERAL
                            && tokens->operandThree->tokenType != HEX_LITERAL
                            && tokens->operandThree->tokenType != OCTAL_LITERAL
                            && tokens->operandThree->tokenType != BIN_LITERAL )) {
                            printf("%s instruction on line %d must have register then register then register or int literal\n", 
                                tokens->tokenText, tokens->lineNum);
                            return false;
                        }
                        
                        if (tokens->instructionType == I_MUL) {
                            if (tokens->operandThree->tokenType == REGISTER) {

                                instrs[0] = 0xCC80;
                                instrs[0] |= (tokens->operandTwo->registerNum);

                                instrs[1] = 0xCD80;
                                instrs[1] |= (tokens->operandThree->registerNum);

                                instrs[2] = 0x0;

                                instrs[3] = 0xC000;
                                instrs[3] |= (tokens->operandOne->registerNum << 8);
                                instrs[3] |= 0x88;

                                instrs[4] = 0x2E01;
                                instrs[5] = 0x3E00;

                                instrs[6] = 0xEDDE;

                                instrs[7] = 0xBED8;

                                instrs[8] = 0x2F05;
                                instrs[9] = 0x3F00;

                                instrs[10] = 0x7E8F;

                                instrs[11] = 0xC000;
                                instrs[11] |= (tokens->operandOne->registerNum << 8);
                                instrs[11] |= (tokens->operandOne->registerNum << 4);
                                instrs[11] |= 0xC;

                                instrs[12] = 0x2F00;
                                instrs[12] |= ((tokens->address + 4) & 0xFF);

                                instrs[13] = 0x3F00;
                                instrs[13] |= ((tokens->address + 4) & 0xFF00) >> 8;

                                instrs[14] = 0x10F0;
                                instrs[15] = 0x0;

                                tokens->numPrimitives = 16;

                            } else {
                               
                                instrs[0] = 0xCC00;
                                instrs[0] |= (tokens->operandTwo->registerNum << 4);
                                instrs[0] |= 0x8;

                                instrs[1] = 0x2D00;
                                instrs[1] |= (tokens->operandThree->intValue & 0xFF);
                                instrs[2] = 0x3D00;
                                instrs[2] |= ((tokens->operandThree->intValue & 0xFF00) >> 8);

                                instrs[3] = 0xC000;
                                instrs[3] |= (tokens->operandOne->registerNum << 8);
                                instrs[3] |= 0x88;

                                instrs[4] = 0x2E01;
                                instrs[5] = 0x3E00;

                                instrs[6] = 0xEDDE;

                                instrs[7] = 0xBED8;

                                instrs[8] = 0x2F05;
                                instrs[9] = 0x3F00;

                                instrs[10] = 0x7E8F;

                                instrs[11] = 0xC000;
                                instrs[11] |= (tokens->operandOne->registerNum << 8);
                                instrs[11] |= (tokens->operandOne->registerNum << 4);
                                instrs[11] |= 0xC;

                                instrs[12] = 0x2F00;
                                instrs[12] |= ((tokens->address + 4) & 0xFF);

                                instrs[13] = 0x3F00;
                                instrs[13] |= ((tokens->address + 4) & 0xFF00) >> 8;

                                instrs[14] = 0x10F0;
                                instrs[15] = 0x0;

                                tokens->numPrimitives = 16;

                            }
                        } else if (tokens->instructionType == I_DIV) {
                            if (tokens->operandThree->tokenType == REGISTER) {

                                /* $t1 = $ry */
                                instrs[0] = 0xCC00;
                                instrs[0] |= (tokens->operandTwo->registerNum << 4);
                                instrs[0] |= 0x8;

                                /* $t2 = $rz */
                                instrs[1] = 0xCD00;
                                instrs[1] |= (tokens->operandThree->registerNum) << 4;
                                instrs[1] |= 0x8;

                                /* clear $rx */
                                instrs[2] = 0xC000;
                                instrs[2] |= (tokens->operandOne->registerNum << 8);
                                instrs[2] |= 0x88;

                                instrs[3] = 0x0;
                                
                                /* $t1 -= $t2 */
                                instrs[4] = 0xECCD;

                                /* $t3 = ($t1 < 0) */
                                instrs[5] = 0xBEC8;


                                instrs[6] = 0x2F06;
                                instrs[7] = 0x3F00;

                                instrs[8] = 0x7E8F;

                                instrs[9] = 0x2E01;
                                instrs[10] = 0x3E00;

                                instrs[11] = 0xC000;
                                instrs[11] |= (tokens->operandOne->registerNum << 8);
                                instrs[11] |= (tokens->operandOne->registerNum << 4);
                                instrs[11] |= 0xE;

                                instrs[12] = 0x2E00;
                                instrs[12] |= (tokens->address + 4) & 0xFF;
                                instrs[13] = 0x3E00;
                                instrs[13] |= ((tokens->address + 4) & 0xFF00) >> 8;

                                instrs[14] = 0x10E0;

                                instrs[15] = 0x0;
                                tokens->numPrimitives = 16;                               

                            } else {

                                if (tokens->operandThree->intValue == 0) {
                                    printf("Don't divide by 0. You will halt the processor\n");
                                    return false;
                                }

                                /* $t1 = $ry */
                                instrs[0] = 0xCC00;
                                instrs[0] |= (tokens->operandTwo->registerNum << 4);
                                instrs[0] |= 0x8;

                                /* $t2 = $rz */
                                instrs[1] = 0x2D00;
                                instrs[1] |= (tokens->operandThree->intValue & 0xFF);
                                instrs[2] = 0x3D00;
                                instrs[2] |= (tokens->operandThree->intValue & 0xFF00) >> 8;

                                                                /* clear $rx */
                                instrs[3] = 0xC000;
                                instrs[3] |= (tokens->operandOne->registerNum << 8);
                                instrs[3] |= 0x88;
                                
                                /* $t1 -= $t2 */
                                instrs[4] = 0xECCD;

                                /* $t3 = ($t1 < 0) */
                                instrs[5] = 0xBEC8;


                                instrs[6] = 0x2F06;
                                instrs[7] = 0x3F00;

                                instrs[8] = 0x7E8F;

                                instrs[9] = 0x2E01;
                                instrs[10] = 0x3E00;

                                instrs[11] = 0xC000;
                                instrs[11] |= (tokens->operandOne->registerNum << 8);
                                instrs[11] |= (tokens->operandOne->registerNum << 4);
                                instrs[11] |= 0xE;

                                instrs[12] = 0x2E00;
                                instrs[12] |= (tokens->address + 4) & 0xFF;
                                instrs[13] = 0x3E00;
                                instrs[13] |= ((tokens->address + 4) & 0xFF00) >> 8;

                                instrs[14] = 0x10E0;

                                instrs[15] = 0x0;
                                tokens->numPrimitives = 16;
                            }
                        } else if (tokens->instructionType == I_REM) {
                            if (tokens->operandThree->tokenType == REGISTER) {

  

                                /* $t1 = $ry */
                                instrs[0] = 0xCC00;
                                instrs[0] |= (tokens->operandTwo->registerNum << 4);
                                instrs[0] |= 0x8;

                                /* $t2 = $rz */
                                instrs[1] = 0xCD00;
                                instrs[1] |= (tokens->operandThree->registerNum) << 4;
                                instrs[1] |= 0x8;

                                instrs[2] = 0x0;

                                /* clear $rx */
                                instrs[3] = 0xC000;
                                instrs[3] |= (tokens->operandOne->registerNum << 8);
                                instrs[3] |= 0x88;
                                
                                /* $t1 -= $t2 */
                                instrs[4] = 0xECCD;

                                /* $t3 = ($t1 < 0) */
                                instrs[5] = 0xBEC8;


                                instrs[6] = 0x2F06;
                                instrs[7] = 0x3F00;

                                instrs[8] = 0x7E8F;

                                instrs[9] = 0x2E01;
                                instrs[10] = 0x3E00;

                                instrs[11] = 0xC000;
                                instrs[11] |= (tokens->operandOne->registerNum << 8);
                                instrs[11] |= (tokens->operandOne->registerNum << 4);
                                instrs[11] |= 0xE;

                                instrs[12] = 0x2E00;
                                instrs[12] |= (tokens->address + 4) & 0xFF;
                                instrs[13] = 0x3E00;
                                instrs[13] |= ((tokens->address + 4) & 0xFF00) >> 8;

                                instrs[14] = 0x10E0;

                                instrs[15] = 0xC000;
                                instrs[15] |= (tokens->operandOne->registerNum << 8);
                                instrs[15] |= 0xCD;

                                tokens->numPrimitives = 16;                                   

                            } else {

                                if (tokens->operandThree->intValue == 0) {
                                    printf("Don't divide by 0. You will halt the processor\n");
                                    return false;
                                }

                                

                                /* $t1 = $ry */
                                instrs[0] = 0xCC00;
                                instrs[0] |= (tokens->operandTwo->registerNum << 4);
                                instrs[0] |= 0x8;

                                /* $t2 = $rz */
                                instrs[1] = 0x2D00;
                                instrs[1] |= (tokens->operandThree->intValue & 0xFF);
                                instrs[2] = 0x3D00;
                                instrs[2] |= (tokens->operandThree->intValue & 0xFF00) >> 8;

                                /* clear $rx */
                                instrs[3] = 0xC000;
                                instrs[3] |= (tokens->operandOne->registerNum << 8);
                                instrs[3] |= 0x88;
                                
                                /* $t1 -= $t2 */
                                instrs[4] = 0xECCD;

                                /* $t3 = ($t1 < 0) */
                                instrs[5] = 0xBEC8;


                                instrs[6] = 0x2F06;
                                instrs[7] = 0x3F00;

                                instrs[8] = 0x7E8F;

                                instrs[9] = 0x2E01;
                                instrs[10] = 0x3E00;

                                instrs[11] = 0xC000;
                                instrs[11] |= (tokens->operandOne->registerNum << 8);
                                instrs[11] |= (tokens->operandOne->registerNum << 4);
                                instrs[11] |= 0xE;

                                instrs[12] = 0x2E00;
                                instrs[12] |= (tokens->address + 4) & 0xFF;
                                instrs[13] = 0x3E00;
                                instrs[13] |= ((tokens->address + 4) & 0xFF00) >> 8;

                                instrs[14] = 0x10E0;

                                instrs[15] = 0xC000;
                                instrs[15] |= (tokens->operandOne->registerNum << 8);
                                instrs[15] |= 0xCD;

                                tokens->numPrimitives = 16;
                                

                            }
                        } else {
                            if (tokens->operandThree->tokenType == REGISTER) {
                                instrs[0] = tokens->instructionType << 12;      /* opcode */
                                instrs[0] |= (tokens->operandOne->registerNum << 8);
                                instrs[0] |= (tokens->operandTwo->registerNum << 4);
                                instrs[0] |= (tokens->operandThree->registerNum);
                                tokens->numPrimitives = 1;
                            } else {
                                instrs[0] = 0x2D00;
                                instrs[0] |= (tokens->operandThree->intValue & 0xFF);
                                instrs[1] = 0x3D00;
                                instrs[1] |= ((tokens->operandThree->intValue & 0xFF00) >> 8);
                                instrs[2] = tokens->instructionType << 12;
                                instrs[2] |= (tokens->operandOne->registerNum << 8);
                                instrs[2] |= (tokens->operandTwo->registerNum << 4);
                                instrs[2] |= 0xD;
                                tokens->numPrimitives = 3;
                            }
                        }
                        
                        break;

                    }

                    case I_BEQ: case I_BNE: case I_BLT: case I_BGT: case I_BGE: case I_BLE: {
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
                        if (tokens->operandThree == NULL) {
                            printf("Invalid third operand for %s instruction on line %d\n",
                                tokens->tokenText, tokens->lineNum);
                            return false;
                        }

                        if (tokens->operandOne->tokenType != REGISTER
                            || (tokens->operandTwo->tokenType != REGISTER
                                && tokens->operandTwo->tokenType != DECIMAL_LITERAL
                            && tokens->operandTwo->tokenType != HEX_LITERAL
                            && tokens->operandTwo->tokenType != OCTAL_LITERAL
                            && tokens->operandTwo->tokenType != BIN_LITERAL)
                            || (tokens->operandThree->tokenType != REGISTER
                                && tokens->operandThree->tokenType != DECIMAL_LITERAL
                            && tokens->operandThree->tokenType != HEX_LITERAL
                            && tokens->operandThree->tokenType != OCTAL_LITERAL
                            && tokens->operandThree->tokenType != BIN_LITERAL 
                            && tokens->operandThree->tokenType != IDENTIFIER)) {
                            printf("%s instruction on line %d must have register then register or int literal then register or int literal or label\n", 
                                tokens->tokenText, tokens->lineNum);
                            return false;
                        }

                        if (tokens->operandTwo->tokenType == REGISTER && tokens->operandThree->tokenType == REGISTER) {

                            switch(tokens->instructionType) {
                                case I_BEQ:
                                instrs[0] = 0x6000;
                                instrs[0] |= (tokens->operandOne->registerNum << 8);
                                instrs[0] |= (tokens->operandTwo->registerNum << 4);
                                instrs[0] |= (tokens->operandThree->registerNum);
                                tokens->numPrimitives = 1;
                                break;

                                case I_BNE:
                                instrs[0] = 0x7000;
                                instrs[0] |= (tokens->operandOne->registerNum << 8);
                                instrs[0] |= (tokens->operandTwo->registerNum << 4);
                                instrs[0] |= (tokens->operandThree->registerNum);
                                tokens->numPrimitives = 1;
                                break;

                                case I_BGT:
                                instrs[0] = 0xBC00;
                                instrs[0] |= (tokens->operandTwo->registerNum << 4);
                                instrs[0] |= (tokens->operandOne->registerNum);
                                instrs[1] = 0x7C80;
                                instrs[1] |= (tokens->operandThree->registerNum);
                                tokens->numPrimitives = 2;
                                break;

                                case I_BLT:
                                instrs[0] = 0xBC00;
                                instrs[0] |= (tokens->operandOne->registerNum << 4);
                                instrs[0] |= (tokens->operandTwo->registerNum);
                                instrs[1] = 0x7C80;
                                instrs[1] |= (tokens->operandThree->registerNum);
                                tokens->numPrimitives = 2;
                                break;

                                case I_BGE:
                                instrs[0] = 0xBC00;
                                instrs[0] |= (tokens->operandOne->registerNum << 4);
                                instrs[0] |= (tokens->operandTwo->registerNum);
                                instrs[1] = 0x6C80;
                                instrs[1] |= (tokens->operandThree->registerNum);
                                tokens->numPrimitives = 2;
                                break;

                                case I_BLE:
                                instrs[0] = 0xBC00;
                                instrs[0] |= (tokens->operandTwo->registerNum << 4);
                                instrs[0] |= (tokens->operandOne->registerNum);
                                instrs[1] = 0x6C80;
                                instrs[1] |= (tokens->operandThree->registerNum);
                                tokens->numPrimitives = 2;
                                break;

                                default:
                                return false;
                            }

                        } else if (tokens->operandTwo->tokenType != REGISTER && tokens->operandThree->tokenType == REGISTER) {

                            switch(tokens->instructionType) {
                                case I_BEQ:
                                instrs[0] = 0x2D00;
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);
                                instrs[1] = 0x3D00;
                                instrs[1] |= ((tokens->operandTwo->intValue & 0xFF00) >> 8);

                                instrs[2] = 0x6000;
                                instrs[2] |= (tokens->operandOne->registerNum << 8);
                                instrs[2] |= 0xD0;
                                instrs[2] |= (tokens->operandThree->registerNum);
                                tokens->numPrimitives = 3;
                                break;

                                case I_BNE:
                                instrs[0] = 0x2D00;
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);
                                instrs[1] = 0x3D00;
                                instrs[1] |= ((tokens->operandTwo->intValue & 0xFF00) >> 8);

                                instrs[2] = 0x7000;
                                instrs[2] |= (tokens->operandOne->registerNum << 8);
                                instrs[2] |= 0xD0;
                                instrs[2] |= (tokens->operandThree->registerNum);
                                tokens->numPrimitives = 3;
                                break;

                                case I_BGT:
                                instrs[0] = 0x2D00;
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);
                                instrs[1] = 0x3D00;
                                instrs[1] |= ((tokens->operandTwo->intValue & 0xFF00) >> 8);

                                instrs[2] = 0xBCD0;
                                instrs[2] |= (tokens->operandOne->registerNum);
                                instrs[3] = 0x7C80;
                                instrs[3] |= (tokens->operandThree->registerNum);
                                tokens->numPrimitives = 4;
                                break;

                                case I_BLT:
                                instrs[0] = 0x2D00;
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);
                                instrs[1] = 0x3D00;
                                instrs[1] |= ((tokens->operandTwo->intValue & 0xFF00) >> 8);

                                instrs[2] = 0xBC00;
                                instrs[2] |= (tokens->operandOne->registerNum << 4);
                                instrs[2] |= 0xD;

                                instrs[3] = 0x7C80;
                                instrs[3] |= (tokens->operandThree->registerNum);
                                tokens->numPrimitives = 4;
                                break;

                                case I_BGE:
                                instrs[0] = 0x2D00;
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);
                                instrs[1] = 0x3D00;
                                instrs[1] |= ((tokens->operandTwo->intValue & 0xFF00) >> 8);

                                instrs[2] = 0xBC00;
                                instrs[2] |= (tokens->operandOne->registerNum << 4);
                                instrs[2] |= 0xD;

                                instrs[3] = 0x6C80;
                                instrs[3] |= (tokens->operandThree->registerNum);
                                tokens->numPrimitives = 4;
                                break;

                                case I_BLE:
                                instrs[0] = 0x2D00;
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);
                                instrs[1] = 0x3D00;
                                instrs[1] |= ((tokens->operandTwo->intValue & 0xFF00) >> 8);

                                instrs[2] = 0xBCD0;
                                instrs[2] |= (tokens->operandOne->registerNum);

                                instrs[3] = 0x6C80;
                                instrs[3] |= (tokens->operandThree->registerNum);
                                tokens->numPrimitives = 4;
                                break;

                                default:
                                return false;
                            }

                        } else if (tokens->operandTwo->tokenType == REGISTER && tokens->operandThree->tokenType != REGISTER) {
                            
                            if (tokens->operandThree->tokenType == IDENTIFIER) {
                                int offset;
                                if (tokens->instructionType == I_BEQ || tokens->instructionType == I_BNE) {
                                    offset = tokens->operandThree->intValue - (tokens->address + 3);
                                } else {
                                    offset = tokens->operandThree->intValue - (tokens->address + 4);
                                }
                                if (offset < 0) {
                                    offset += 0x10000;
                                }
                                tokens->operandThree->intValue = offset;
                            }

                            switch(tokens->instructionType) {
                                case I_BEQ:
                                instrs[0] = 0x2E00;
                                instrs[0] |= (tokens->operandThree->intValue & 0xFF);
                                instrs[1] = 0x3E00;
                                instrs[1] |= ((tokens->operandThree->intValue & 0xFF00) >> 8);

                                instrs[2] = 0x6000;
                                instrs[2] |= (tokens->operandOne->registerNum << 8);
                                instrs[2] |= (tokens->operandTwo->registerNum << 4);
                                instrs[2] |= 0xE;
                                tokens->numPrimitives = 3;
                                break;

                                case I_BNE:
                                instrs[0] = 0x2E00;
                                instrs[0] |= (tokens->operandThree->intValue & 0xFF);
                                instrs[1] = 0x3E00;
                                instrs[1] |= ((tokens->operandThree->intValue & 0xFF00) >> 8);

                                instrs[2] = 0x7000;
                                instrs[2] |= (tokens->operandOne->registerNum << 8);
                                instrs[2] |= (tokens->operandTwo->registerNum << 4);
                                instrs[2] |= 0xE;
                                tokens->numPrimitives = 3;
                                break;

                                case I_BGT:
                                instrs[0] = 0x2E00;
                                instrs[0] |= (tokens->operandThree->intValue & 0xFF);
                                instrs[1] = 0x3E00;
                                instrs[1] |= ((tokens->operandThree->intValue & 0xFF00) >> 8);

                                instrs[2] = 0xBC00;
                                instrs[2] |= (tokens->operandTwo->registerNum << 4);
                                instrs[2] |= (tokens->operandOne->registerNum);

                                instrs[3] = 0x7C8E;
                                tokens->numPrimitives = 4;
                                break;

                                case I_BLT:
                                instrs[0] = 0x2E00;
                                instrs[0] |= (tokens->operandThree->intValue & 0xFF);
                                instrs[1] = 0x3E00;
                                instrs[1] |= ((tokens->operandThree->intValue & 0xFF00) >> 8);

                                instrs[2] = 0xBC00;
                                instrs[2] |= (tokens->operandOne->registerNum << 4);
                                instrs[2] |= (tokens->operandTwo->registerNum);

                                instrs[3] = 0x7C8E;
                                tokens->numPrimitives = 4;
                                break;

                                case I_BGE:
                                instrs[0] = 0x2E00;
                                instrs[0] |= (tokens->operandThree->intValue & 0xFF);
                                instrs[1] = 0x3E00;
                                instrs[1] |= ((tokens->operandThree->intValue & 0xFF00) >> 8);

                                instrs[2] = 0xBC00;
                                instrs[2] |= (tokens->operandOne->registerNum << 4);
                                instrs[2] |= (tokens->operandTwo->registerNum);

                                instrs[3] = 0x6C8E;
                                tokens->numPrimitives = 4;
                                break;

                                case I_BLE:
                                instrs[0] = 0x2E00;
                                instrs[0] |= (tokens->operandThree->intValue & 0xFF);
                                instrs[1] = 0x3E00;
                                instrs[1] |= ((tokens->operandThree->intValue & 0xFF00) >> 8);

                                instrs[2] = 0xBC00;
                                instrs[2] |= (tokens->operandTwo->registerNum << 4);
                                instrs[2] |= (tokens->operandOne->registerNum);
                                
                                instrs[3] = 0x6C8E;
                                tokens->numPrimitives = 4;
                                break;

                                default:
                                return false;
                            }

                        } else {

                            if (tokens->operandThree->tokenType == IDENTIFIER) {
                                int offset;
                                if (tokens->instructionType == I_BEQ || tokens->instructionType == I_BNE) {
                                    offset = tokens->operandThree->intValue - (tokens->address + 5);
                                } else {
                                    offset = tokens->operandThree->intValue - (tokens->address + 6);
                                }
                                if (offset < 0) {
                                    offset += 0x10000;
                                }
                                tokens->operandThree->intValue = offset;
                            }

                            switch(tokens->instructionType) {
                                case I_BEQ:
                                instrs[0] = 0x2D00;
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);
                                instrs[1] = 0x3D00;
                                instrs[1] |= ((tokens->operandTwo->intValue & 0xFF00) >> 8);

                                instrs[2] = 0x2E00;
                                instrs[2] |= (tokens->operandThree->intValue & 0xFF);
                                instrs[3] = 0x3E00;
                                instrs[3] |= ((tokens->operandThree->intValue & 0xFF00) >> 8);

                                instrs[4] = 0x6000;
                                instrs[4] |= (tokens->operandOne->registerNum << 8);
                                instrs[4] |= 0xDE;
                                tokens->numPrimitives = 5;
                                break;

                                case I_BNE:
                                instrs[0] = 0x2D00;
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);
                                instrs[1] = 0x3D00;
                                instrs[1] |= ((tokens->operandTwo->intValue & 0xFF00) >> 8);

                                instrs[2] = 0x2E00;
                                instrs[2] |= (tokens->operandThree->intValue & 0xFF);
                                instrs[3] = 0x3E00;
                                instrs[3] |= ((tokens->operandThree->intValue & 0xFF00) >> 8);

                                instrs[4] = 0x7000;
                                instrs[4] |= (tokens->operandOne->registerNum << 8);
                                instrs[4] |= 0xDE;
                                tokens->numPrimitives = 5;
                                break;

                                case I_BGT:
                                instrs[0] = 0x2D00;
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);
                                instrs[1] = 0x3D00;
                                instrs[1] |= ((tokens->operandTwo->intValue & 0xFF00) >> 8);

                                instrs[2] = 0x2E00;
                                instrs[2] |= (tokens->operandThree->intValue & 0xFF);
                                instrs[3] = 0x3E00;
                                instrs[3] |= ((tokens->operandThree->intValue & 0xFF00) >> 8);

                                instrs[4] = 0xBCD0;
                                instrs[4] |= (tokens->operandOne->registerNum);

                                instrs[5] = 0x7C8E;
                                tokens->numPrimitives = 6;
                                break;

                                case I_BLT:
                                instrs[0] = 0x2D00;
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);
                                instrs[1] = 0x3D00;
                                instrs[1] |= ((tokens->operandTwo->intValue & 0xFF00) >> 8);

                                instrs[2] = 0x2E00;
                                instrs[2] |= (tokens->operandThree->intValue & 0xFF);
                                instrs[3] = 0x3E00;
                                instrs[3] |= ((tokens->operandThree->intValue & 0xFF00) >> 8);

                                instrs[4] = 0xBC00;
                                instrs[4] |= (tokens->operandOne->registerNum << 4);
                                instrs[4] |= 0xD;

                                instrs[5] = 0x7C8E;
                                tokens->numPrimitives = 6;
                                break;

                                case I_BGE:
                                instrs[0] = 0x2D00;
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);
                                instrs[1] = 0x3D00;
                                instrs[1] |= ((tokens->operandTwo->intValue & 0xFF00) >> 8);

                                instrs[2] = 0x2E00;
                                instrs[2] |= (tokens->operandThree->intValue & 0xFF);
                                instrs[3] = 0x3E00;
                                instrs[3] |= ((tokens->operandThree->intValue & 0xFF00) >> 8);

                                instrs[4] = 0xBC00;
                                instrs[4] |= (tokens->operandOne->registerNum << 4);
                                instrs[4] |= 0xD;

                                instrs[5] = 0x6C8E;
                                tokens->numPrimitives = 6;
                                break;

                                case I_BLE:
                                instrs[0] = 0x2D00;
                                instrs[0] |= (tokens->operandTwo->intValue & 0xFF);
                                instrs[1] = 0x3D00;
                                instrs[1] |= ((tokens->operandTwo->intValue & 0xFF00) >> 8);

                                instrs[2] = 0x2E00;
                                instrs[2] |= (tokens->operandThree->intValue & 0xFF);
                                instrs[3] = 0x3E00;
                                instrs[3] |= ((tokens->operandThree->intValue & 0xFF00) >> 8);

                                instrs[4] = 0xBCD0;
                                instrs[4] |= (tokens->operandOne->registerNum);

                                instrs[5] = 0x6C8E;
                                tokens->numPrimitives = 6;
                                break;

                                default:
                                return false;
                            }

                        }

                        break;
                    }
                    
                    default:
                    printf("Instruction %s on line %d not recognized\n",
                        tokens->tokenText, tokens->lineNum);
                    return false;

                }

                if (writeCode) {
                    unsigned int i;
                    for (i = 0; i < tokens->numPrimitives; i++) {

                        if ((int) (tokens->address + i) != lastAddress + 1) {
                            printf("An issue was detected when translating %s instruction on line %d\n", tokens->tokenText, tokens->lineNum);
                            printf("Please ensure code generation code is correct\n");
                            return false;
                        }

                        lastAddress = tokens->address + i;
                        printWord(fp, instrs[i], lastAddress);

                    }
                }

                if (tokens->operandThree != NULL) {
                    tokens = tokens->operandThree->next;
                } else if (tokens->operandTwo != NULL) {
                    tokens = tokens->operandTwo->next;
                } else if (tokens->operandOne != NULL) {
                    tokens = tokens->operandOne->next;
                } else {
                    tokens = tokens->next;
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
    if (writeCode) {
        fprintf(fp, "others => \"0000000000000000\"\n");
    }
   

    return true;
}

/*
    in future this function should make sure there arent duplicate
    labels
    does processing including calculating cumulative addresses for label
    resolution and also converting and int values into twos complement
    representation if they are negative. the actual address that a label
    corresponds to is treated similarly to an int literal value
*/
bool resolveLabels(struct LinkedToken * tokens, uint16_t startAddress) {
    if (tokens == NULL || startAddress >= 0x7000 || startAddress < 0x4000) {
        return false;
    }

    struct LinkedToken * iterator = tokens;
    uint16_t cumulativeAddress = startAddress + 2;

    while (iterator != NULL) {
        if (iterator->tokenType == INSTRUCTION) {
            iterator->address = cumulativeAddress;
            cumulativeAddress += iterator->numPrimitives;
            if (cumulativeAddress >= 0x7000) {
                printf("Error. Code generated past 0x6FFF which would overflow into stack\n");
                return false;
            }
        }
        iterator = iterator->next;
    }

    iterator = tokens;
    while (iterator != NULL) {

        if (iterator->tokenType == IDENTIFIER) {
            if (iterator->tokenText == NULL) {
                return false;
            }
            char * matchingLabel = malloc(sizeof(char) * (iterator->textSize + 2));
            if (matchingLabel == NULL) {
                return false;
            }
            strcpy(matchingLabel, iterator->tokenText);
            matchingLabel[iterator->textSize] = ':';
            matchingLabel[iterator->textSize + 1] = '\0';

            struct LinkedToken * innerIterator = tokens;
            bool found = false;

            while (innerIterator != NULL) {

                if (innerIterator->tokenType == LABEL) {
                    if (innerIterator->tokenText == NULL) {
                        free(matchingLabel);
                        return false;
                    }
                    if (strcmp(matchingLabel, innerIterator->tokenText) == 0) {
                        if (innerIterator->nextInstruction == NULL) {
                            free(matchingLabel);
                            return false;
                        }
                        iterator->intValue = innerIterator->nextInstruction->address;
                        found = true;
                    }
                }
                innerIterator = innerIterator->next;
            }
            free(matchingLabel);

            if (!found) {
                printf("Identifier %s on line %d could not be matched with a label\n",
                    iterator->tokenText, iterator->lineNum);
                return false;
            }

        } else if (iterator->tokenType == DECIMAL_LITERAL
            || iterator->tokenType == HEX_LITERAL
            || iterator->tokenType == BIN_LITERAL
            || iterator->tokenType == OCTAL_LITERAL
            || iterator->tokenType == OFFSET) {
            if (iterator->intValue < 0) {
                /* twos complement representation */
                iterator->intValue = 0x10000 + iterator->intValue;
            }
        }


        iterator = iterator->next;

    }

    return true;
}

/*
    higher level function which handles file IO,
    calls functions to perform final stages of
    processing and then finally code generation
*/
bool generateCode(struct LinkedToken * tokens, const char * fileLoc, uint16_t startAddress) {
    if (tokens == NULL || fileLoc == NULL || startAddress >= 0x7000 || startAddress < 0x4000) {
        return false;
    }

    /* PHASE 4.1: first stage evaluation where just lengths of instructions are obtained */
    if (!evaluateInstructions(tokens, startAddress, false, NULL)) {
        printf("Problem evaluating code. Some of your instructions may be malformed\n");
        return false;
    }

    /* PHASE 4.2: convert int values to twos complement, calculate cumulative
    addresses for each instruction and resolve label references */
    if (!resolveLabels(tokens, startAddress)) {
        printf("Problem resolving addresses or labels\n");
        return false;
    }

    int fileNameLen = strlen(fileLoc);

    char * textFileName = malloc(sizeof(char) * (fileNameLen + 1));
    if (textFileName == NULL) {
        return false;
    }
    strcpy(textFileName, fileLoc);
    textFileName[fileNameLen - 3] = 't';
    textFileName[fileNameLen - 2] = 'x';
    textFileName[fileNameLen - 1] = 't';

    FILE * textFile = fopen(textFileName, "w");
    if (textFile == NULL) {
        free(textFileName);
        return false;
    }

    /* PHASE 4.3: emit code */
    if (!evaluateInstructions(tokens, startAddress, true, textFile)) {
        printf("Could not write code to file\n");
        fclose(textFile);
        free(textFileName);
        return false;
    }
    printf("%s created\n", textFileName);
    fclose(textFile);
    free(textFileName);
    return true;
}

/*
    returns register number based on label
    returns -1 if not recognized
*/
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
    } else {
        return -1;
    }
}

/*
    decodes a string representation of an int literal
    can decode decimal, octal, binary or hex

    returns -1 on failure
*/
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
        int weight = 1;
        int i;

        if (numStr[neg + 1] == 'x') {
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
            for (i = numLen - 1; i >= 2 + neg; i--) {
                if (!(numStr[i] >= '0' && numStr[i] <= '7')) {
                    *valid = false;
                    return -1;
                }
                total += weight * (numStr[i] - '0');
                weight *= 8;
            }
        } else if (numStr[neg + 1] == 'b') {
            for (i = numLen - 1; i >= 2 + neg; i--) {
                if (!(numStr[i] >= '0' && numStr[i] <= '1')) {
                    *valid = false;
                    return -1;
                }
                total += weight * (numStr[i] - '0');
                weight *= 2;
            }
            
        } else {
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
