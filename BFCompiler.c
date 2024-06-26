#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OUTPUT_FILE "BFIO/output.txt"
#define INPUT_FILE "BFIO/input.txt"
#define CODE_FILE "BFIO/code.txt"

typedef enum FileStatus {
    FILES_OPENED_SUCCESSFULLY,
    COULD_NOT_OPEN_FILES,
    FILES_CLOSED_SUCCESSFULLY,
} FileStatus;

typedef enum LogLevel {
    ERROR,
    WARNING
} LogLevel;

typedef enum ExecutionStatus {
    CODE_EXECUTED_WITHOUT_ISSUES,
    ERROR_WHILE_EXECUTING_THE_CODE,
    BRACKET_WAS_NOT_CLOSED,
    BRACKET_WAS_NOT_OPENED,
    FAILED_TO_READ_INPUT_FILE
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

void logError(char* message, LogLevel logLevel, System* files) {
    char level[8];
    if (logLevel == ERROR) {
        strcpy(level, "Error");
    } else if (logLevel == WARNING) {
        strcpy(level, "Warning");
    }
    printf("%s: %s\n", level, message);
    if (files->output) {
        fprintf(files->output, "// LOG: %s: %s\n", level, message);
    }
}

int openFiles(System* files) {
    files->output = fopen(OUTPUT_FILE, "w");
    if (files->output == NULL) {
        logError("Error: Could not create the output file", ERROR, files);
        return COULD_NOT_OPEN_FILES;
    }
    files->code = fopen(CODE_FILE, "r");
    if (files->code == NULL) {
        logError("Could not open code.txt", ERROR, files);
        fclose(files->output);
        return COULD_NOT_OPEN_FILES;
    }
    files->input = fopen(INPUT_FILE, "r");
    if (files->input == NULL) {
        logError("Could not open input.txt", WARNING, files);
    }
    return FILES_OPENED_SUCCESSFULLY;
}

int closeFiles(System* files) {
    if (files->input != NULL) {
        fclose(files->input);
    }
    fclose(files->code);
    fclose(files->output);
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

ExecutionStatus finalBracketCheck(Node* bracket, Node* mem, Node* brackets) {
    if (bracket->ivalue != -1) {
        return finishExecution(BRACKET_WAS_NOT_CLOSED, mem, brackets);
    }
    return finishExecution(CODE_EXECUTED_WITHOUT_ISSUES, mem, brackets);
}

int parseCode(System *files) {
    Node *mem = getNode(CHAR, 0), *brackets = getNode(INTEGER, -1), *p = mem, *bracket = brackets;
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
                    return finishExecution(ERROR_WHILE_EXECUTING_THE_CODE, mem, brackets);
                }
                p = p->prev;
                break;
            case '[':
                bracket = handleBracketOpening(p, bracket, files);
                break;
            case ']':
                if (bracket->ivalue == -1) {
                    return finishExecution(BRACKET_WAS_NOT_OPENED, mem, brackets);
                }
                bracket = handleBracketClosing(p, bracket, files);
                break;
            case '.':
                putc(p->cvalue, files->output);
                break;
            case ',':
                if (!files->input) {
                    return finishExecution(FAILED_TO_READ_INPUT_FILE, mem, brackets);
                }
                fscanf(files->input, "%d", &tmp);
                p->cvalue = (char) (tmp % 256);
                break;
        }
    }
    return finalBracketCheck(bracket, mem, brackets);
}

void printExecutionIssues(ExecutionStatus result, System* files) {
    switch (result)
    {
    case BRACKET_WAS_NOT_CLOSED:
        logError("No ']' character found after the '[' character", ERROR, files);
        break;
    case BRACKET_WAS_NOT_OPENED:
        logError("']' character found before the opening '[' character", ERROR, files);
        break;
    case FAILED_TO_READ_INPUT_FILE:
        logError("Tried to read from non-existing input.txt file", ERROR, files);
        break;
    case ERROR_WHILE_EXECUTING_THE_CODE: // TODO: make every error message meaningful
        logError("There was an error during the process", ERROR, files);
        break;
    }
}

int main() {
    System files;
    FileStatus status = openFiles(&files);
    if (status == FILES_OPENED_SUCCESSFULLY) {
        ExecutionStatus executionResult = parseCode(&files);
        if (executionResult == CODE_EXECUTED_WITHOUT_ISSUES) {
            printf("Program executed successfully!");
            if (closeFiles(&files) != FILES_CLOSED_SUCCESSFULLY) {
                logError("There was an error during the process", ERROR, &files);
                return -1;
            }
            return 0;
        } else {
            printExecutionIssues(executionResult, &files);
            closeFiles(&files);
            return -1;
        }
    } else if (status == COULD_NOT_OPEN_FILES) {
        return -1;
    }
}
