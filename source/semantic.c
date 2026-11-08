#include "../header/semantic.h"
#include <string.h>
#include <stdio.h>

void symbol_add(SymbolTable *table, char *varname) {
    if(table->count >= MAX_VARS) {
        printf("\033[1;31m✘ error:\033[0m no more memory for variables to create\n");
        return;
    }

    strncpy(table->variables[table->count],varname,VAL_SIZE);
    table->count++;
}

int symbol_exists(SymbolTable *table, char *varname) {
    for(int i=0; i<table->count; i++) {
        if(strcmp(table->variables[i],varname)==0) {
            return 1;
        }
    }
    return 0;
}

void func_add(FunctionTable *ft, char *name, int nparams) {
    if(ft->count >= MAX_FUNCS) {
        printf("\033[1;31m✘ error:\033[0m too many functions\n");
        return;
    }

    strncpy(ft->names[ft->count],name,VAL_SIZE);
    ft->nparams[ft->count] = nparams;
    ft->count++;
}

int func_find(FunctionTable *ft, char *name) {
    for(int i=0; i<ft->count; i++) {
        if(strcmp(ft->names[i],name)==0) {
            return i;
        } 
    }
    return -1;
}

static int isparam(char params[][VAL_SIZE], int nparams, char *name) {
    for(int i=0; i<nparams; i++)
        if(strcmp(params[i],name)==0) return 1;
    return 0;
}

static int analyze_expr(SymbolTable *table, FunctionTable *ft, char params[][VAL_SIZE], int nparams, Expr *ex) {
    switch(ex->type) {
        case EXPR_NUM:
            return 0;
        case EXPR_PI:
            return 0;
        case EXPR_IDENT:
            if(!symbol_exists(table,ex->ident) && !isparam(params,nparams,ex->ident)) {
                printf("\033[1;31m✘ error:\033[0m variable '%s' does not exists\n",ex->ident);
                return -1;
            }
            return 0;
        case EXPR_UNARY:
            return analyze_expr(table,ft,params,nparams,ex->unary.ex);
        case EXPR_BIN:
            if(analyze_expr(table,ft,params,nparams,ex->bin.left)==-1) return -1;
            if(analyze_expr(table,ft,params,nparams,ex->bin.right)==-1) return -1;
            return 0;
        case EXPR_CALL: {
            int index = func_find(ft,ex->call.name);
            if(index == -1) {
                printf("\033[1;31m✘ error:\033[0m function '%s' does not exists\n",ex->call.name);
                return -1;
            }

            if(ft->nparams[index] != ex->call.count) {
                printf("\033[1;31m✘ error:\033[0m function '%s' expects %d args not %d\n",
                        ex->call.name,ft->nparams[index],ex->call.count);
                return -1;
            }

            for(int i=0; i<ex->call.count; i++) {
                if(analyze_expr(table,ft,params,nparams,ex->call.args[i])==-1) {
                    return -1;
                } 
            } 

            return 0;
        }
    }
    return 0;
}

static int analyze_var_dec(SymbolTable *table, FunctionTable *ft, Ast *stmt) {
    if(strcmp(stmt->as.var_dec.varname,"PI")==0) {
        printf("\033[1;31m✘ error:\033[0m cannot name a variable 'PI' because thats a constant\n");
        return -1;
    }

    if(symbol_exists(table,stmt->as.var_dec.varname)) {
        printf("\033[1;31m✘ error:\033[0m variable '%s' already exists\n",stmt->as.var_dec.varname);
        return -1;
    }

    if(analyze_expr(table,ft,NULL,0,stmt->as.var_dec.ex)==-1) return -1;
    symbol_add(table,stmt->as.var_dec.varname);

    return 0;
}

static int analyze_func_def(SymbolTable *table,FunctionTable *ft, Ast *stmt) {
    if(strcmp(stmt->as.func_def.name,"PI")==0) {
        printf("\033[1;31m✘ error:\033[0m cannot name a function 'PI' because thats a constant\n");
        return -1;
    }

    if(func_find(ft,stmt->as.func_def.name)!=-1) {
        printf("\033[1;31m✘ error:\033[0m function '%s' already exists\n",stmt->as.func_def.name);
        return -1;
    }

    for(int i=0; i<stmt->as.func_def.count; i++) {
        for(int j=i+1; j<stmt->as.func_def.count; j++) {
            if(strcmp(stmt->as.func_def.params[i],
                        stmt->as.func_def.params[j])==0) {
                printf("\033[1;31m✘ error:\033[0m duplicate parameter '%s'\n",stmt->as.func_def.params[i]);
                return -1;
            }
        }
    }

    if(analyze_expr(table,ft,
                stmt->as.func_def.params,
                stmt->as.func_def.count,
                stmt->as.func_def.body)==-1) {
        return -1;
    }

    func_add(ft,stmt->as.func_def.name,stmt->as.func_def.count);
    return 0;
}

int analyze_program(SymbolTable *table, FunctionTable *ft ,Ast *list) {
    while(list != NULL) {
        switch(list->type) {
            case AST_EXPR:
                if(analyze_expr(table,ft,NULL,0,list->as.ex)==-1) return -1;
                break;
            case AST_VAR_DEC:
                if(analyze_var_dec(table,ft,list)==-1) return -1;
                break;
            case AST_DEL:
                if(strcmp(list->as.del.varname,"PI")==0) {
                    printf("\033[1;31m✘ error:\033[0m cannot delete PI, its a constant\n");
                    return -1;
                }

                if(!symbol_exists(table,list->as.del.varname)) {
                    printf("\033[1;31m✘ error:\033[0m variable '%s' does not exists\n",list->as.del.varname);
                    return -1;
                }
                break;
            case AST_FUNC_DEF:
                if(analyze_func_def(table,ft,list)==-1) return -1;
                break;
            case AST_FDEL:
                if(strcmp(list->as.fdel.funcname,"PI")==0) {
                    printf("\033[1;31m✘ error:\033[0m cannot delete PI, its a constant\n");
                    return -1;
                }

                if(func_find(ft,list->as.fdel.funcname)==-1) {
                    printf("\033[1;31m✘ error:\033[0m function '%s' does not exists\n",list->as.fdel.funcname);
                    return -1;
                }
                break;
            default:
                break;
        }
        list = list->next;
    }
    return 0;
}
