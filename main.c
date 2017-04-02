#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "tokenData.h"
#include "tokenIdentify.h"

static void printUsageInfo(const char * binName);

static bool isWhitespace(char c) {
    return (c == '\0' || c == '\n' || c == '\t'
            || c == ' ' || c == '\r' || c == '\v');
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

        if ((isWhitespace(c) || c == ',')
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

    struct LinkedToken * tokens = tokenize(arg);
    if (tokens == NULL) {
        printf("%s could not be processed\n", arg);
        return 1;
    }

    if (!identifyTokens(tokens)) {
        printf("Tokens could not be identified\n");
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

