#include "../header/ast.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

Expr *expr_new_num(float num) {
    Expr *new = malloc(sizeof(Expr));
    if(!new) return NULL;
    new->type = EXPR_NUM;
    new->number = num;
    return new;
}

Expr *expr_new_ident(char *ident) {
    Expr *new = malloc(sizeof(Expr));
    if(!new) return NULL;
    new->type = EXPR_IDENT;
    strncpy(new->ident,ident,VAL_SIZE);
    return new;
}

Expr *expr_new_unary(Expr *ex, int negate) {
    Expr *new = malloc(sizeof(Expr));
    if(!new) return NULL;
    new->type = EXPR_UNARY;
    new->unary.ex = ex;
    new->unary.negate = negate;
    return new;
}

Expr *expr_new_binary(Expr *left, BinopType op, Expr *right) {
    Expr *new = malloc(sizeof(Expr));
    if(!new) return NULL;
    new->type = EXPR_BIN;
    new->bin.left = left;
    new->bin.op = op;
    new->bin.right = right;
    return new;
}

Expr *expr_new_call(char *name, Expr **args, int count) {
    Expr *new = malloc(sizeof(Expr));
    if(!new) return NULL;

    new->type =EXPR_CALL;
    strncpy(new->call.name,name,VAL_SIZE);
    new->call.count = count;
    for(int i=0; i<count; i++) 
        new->call.args[i] = args[i];
    return new;
}

Expr *expr_new_pi(void) {
    Expr *new = malloc(sizeof(Expr));
    if(!new) return NULL;

    new->type = EXPR_PI;
    return new;
}

void expr_destroy(Expr *ex) {
    switch(ex->type) {
        case EXPR_UNARY:
            expr_destroy(ex->unary.ex);
            free(ex);
            break;
        case EXPR_BIN:
            expr_destroy(ex->bin.left);
            expr_destroy(ex->bin.right);
            free(ex);
            break;
        case EXPR_CALL:
            for(int i=0; i<ex->call.count; i++)
                expr_destroy(ex->call.args[i]);
            free(ex);
            break;
        default:
            free(ex);
            break;
    }
}

void expr_print(Expr *ex) {
    switch(ex->type) {
        case EXPR_NUM:
            printf("\033[36m%.2f\033[0m",ex->number);
            break;
        case EXPR_IDENT:
            printf("\033[33m%s\033[0m",ex->ident);
            break;
        case EXPR_UNARY:
            printf("\033[35m%c\033[0m",ex->unary.negate ? '-' : '+');
            expr_print(ex->unary.ex);
            break;
        case EXPR_BIN:
            expr_print(ex->bin.left);
            switch(ex->bin.op) {
                case OP_ADD: printf(" \033[35m+\033[0m "); break;
                case OP_SUB: printf(" \033[35m-\033[0m "); break;
                case OP_MUL: printf(" \033[35m*\033[0m "); break;
                case OP_DIV: printf(" \033[35m/\033[0m "); break;
                case OP_MOD: printf(" \033[35m%%\033[0m "); break;
                case OP_POW: printf(" \033[35m^\033[0m "); break;
            }
            expr_print(ex->bin.right);
            break;
        case EXPR_CALL:
            printf("\033[32m%s\033[0m(",ex->call.name);
            for(int i=0; i<ex->call.count; i++) {
                expr_print(ex->call.args[i]);
                if(i < ex->call.count-1) printf("\033[90m,\033[0m");
            }
            printf(")");
            break;
        case EXPR_PI:
            printf("\033[36m3.14\033[0m");
            break;
    }
}

Ast *ast_new_expr(Expr *ex) {
    Ast *new = malloc(sizeof(Ast));
    if(!new) return NULL;
    new->type = AST_EXPR;
    new->as.ex = ex;
    new->next = NULL;
    return new;
}

Ast *ast_new_var_dec(char *varname, Expr *ex) {
    Ast *new = malloc(sizeof(Ast));
    if(!new) return NULL;
    new->type = AST_VAR_DEC;
    new->as.var_dec.ex = ex;
    strncpy(new->as.var_dec.varname,varname,VAL_SIZE);
    new->next = NULL;
    return new;
}

