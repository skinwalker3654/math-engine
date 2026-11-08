#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "semantic.h"

#define MAX_VARS 100
#define VAL_SIZE 256
#define MAX_FUNCS 64
#define MAX_PARAMS 8

typedef struct {
    char variables[MAX_VARS][VAL_SIZE];
    float values[MAX_VARS];
    int count;
} VariableList;

typedef struct {
    char name[VAL_SIZE];
    char params[MAX_VARS][VAL_SIZE];
    int count;
    Expr *body;
} Function;

typedef struct {
    Function funcs[MAX_FUNCS];
    int count;
} FunctionList;

void var_add(VariableList *list, char *varname, float value);
int var_find(VariableList *list, char *varname);
int execute_program(VariableList *list, FunctionList *fl, SymbolTable *table, FunctionTable *ftable, Ast *program);
void functions_free(FunctionList *fl);

#endif
