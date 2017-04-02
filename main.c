#include <stdio.h>
#include <string.h>

static void printUsageInfo(const char * binName);

static int isWhitespace(char c) {
    return (c == '\0' || c == '\n' || c == '\t'
            || c == ' ' || c == '\r' || c == '\v');
}

static int isDigit(char c) {
    return (c >= '0' && c <= '9');
}

static int isAlpha(char c) {
    return (c >= 'a' && c <= 'z');
}

static int translate(const char * filename) {
    if (filename == NULL) {
        return 0;
    }

    FILE * fp = fopen(filename, "r");
    if (fp == NULL) {
        return 0;
    }

    enum ScanState {
        STARTLINE,
        LABEL,
        INSTRUCTION,
        FIRST_OPERAND,
        SECOND_OPERAND,
        THIRD_OPERAND,
        COMMENT,
    } state = DEFAULT;

    struct LineInfo {
        char label[64];
        int labelIndex;
        char instruction[16];
        int instructionIndex;
        char operand1[16];
        int operand1Index;
        char operand2[16];
        int operand2Index;
        char operand3[16];
        int operand3Index;
        char comment[256];
        int commentIndex;
    } line;

    line.labelIndex = 0;
    line.instructionIndex = 0;
    line.operand1Index = 0;
    line.operand2Index = 0;
    line.operand3Index = 0;
    line.commentIndex = 0;

    int c;
    while ((c = fgetc(fp)) != EOF) {
        /* force lowercase */
        if (c >= 'A' && c <= 'Z') {
            c += ('a' - 'A');
        }

        switch (state) {

            case STARTLINE:
            if (isWhitespace(c)) {
                break;
            } else if (c == '#') {
                
            } else if (isDigit(c)) {

            } else if (isAlpha(c)) {

            }
            break;
          
        }

        printf("%c", c);

    }

    fclose(fp);
    return 1;
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

    if (!processPseudoinstructions(arg)) {
        printf("%s could not be processed\n", arg);
        return 1;
    }


    return 0;
}

static void printUsageInfo(const char * binName) {
    printf("\nUsage:\n");
    printf("\t%s *.asm\n", binName);
    
    return;
}
