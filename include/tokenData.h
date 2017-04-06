#ifndef __DMAAROUF_TOKENDATA__
#define __DMAAROUF_TOKENDATA__

/*
    for storing coded representation of the instruction type
*/
enum InstructionType {
    I_NOP = 0, I_JMP = 1, I_LLO = 2, I_LHI = 3, I_LW = 4, I_SW = 5, I_BEQ = 6, I_BNE = 7,
    I_AND = 8, I_OR = 9, I_XOR = 10, I_SLT = 11, I_UADD = 12, I_SADD = 13, I_SSUB = 14, I_USUB = 15,
    I_BLT = 16, I_BGT = 17, I_BGE = 18, I_BLE = 19, I_INC = 20, I_DEC = 21, I_MUL = 22, I_DIV = 23,
    I_SLL = 24, I_SRL = 25, I_PUSH = 26, I_POP = 27, I_NOT = 28, I_REM = 29, I_CLEAR = 30, I_OUT = 31,
    I_LOAD = 32, I_CALL = 33, I_RET = 34, I_WAIT = 35, I_IN = 36, I_OTHER = 37
};

/*
    every token which is found in a file will have a matching LinkedToken struct

*/
struct LinkedToken {

    /* this identifies the type of the token */
    /* filled during token identification phase */
    enum TokenType {
        LABEL,
        INSTRUCTION,
        REGISTER,
        DECIMAL_LITERAL,
        HEX_LITERAL,
        BIN_LITERAL,
        OCTAL_LITERAL,
        OFFSET,
        COMMENT,
        STRING_LITERAL,
        IDENTIFIER,
        NONE
    } tokenType;
    int lineNum;

    /* these fields are valid if token is an instruction
    all 3 fields will be non null only for instructions which
    take 3 operands */
    enum InstructionType instructionType;
    struct LinkedToken * operandOne;
    struct LinkedToken * operandTwo;
    struct LinkedToken * operandThree;

    /* this field is valid if token is a register OR an offset */
    unsigned int registerNum;

    /* this field is valid if token is an int literal OR an offset */
    int intValue;

    /* this field is only valid if the token is a label */
    struct LinkedToken * nextInstruction;

    char * tokenText;               /* ascii text of token */
    unsigned int textSize;          /* string size */

    /* used during code generation */
    unsigned int numPrimitives;     /* this allows label resolution */
    unsigned int address;           /* stores address of first primitive */

    /* for linked list */
    struct LinkedToken * next;
};

struct LinkedToken * createToken();
void destroyTokens(struct LinkedToken * list);
void printTokens(struct LinkedToken * tokens);
struct LinkedToken * tokenize(const char * filename);

#endif
/* defined __DMAAROUF_TOKENDATA__ */
