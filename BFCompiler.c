#include <stdio.h>
#include <stdlib.h>

#define OUTPUT_FILE "BFIO/output.txt"
#define INPUT_FILE "BFIO/input.txt"
#define CODE_FILE "BFIO/code.txt"

enum FileStatus {
    FILES_OPENED_SUCCESSFULLY,
    COULD_NOT_OPEN_FILES,
    FILES_CLOSED_SUCCESSFULLY,
};

typedef enum ExecutionStatus {
    CODE_EXECUTED_WITHOUT_ISSUES,
    ERROR_WHILE_EXECUTING_THE_CODE,
    BRACKET_WAS_NOT_CLOSED,
    BRACKET_WAS_NOT_OPENED,
} ExecutionStatus;

typedef enum NodeType {
    INTEGER,
    CHAR
} NodeType;

typedef struct files {
    FILE *input;
    FILE *output;
    FILE *code;
} System;

typedef struct node {
    struct node *next;
    struct node *prev;
    enum NodeType type;
    union {
        char cvalue;
        int ivalue;
    };
} Node;

int openFiles(System* files) {
    files->output = fopen(OUTPUT_FILE, "w");
    if (files->output == NULL) {
        printf("Error: Could not create the output file");
        return COULD_NOT_OPEN_FILES;
    }
    files->code = fopen(CODE_FILE, "r");
    if (files->code == NULL) {
        fprintf(files->output, "//LOG: Error: Could not open code.txt");
        fclose(files->output);
        return COULD_NOT_OPEN_FILES;
    }
    files->input = fopen(INPUT_FILE, "r");
    if (files->input == NULL) {
        fprintf(files->output, "//LOG: Warning: Could not open input.txt\n");
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
    if (head != NULL) {
        Node *p = head;
        while (head->next) {
            head = head->next;
            free(p);
            p = head;
        }
        free(head);
    }
}

Node* getNode(NodeType type, int value) {
    Node *mem = (Node*) malloc(sizeof(Node));
    mem->next = NULL;
    mem->prev = NULL;
    mem->type = type;
    if (type == INTEGER) {
        mem->ivalue = value;
    } else if (type == CHAR) {
        mem->cvalue = (char) value;
    }
    return mem;
}

void createNodeIfMissing(Node *p) {
    if (p->next == NULL) {
        Node *new = getNode(CHAR, 0);
        p->next = new;
        new->prev = p;
    }
}

Node* handleBracketOpening(Node* p, Node* bracket, System* files) {
    char c;
    if (p->cvalue == 0) {
        while(c != ']')
            c = getc(files->code);
    } else {
        Node *new = getNode(INTEGER, ftell(files->code));
        bracket->next = new;
        new->prev = bracket;
        bracket = bracket->next;
    }
    return bracket;
}

Node* handleBracketClosing(Node* p, Node* bracket, System* files) {
    if (p->cvalue == 0) {
        bracket = bracket->prev;
        free(bracket->next);
        bracket->next = NULL;
    } else {
        fseek(files->code, bracket->ivalue, SEEK_SET);
    }
    return bracket;
}

ExecutionStatus finishExecution(ExecutionStatus code, Node *memory, Node *brackets) {
    freeMemory(memory);
    freeMemory(brackets);
    return code;
}

ExecutionStatus finalBracketCheck(Node* bracket, Node* mem, Node* bracketsNext) {
    if (bracket->ivalue != -1) {
        return finishExecution(BRACKET_WAS_NOT_CLOSED, mem, bracketsNext);
    }
    return finishExecution(CODE_EXECUTED_WITHOUT_ISSUES, mem, bracketsNext);
}

int parseCode(System *files) {
    Node *mem = getNode(CHAR, 0), brackets = {.prev = NULL, .next = NULL, .ivalue = -1, .type = INTEGER};
    Node *p = mem, *bracket = &brackets;
    int tmp;
    char c;
    while((c = getc(files->code)) != EOF) {
        switch(c) {
            case '+': 
                p->cvalue++;
                break;
            case '-':
                p->cvalue--;
                break;
            case '>':
                createNodeIfMissing(p);
                p = p->next;
                break;
            case '<':
                if (p->prev == NULL) {
                    return finishExecution(ERROR_WHILE_EXECUTING_THE_CODE, mem, brackets.next);
                }
                p = p->prev;
                break;
            case '[':
                bracket = handleBracketOpening(p, bracket, files);
                break;
            case ']':
                if (bracket->ivalue == -1) {
                    return finishExecution(BRACKET_WAS_NOT_OPENED, mem, brackets.next);
                }
                bracket = handleBracketClosing(p, bracket, files);
                break;
            case '.':
                putc(p->cvalue, files->output);
                break;
            case ',':
                if (!files->input) {
                    fprintf(files->output, "//LOG: Error: Tried to read from non-existing input.txt file");
                    return finishExecution(ERROR_WHILE_EXECUTING_THE_CODE, mem, brackets.next);
                }
                fscanf(files->input, "%d", &tmp);
                p->cvalue = (char) (tmp % 256);
                break;
        }
    }
    return finalBracketCheck(bracket, mem, brackets.next);
}

void printExecutionIssues(ExecutionStatus result) {

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
