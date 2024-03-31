#include <stdio.h>
#include <stdlib.h>

enum fileStatus {
    FILES_OPENED_SUCCESSFULLY,
    COULD_NOT_OPEN_FILES,
    FILES_CLOSED_SUCCESSFULLY,
};

enum executionStatus {
    CODE_EXECUTED_WITHOUT_ISSUES
};

typedef struct files {
    FILE *input;
    FILE *output;
    FILE *code;
} System;

int openFiles(System* files) {
    files->output = fopen("BFIO/output.txt", "w");
    if (files->output == NULL) {
        printf("Error: Could not create the output file");
        return COULD_NOT_OPEN_FILES;
    }
    files->code = fopen("BFIO/code.txt", "r");
    if (files->code == NULL) {
        fprintf(files->output, "//LOG: Error: Could not open code.txt");
        fclose(files->output);
        return COULD_NOT_OPEN_FILES;
    }
    files->input = fopen("BFIO/input.txt", "r");
    if (files->code == NULL) {
        fprintf(files->output, "//LOG: Warning: Could not open input.txt");
    }
    return FILES_OPENED_SUCCESSFULLY;
}

int closeFiles(FILE *output, FILE *code, FILE *input) {
    if (input != NULL) {
        fclose(input);
    }
    fclose(code);
    fclose(output);
    return FILES_CLOSED_SUCCESSFULLY;
}

int parseCode(System *files) {
    int mem[100] = {0};
    int *p = mem;
    int index = 0;
    FILE *output = files->output;
    FILE *code = files->code;
    FILE *input = files->input;
    char c = getc(code);
    while(c != EOF) {
        switch(c) {
            case '+': 
                ++*p;
                if (*p > 255) {
                    *p %= 256;
                }
                break;
            case '-':
                --*p;
                while (*p < 0) {
                    *p += 256;
                }
                break;
            case '>':
                index++;
                if (index == 100) {
                    index = 0;
                    p = mem;
                } else {
                    p++;
                }
                break;
            case '<':
                index--;
                if (index == -1) {
                    index = 99;
                    p = mem + 99;
                } else {
                    p++;
                }
                break;
            case '.':
                putc(*p, output);
                break;
            case ',':
                fscanf(input, "%d", p);
                break;
        }
        c = getc(code);
    }
    return CODE_EXECUTED_WITHOUT_ISSUES;
}

int main() {
    System files;
    if (openFiles(&files) == FILES_OPENED_SUCCESSFULLY) {
        int executionResult = parseCode(&files);
        if (executionResult == CODE_EXECUTED_WITHOUT_ISSUES) {
            if (closeFiles(files.output, files.code, files.input) != FILES_CLOSED_SUCCESSFULLY) {
                printf("There was an error during closing the files");
                printf("The code executed successfully");
                return -1;
            }
            return 0;
        } else {
            // Print the error message;
        }
    } else if (openFiles(&files) == COULD_NOT_OPEN_FILES) {
        printf("Could not open one of the required files.");
        return -1;
    }
}