Ast *ast_new_clear(void) {
    Ast *new = malloc(sizeof(Ast));
    if(!new) return NULL;
    new->type = AST_CLEAR;
    new->next = NULL;
    return new;
}

Ast *ast_new_showvars(void) {
    Ast *new = malloc(sizeof(Ast));
    if(!new) return NULL;
    new->type = AST_SHOWVARS;
    new->next = NULL;
    return new;
}

Ast *ast_new_showfuncs(void) {
    Ast *new = malloc(sizeof(Ast));
    if(!new) return NULL;
    new->type = AST_SHOWFUNCS;
    new->next = NULL;
    return new;
}

Ast *ast_new_del(char *varname) {
    Ast *new = malloc(sizeof(Ast));
    if(!new) return NULL;
    new->type = AST_DEL;
    strncpy(new->as.del.varname,varname,VAL_SIZE);
    new->next = NULL;
    return new;
}

Ast *ast_new_fdel(char *funcname) {
    Ast *new = malloc(sizeof(Ast));
    if(!new) return NULL;
    new->type = AST_FDEL;
    strncpy(new->as.fdel.funcname,funcname,VAL_SIZE);
    new->next = NULL;
    return new;
}

Ast *ast_new_func_def(char *name, char params[][VAL_SIZE], int count, Expr *body) {
    Ast *new = malloc(sizeof(Ast));
    if(!new) return NULL;

    new->type = AST_FUNC_DEF;
    strncpy(new->as.func_def.name,name,VAL_SIZE);
    new->as.func_def.count = count;

    for(int i=0; i<count; i++)
        strncpy(new->as.func_def.params[i],params[i],VAL_SIZE);
    new->as.func_def.body = body;
    new->next = NULL;

    return new;
}

void ast_append(Ast **list, Ast *stmt) {
    if(!*list) {
        *list = stmt;
        return;
    }

    Ast *temp = *list;
    while(temp->next != NULL)
        temp = temp->next;
    temp->next = stmt;
}

void ast_destroy(Ast *list) {
    Ast *temp;
    while(list != NULL) {
        temp = list;
        list = list->next;
        switch(temp->type) {
            case AST_EXPR:
                expr_destroy(temp->as.ex);
                free(temp);
                break;
            case AST_VAR_DEC:
                expr_destroy(temp->as.var_dec.ex);
                free(temp);
                break;
            case AST_FUNC_DEF:
                expr_destroy(temp->as.func_def.body);
                free(temp);
                break;
            default:
                free(temp);
                break;
        }
    }
}

void ast_print(Ast *list) {
    while(list != NULL) {
        switch(list->type) {
            case AST_EXPR:
                printf("\033[1;34mEXPR:\033[0m ");
                expr_print(list->as.ex);
                printf("\n");
                break;
            case AST_VAR_DEC:
                printf("\033[1;34mVAR\033[0m \033[33m%s\033[0m: ",list->as.var_dec.varname);
                expr_print(list->as.var_dec.ex);
                printf("\n");
                break;
            case AST_CLEAR:
                printf("\033[1;34mCLEAR\033[0m\n");
                break;
            case AST_DEL:
                printf("\033[1;34mDEL\033[0m \033[33m%s\033[0m\n",list->as.del.varname);
                break;
            case AST_FDEL:
                printf("\033[1;34mFDEL\033[0m \033[32m%s\033[0m\n",list->as.fdel.funcname);
                break;
            case AST_SHOWVARS:
                printf("\033[1;34mSHOWVARS\033[0m\n");
                break;
            case AST_SHOWFUNCS:
                printf("\033[1;34mSHOWFUNCS\033[0m\n");
                break;
            case AST_FUNC_DEF:
                printf("\033[1;34mFN\033[0m \033[32m%s\033[0m(",list->as.func_def.name);
                for(int i=0; i<list->as.func_def.count; i++) {
                    printf("\033[33m%s\033[0m ",list->as.func_def.params[i]);
                    if(i < list->as.func_def.count-1) printf("\033[90m,\033[0m");
                }
                printf(") = ");
                expr_print(list->as.func_def.body);
                printf("\n");
                break;
        }
        list = list->next;
    }
}
