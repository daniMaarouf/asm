#include <stdio.h>
#include <stdlib.h>
#include "tokenData.h"

struct LinkedToken * createToken() {
    struct LinkedToken * temp = malloc(sizeof(struct LinkedToken));
    if (temp == NULL) {
        return NULL;
    }
    temp->tokenType = NONE;
    temp->instructionType = I_OTHER;
    temp->operandOne = NULL;
    temp->operandTwo = NULL;
    temp->operandThree = NULL;
    temp->registerNum = 0;
    temp->intValue = 0;
    temp->tokenText = NULL;
    temp->textSize = 0;
    temp->next = NULL;
    return temp;
}

void destroyTokens(struct LinkedToken * list) {
    while (list != NULL) {
        free(list->tokenText);
        struct LinkedToken * temp = list->next;
        free(list);
        list = temp;
    }
}

void printTokens(struct LinkedToken * tokens) {
    while (tokens != NULL) {
        if (tokens->tokenText != NULL) {
            printf("%s\n", tokens->tokenText);
        } else {
            break;
        }
        tokens = tokens->next;
    }
}
