#include <stdio.h>
#include <string.h>
#include "tokenIdentify.h"

static bool isDigit(char c) {
    return (c >= '0' && c <= '9');
}

static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z');
}

bool isLabel(struct LinkedToken * token) {
    if (token == NULL || token->tokenText == NULL
            || token->textSize < 2) {
        return false;
    }
    return (token->tokenText[token->textSize - 1] == ':');
}

bool allChars(struct LinkedToken * token, bool (*func)(char)) {
    if (token == NULL || token->tokenText == NULL
            || token->textSize == 0 || func == NULL) {
        return false;
    }

    int i;
    for (i = 0; i < token->textSize; i++) {
        if (!func(token->tokenText[i])) {
            return false;
        }
    }

    return true;
}

bool nonDecimalLiteral(struct LinkedToken * token, char c) {
    if (token == NULL || token->tokenText == NULL
            || token->textSize < 3) {
        return false;
    }

    if (token->tokenText[0] != '0' || token->tokenText[1] != c) {
        return false;
    }
    int i;
    for (i = 2; i < token->textSize; i++) {

        if (!( (token->tokenText[i] >= '0' && token->tokenText[i] <= '9') 
            || (token->tokenText[i] >= 'a' && token->tokenText[i] <= 'f') ) ) {
            return false;
        }
    }
    return true;
}

bool stringLiteral(struct LinkedToken * token) {
    if (token == NULL || token->tokenText == NULL
            || token->textSize < 2) {
        return false;
    }
    return (token->tokenText[0] == '\"' && token->tokenText[token->textSize - 1] == '\"');
}   

bool isOffset(struct LinkedToken * token) {
    if (token == NULL || token->tokenText == NULL
            || token->textSize < 5) {
        return false;
    }

    int openLoc = -1, closeLoc = -1, i;
    for (i = 0; i < token->textSize; i++) {
        if (token->tokenText[i] == '(') {
            openLoc = i;
            break;
        } else if (token->tokenText[i] == '[') {
            token->tokenText[i] = '(';
            openLoc = i;
            break;
        }
    }

    for (i = token->textSize - 1; i >= 0; i--) {
        if (token->tokenText[i] == ')') {
            closeLoc = i;
            break;
        } else if (token->tokenText[i] == ']') {
            token->tokenText[i] = ')';
            closeLoc = i;
            break;
        }
    }

    if (openLoc == -1 || closeLoc == -1 
        || openLoc > closeLoc || openLoc >= token->textSize - 2) {
        return false;
    }

    if (token->tokenText[openLoc + 1] != '$'
        || !isAlpha(token->tokenText[openLoc + 2])) {
        return false;
    }

    for (i = openLoc + 3; i < closeLoc; i++) {
        if (!isAlpha(token->tokenText[i]) && !isDigit(token->tokenText[i])) {
            return false;
        }
    }

    return true;
}

int instructionType(struct LinkedToken * token) {
    if (token == NULL || token->tokenText == NULL
            || token->textSize < 2) {
        return -1;
    }

    const char * mnemonics[] = {
        "nop", "jmp", "llo", "lhi", "lw", "sw", "beq", "bne",
        "and", "or", "xor", "slt", "uadd", "sadd", "ssub", "usub",
        "blt", "bgt", "bge", "ble", "inc", "dec", "mul", "div",
        "sll", "srl", "push", "pop", "not", "rem", "clear"
    };

    int size = sizeof(mnemonics)/sizeof(mnemonics[0]);
    int i;
    for (i = 0; i < size; i++) {
        if (strcmp(mnemonics[i], token->tokenText) == 0) {
            return i;
        }
    }

    return -1;
}

bool identifyTokens(struct LinkedToken * list) {
    if (list == NULL) {
        return false;
    }

    while (list != NULL) {
        if (list->tokenText == NULL) {
            break;
        } else if (list->textSize == 0) {
            return false;
       }

        if (isLabel(list)) {
            list->tokenType = LABEL;
        } else if (list->tokenText[0] == '#') {
            list->tokenType = COMMENT;
        } else if (list->tokenText[0] == '$') {
            list->tokenType = REGISTER;
        } else if (allChars(list, isDigit)) {
            list->tokenType = DECIMAL_LITERAL;
        } else if (nonDecimalLiteral(list, 'x')) {
            list->tokenType = HEX_LITERAL;
        } else if (nonDecimalLiteral(list, 'b')) {
            list->tokenType = BIN_LITERAL;
        } else if (nonDecimalLiteral(list, 'o')) {
            list->tokenType = OCTAL_LITERAL;
        } else if (stringLiteral(list)) {
            list->tokenType = STRING_LITERAL;
            printf("String literals not supported: %s\n", list->tokenText);
            return false;
        } else if (allChars(list, isAlpha)) {

            int type = instructionType(list);
            if (type == -1) {
                printf("Unrecognized token %s\n", list->tokenText);
                return false;
            }

            list->tokenType = INSTRUCTION;
            list->instructionType = (enum InstructionType) type;

        } else if (isOffset(list)) {
            list->tokenType = OFFSET;
        } else {
            printf("Unrecognized token %s\n", list->tokenText);
            return false;
        }

        list = list->next;
    }

    return true;
}