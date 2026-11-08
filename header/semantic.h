#ifndef SYMBOL_H
#define SYMBOL_H

#include "ast.h"
#define VAL_SIZE 256
#define MAX_VARS 100
#define MAX_FUNCS 64

typedef struct {
    char variables[MAX_VARS][VAL_SIZE];
    int count;
} SymbolTable;

typedef struct {
    char names[MAX_FUNCS][VAL_SIZE];
    int nparams[MAX_FUNCS];
    int count;
} FunctionTable;

void symbol_add(SymbolTable *table, char *varname);
int symbol_exists(SymbolTable *table, char *varname);

void func_add(FunctionTable *ft, char *name, int nparams);
int fund_find(FunctionTable *ft, char *name);

int analyze_program(SymbolTable *table, FunctionTable *ft,  Ast *list);

#endif
