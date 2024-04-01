#include <stdio.h>
#include <stdlib.h>

enum fileStatus {
    FILES_OPENED_SUCCESSFULLY,
    COULD_NOT_OPEN_FILES,
    FILES_CLOSED_SUCCESSFULLY,
};

enum executionStatus {
    CODE_EXECUTED_WITHOUT_ISSUES,
    ERROR_WHILE_EXECUTING_THE_CODE
};

typedef struct files {
    FILE *input;
    FILE *output;
    FILE *code;
} System;

typedef struct node {
    struct node *next;
    struct node *prev;
    char value;
} Node;

typedef struct intNode {
    struct intNode *next;
    struct intNode *prev;
    int value;
} IntNode;

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

void freeMemory(Node *head) {
    Node *p = head;
    while (head->next) {
        head = head->next;
        free(p);
        p = head;
    }
    free(head);
}

void freeBrackets(IntNode *head) {
    IntNode *p = head;
    while (head->next) {
        head = head->next;
        free(p);
        p = head;
    }
    free(head);
}

void freeMemoryAndBrackets(Node *p, IntNode *q) {
    freeMemory(p);
    if (q) {
        freeBrackets(q);
    }
}

void createNext(Node *p) {
    Node *next = (Node*) malloc(sizeof(Node));
    next->next = NULL;
    next->prev = p;
    next->value = 0;
    p->next = next;
}

Node* getHead() {
    Node *mem = (Node*) malloc(sizeof(Node));
    mem->next = NULL;
    mem->prev = NULL;
    mem->value = 0;
    return mem;
}

IntNode* getHeadInt() {
    IntNode *p = (IntNode*) malloc(sizeof(IntNode));
    p->next = NULL;
    p->prev = NULL;
    p->value = -1;
    return p;
}

void createNextInt(IntNode *p, int value) {
    IntNode *next = (IntNode*) malloc(sizeof(Node));
    next->next = NULL;
    next->prev = p;
    next->value = value;
    p->next = next;
}

int parseCode(System *files) {
    Node *mem = getHead();
    IntNode brackets = {.prev = NULL, .next = NULL, .value = -1};
    Node *p = mem;
    IntNode *bracket = &brackets;
    FILE *output = files->output, *code = files->code, *input = files->input;
    int tmp;
    char c;
    while((c = getc(code)) != EOF) {
        switch(c) {
            case '+': 
                p->value++;
                break;
            case '-':
                p->value--;
                break;
            case '>':
                if (p->next == NULL)
                    createNext(p);
                p = p->next;
                break;
            case '<':
                if (p->prev == NULL) {
                    freeMemoryAndBrackets(mem, brackets.next);
                    return ERROR_WHILE_EXECUTING_THE_CODE;
                }
                p = p->prev;
                break;
            case '[':
                if (p->value == 0) {
                    while(c != ']')
                        c = getc(code);
                } else {
                    createNextInt(bracket, ftell(code));
                    bracket = bracket->next;
                }
                break;
            case ']':
                if (bracket->value == -1) {
                    freeMemoryAndBrackets(mem, brackets.next);
                    return ERROR_WHILE_EXECUTING_THE_CODE;
                }
                if (p->value == 0) {
                    bracket = bracket->prev;
                    free(bracket->next);
                    bracket->next = NULL;
                } else {
                    fseek(code, bracket->value, SEEK_SET);
                }
                break;
            case '.':
                putc(p->value, output);
                break;
            case ',':
                fscanf(input, "%d", &tmp);
                p->value = (char) (tmp % 256);
                break;
        }
    }
    freeMemory(mem);
    if (bracket->value != -1) {
        return ERROR_WHILE_EXECUTING_THE_CODE;
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
            printf("Program executed successfully!");
            return 0;
        } else {
            printf("There was an error during the process");
            closeFiles(files.output, files.code, files.input);
            return -1;
        }
    } else if (openFiles(&files) == COULD_NOT_OPEN_FILES) {
        printf("Could not open one of the required files.");
        return -1;
    }
}
