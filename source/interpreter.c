#include "../header/interpreter.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static Expr *expr_clone(Expr *ex) {
    if(!ex) return NULL;
    switch(ex->type) {
        case EXPR_NUM:   return expr_new_num(ex->number);
        case EXPR_PI:    return expr_new_pi();
        case EXPR_IDENT: return expr_new_ident(ex->ident);
        case EXPR_UNARY: return expr_new_unary(expr_clone(ex->unary.ex),ex->unary.negate);
        case EXPR_BIN:   return expr_new_binary(expr_clone(ex->bin.left),ex->bin.op,expr_clone(ex->bin.right));
        case EXPR_CALL: {
            Expr *args[MAX_ARGS];
            for(int i=0; i<ex->call.count; i++)
                args[i] = expr_clone(ex->call.args[i]);
            return expr_new_call(ex->call.name,args,ex->call.count);
        }
    }

    return NULL;
}


void var_add(VariableList *list, char *varname, float value) {
    if(list->count >= MAX_VARS) {
        printf("\033[1;31m✘ error:\033[0m you cannot create more variables\n");
        return;
    }

    strncpy(list->variables[list->count],varname,VAL_SIZE);
    list->values[list->count] = value;
    list->count++;
}

int var_find(VariableList *list, char *varname) {
    for(int i=0; i<list->count; i++) {
        if(strcmp(list->variables[i],varname)==0) {
            return i;
        }
    }
    return -1;
}

static void var_set(VariableList *list, char *name, float value) {
    int i = var_find(list,name);
    if(i >= 0) { 
        list->values[i] = value;
        return; 
    }

    var_add(list, name, value);
}

static void var_remove(VariableList *list, char *name) {
    int idx = var_find(list, name);
    if(idx < 0) return;

    for(int i=idx; i<list->count-1; i++) {
        list->values[i] = list->values[i+1];
        strncpy(list->variables[i],list->variables[i+1],VAL_SIZE);
    }

    list->count--;
}

static int find_func(FunctionList *fl,char *name) {
    for(int i=0; i<fl->count; i++)
        if(strcmp(fl->funcs[i].name,name)==0) return i;
    return -1;
}

static void def_func(FunctionList *fl, Ast *stmt) {
    if(fl->count >= MAX_FUNCS) {
        printf("\033[1;31m✘ error:\033[0m too many functions\n");
        return;
    }

    Function *f = &fl->funcs[fl->count++];
    strncpy(f->name, stmt->as.func_def.name,VAL_SIZE);
    f->count = stmt->as.func_def.count;

    for(int i=0; i<f->count; i++)
        strncpy(f->params[i],stmt->as.func_def.params[i],VAL_SIZE);
    f->body = expr_clone(stmt->as.func_def.body);
}

static int evaluate_expr(VariableList *list, FunctionList *fl, Expr *ex, float *out) {
    switch(ex->type) {
        case EXPR_NUM:
            *out = ex->number;
            return 0;
        case EXPR_PI:
            *out = 3.14;
            return 0;
        case EXPR_BIN: {
            float left;
            float right;
            if(evaluate_expr(list,fl,ex->bin.left,&left)==-1) return -1;
            if(evaluate_expr(list,fl,ex->bin.right,&right)==-1) return -1;
            switch(ex->bin.op) {
                case OP_ADD: *out = left + right; return 0;
                case OP_SUB: *out = left - right; return 0;
                case OP_MUL: *out = left * right; return 0;
                case OP_DIV: 
                    if(right == 0) {
                        printf("\033[1;31m✘ error:\033[0m division by 0\n");
                        return -1;
                    }
                    *out = left / right; return 0;
                case OP_MOD:
                    *out = fmod(left,right);
                    return 0;
            }
        }
        case EXPR_IDENT:
            *out = list->values[var_find(list,ex->ident)];
            return 0;
        case EXPR_UNARY: {
            float expr;
            if(evaluate_expr(list,fl,ex->unary.ex,&expr)==-1) return -1;
            *out = ex->unary.negate ? expr*-1 : expr;
            return 0;
        }
        case EXPR_CALL: {
            int index = find_func(fl,ex->call.name);
            Function *f = &fl->funcs[index];

            float argvals[MAX_PARAMS];
            for(int i=0; i<f->count; i++) {
                if(evaluate_expr(list,fl,ex->call.args[i],&argvals[i])==-1)
                    return -1;
            }

            float savedvals[MAX_PARAMS];
            int had[MAX_PARAMS];
            
            for(int i=0; i<f->count; i++) {
                int j = var_find(list,f->params[i]);
                had[i] = (j >= 0);
                if(j >= 0) savedvals[i] = list->values[j];
            }

            for(int i=0; i<f->count; i++)
                var_set(list,f->params[i],argvals[i]);
            
            float result;
            int rc = evaluate_expr(list,fl,f->body,&result);

            for(int i=0; i<f->count; i++) {
                if(had[i]) var_set(list,f->params[i],savedvals[i]);
                else var_remove(list,f->params[i]);
            }

            if(rc == -1) return -1;
            *out = result;

            return 0;
        }
    }
}

