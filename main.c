#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct LinkedToken {
    enum TokenType {
        LABEL,
        INSTRUCTION,
        OPERAND,
        DECIMAL_LITERAL,
        HEX_LITERAL,
        BIN_LITERAL,
        OFFSET,
        COMMENT,
        STRING_LITERAL,
        NONE
    } tokenType;
    char * tokenText;
    int textSize;
    struct LinkedToken * next;
};

struct LinkedToken * createToken() {
    struct LinkedToken * temp = malloc(sizeof(struct LinkedToken));
    if (temp == NULL) {
        return NULL;
    }
    temp->tokenType = NONE;
    temp->textSize = 0;
    temp->tokenText = NULL;
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

static void printUsageInfo(const char * binName);

static bool isWhitespace(char c) {
    return (c == '\0' || c == '\n' || c == '\t'
            || c == ' ' || c == '\r' || c == '\v');
}

static bool isDigit(char c) {
    return (c >= '0' && c <= '9');
}

static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z');
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

        if (isWhitespace(c) && !insideComment) {
            if (bufferIndex != 0) {
                buffer[bufferIndex] = '\0';
                int bufferLen = strlen(buffer);
                iterator->tokenText = malloc(sizeof(char) * (bufferLen + 1));
                if (iterator->tokenText == NULL) {
                    destroyTokens(head);
                    return NULL;
                }
                iterator->textSize = bufferLen;
                strcpy(iterator->tokenText, buffer);
                iterator->next = createToken();
                if (iterator->next == NULL) {
                    destroyTokens(head);
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

bool isLabel(struct LinkedToken * token) {
    if (token == NULL || token->tokenText == NULL
            || token->tokenSize == 0) {
        return false;
    }
    return (token->tokenText[token->tokenSize - 1] == ':');
}

bool isComment(struct LinkedToken * comment) {
    if (token == NULL || token->tokenText == NULL
            || token->tokenSize == 0) {
        return false;
    }
    return (token->tokenText[0] == '#');
}

bool identifyTokens(struct LinkedToken * list) {
    if (list == NULL) {
        return false;
    }

    while (list != NULL) {
        if (list->tokenText == NULL) {
            break;
        } else if (list->tokenSize == 0) {
            return false;
       }

        if (isLabel(list)) {
            list->tokenType = LABEL;
        } else if (isComment(list)) {
            list->tokenType = COMMENT;
        }

        list = list->next;
    }

    

    return true;
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

