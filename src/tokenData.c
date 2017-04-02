#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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

struct LinkedToken * tokenize(const char * filename) {
    if (filename == NULL) {
        return NULL;
    }

    FILE * fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("File does not exist or can't be accessed\n");
        return NULL;
    }

    char buffer[1024];
    int bufferIndex = 0;
    int c;

    struct LinkedToken * head = createToken();
    struct LinkedToken * iterator = head;
    if (head == NULL) {
        fclose(fp);
        return NULL;
    }

    bool insideComment = false;

    while ((c = fgetc(fp)) != EOF) {
        /* force lowercase */
        if (c >= 'A' && c <= 'Z') {
            c += ('a' - 'A');
        }

        if (c == '\n') {
            insideComment = false;
        } else if (c == '#') {
            insideComment = true;
        }

        if (((c == '\0' || c == '\n' || c == '\t'
            || c == ' ' || c == '\r' || c == '\v') || c == ',')
                && !insideComment) {
            if (bufferIndex != 0) {
                buffer[bufferIndex] = '\0';
                int bufferLen = strlen(buffer);
                iterator->tokenText = malloc(sizeof(char) * (bufferLen + 1));
                if (iterator->tokenText == NULL) {
                    destroyTokens(head);
                    fclose(fp);
                    return NULL;
                }
                iterator->textSize = bufferLen;
                strcpy(iterator->tokenText, buffer);
                iterator->next = createToken();
                if (iterator->next == NULL) {
                    destroyTokens(head);
                    fclose(fp);
                    return NULL;
                }
                iterator = iterator->next;
                bufferIndex = 0;
            }
        } else {
            buffer[bufferIndex++] = c;
        }
    }

    fclose(fp);
    return head;
}