static int execute_expr_stmt(VariableList *list, FunctionList *fl, Ast *stmt) {
    float result;
    if(evaluate_expr(list,fl,stmt->as.ex,&result)==-1) return -1;
    printf("\033[1;36m➜\033[0m \033[1;37m%.2f\033[0m\n",result);
    return 0;
}

static int execute_var_dec(VariableList *list, FunctionList *fl, Ast *stmt) {
    float result;
    if(evaluate_expr(list,fl,stmt->as.var_dec.ex,&result)==-1) return -1;
    var_add(list,stmt->as.var_dec.varname,result);
    printf("\033[1;32m✔\033[0m variable '\033[33m%s\033[0m' has been added successfully\n",stmt->as.var_dec.varname);
    return 0;
}

static int execute_del(VariableList *list, SymbolTable *table, Ast *stmt) {
    int index = var_find(list,stmt->as.del.varname);
    for(int i=index; i<list->count-1; i++) {
        list->values[i] = list->values[i+1];
        strncpy(list->variables[i],list->variables[i+1],VAL_SIZE);
    }
    list->count--;

    for(int i=index; i<table->count-1; i++) 
        strncpy(table->variables[i],table->variables[i+1],VAL_SIZE);
    table->count--;

    printf("\033[1;32m✔\033[0m variable '\033[33m%s\033[0m' has been deleted successfully\n",stmt->as.del.varname);
    return 0;
}


static int execute_fdel(FunctionList *list, FunctionTable *table, Ast *stmt) {
    int index = find_func(list,stmt->as.fdel.funcname);
    expr_destroy(list->funcs[index].body);

    for(int i=index; i<list->count-1; i++) 
        list->funcs[i] = list->funcs[i+1];
    list->count--;

    for(int i=index; i<table->count-1; i++) {
        strncpy(table->names[i],table->names[i+1],VAL_SIZE);
        table->nparams[i] = table->nparams[i+1];
    }
    table->count--;

    printf("\033[1;32m✔\033[0m function '\033[32m%s\033[0m' has been deleted successfully\n",stmt->as.del.varname);
    return 0;
}

static int execute_showvars(VariableList *list) {
    if(list->count == 0) {
        printf("\033[90m(no variables yet)\033[0m\n");
        return 0;
    }

    printf("\n\033[1;34m╔═══════════════════════════╗\033[0m\n");
    printf("\033[1;34m║\033[0m \033[1;37m%-13s\033[0m \033[1;34m│\033[0m \033[1;37m%-9s\033[0m \033[1;34m║\033[0m\n","Variable name","Value");
    printf("\033[1;34m╠═══════════════════════════╣\033[0m\n");
    for(int i=0; i<list->count; i++)
        printf("\033[1;34m║\033[0m \033[33m%-13s\033[0m \033[1;34m│\033[0m \033[36m%-9.2f\033[0m \033[1;34m║\033[0m\n",
               list->variables[i],list->values[i]);
    printf("\033[1;34m╚═══════════════════════════╝\033[0m\n\n");
    return 0;
}

static int execute_showfuncs(FunctionList *list) {
    if(list->count == 0) {
        printf("\033[90m(no functions yet)\033[0m\n");
        return 0;
    }

    printf("\n\033[1;35m── Functions List ─────────────\033[0m\n");
    for(int i=0; i<list->count; i++) {
        printf(" \033[32m%s\033[0m(",list->funcs[i].name);
        for(int j=0; j<list->funcs[i].count; j++) {
            printf("\033[33m%s\033[0m",list->funcs[i].params[j]);
            if(j < list->funcs[i].count-1) printf("\033[90m,\033[0m");
        }
        printf(") \033[35m=\033[0m ");
        expr_print(list->funcs[i].body);
        printf("\n");
    }
    printf("\033[1;35m───────────────────────────────\033[0m\n\n");
    return 0;
}

int execute_program(VariableList *list, FunctionList *fl, SymbolTable *table, FunctionTable *ftable, Ast *program) {
    while(program != NULL) {
        switch(program->type) {
            case AST_EXPR:
                if(execute_expr_stmt(list,fl,program)==-1) return -1;
                break;
            case AST_VAR_DEC:
                if(execute_var_dec(list,fl,program)==-1) return -1;
                break;
            case AST_CLEAR:
                system("clear");
                break;
            case AST_DEL:
                if(execute_del(list,table,program)==-1) return -1;
                break;
            case AST_SHOWVARS:
                if(execute_showvars(list)==-1) return -1;
                break;
            case AST_FUNC_DEF:
                def_func(fl,program);
                printf("\033[1;32m✔\033[0m function '\033[32m%s\033[0m' has been added successfully\n",program->as.func_def.name);
                break;
            case AST_SHOWFUNCS:
                if(execute_showfuncs(fl)==-1) return -1;
                break;
            case AST_FDEL:
                if(execute_fdel(fl,ftable,program)==-1) return -1;
                break;
        }
        program = program->next;
    }

    return 0;
}

void functions_free(FunctionList *fl) {
    for(int i=0; i<fl->count; i++)
        expr_destroy(fl->funcs[i].body);
    fl->count = 0;
}
