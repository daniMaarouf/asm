#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "tokenData.h"

#define BUFFER_LEN 256

/*
    creates memory for token node and initializes
    fields with default values
*/
struct LinkedToken * createToken() {
    struct LinkedToken * temp = malloc(sizeof(struct LinkedToken));
    if (temp == NULL) {
        return NULL;
    }
    temp->tokenType = NONE;
    temp->lineNum = -1;
    temp->instructionType = I_OTHER;
    temp->operandOne = NULL;
    temp->operandTwo = NULL;
    temp->operandThree = NULL;
    temp->registerNum = 0;
    temp->intValue = 0;
    temp->nextInstruction = NULL;
    temp->tokenText = NULL;
    temp->textSize = 0;
    temp->numPrimitives = 0;
    temp->address = 0;
    temp->next = NULL;
    return temp;
}

/*
    frees memory for a list of tokens
*/
void destroyTokens(struct LinkedToken * list) {
    while (list != NULL) {
        free(list->tokenText);
        struct LinkedToken * temp = list->next;
        free(list);
        list = temp;
    }
}

/*
    tokenize based on whitespace and commas
*/
struct LinkedToken * tokenize(const char * filename) {
    if (filename == NULL) {
        return NULL;
    }

    FILE * fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("File does not exist or can't be accessed\n");
        return NULL;
    }

    char buffer[BUFFER_LEN];
    int bufferIndex = 0;
    int c;

    struct LinkedToken * head = createToken();
    struct LinkedToken * iterator = head;
    if (head == NULL) {
        fclose(fp);
        return NULL;
    }

    bool insideComment = false;
    int lineNum = 0;

    while ((c = fgetc(fp)) != EOF) {
        /* force lowercase */
        if (c >= 'A' && c <= 'Z') {
            c += ('a' - 'A');
        }

        if (c == '\n') {
            lineNum++;
            insideComment = false;
        } else if (c == '#') {
            insideComment = true;
        }

        if (((c == '\0' || c == '\n' || c == '\t'
            || c == ' ' || c == '\r' || c == '\v') || c == ',')
                && !insideComment) {

            if (bufferIndex >= BUFFER_LEN - 1) {
                destroyTokens(head);
                fclose(fp);
                printf("Token on line %d is too large and can't be parsed\n", lineNum);
                printf("Maximum token size is %d\n", BUFFER_LEN);
                return NULL;
            }

            if (bufferIndex != 0) {
                buffer[bufferIndex] = '\0';
                int bufferLen = strlen(buffer);
                iterator->tokenText = malloc(sizeof(char) * (bufferLen + 1));
                if (iterator->tokenText == NULL) {
                    destroyTokens(head);
                    fclose(fp);
                    printf("Malloc failed in tokenize()\n");
                    return NULL;
                }
                iterator->textSize = bufferLen;
                iterator->lineNum = lineNum;
                strcpy(iterator->tokenText, buffer);
                iterator->next = createToken();
                if (iterator->next == NULL) {
                    destroyTokens(head);
                    fclose(fp);
                    printf("Malloc failed in tokenize()\n");
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

    if (bufferIndex != 0) {
        if (bufferIndex >= BUFFER_LEN - 1) {
            destroyTokens(head);
            printf("Token on line %d is too large and can't be parsed\n", lineNum);
            printf("Maximum token size is %d\n", BUFFER_LEN);
            return NULL;
        }
        buffer[bufferIndex] = '\0';
        int bufferLen = strlen(buffer);
        iterator->tokenText = malloc(sizeof(char) * (bufferLen + 1));
        if (iterator->tokenText == NULL) {
            destroyTokens(head);
            printf("Malloc failed in tokenize()\n");
            return NULL;
        }
        iterator->textSize = bufferLen;
        iterator->lineNum = lineNum;
        strcpy(iterator->tokenText, buffer);
        iterator->next = createToken();
        if (iterator->next == NULL) {
            destroyTokens(head);
            printf("Malloc failed in tokenize()\n");
            return NULL;
        }
        iterator = iterator->next;
        bufferIndex = 0;
    }

    return head;
}

