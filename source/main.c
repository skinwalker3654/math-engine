#include "../header/parser.h"
#include "../header/semantic.h"
#include "../header/interpreter.h"
#include <string.h>
#include <stdio.h>

void print_help(void) {
    printf("\n\033[1;34m╭─ Available commands ───────────────────╮\033[0m\n");
    printf("\033[1;34m│\033[0m \033[33mshowvars\033[0m  : shows all stored variables \033[1;34m│\033[0m\n");
    printf("\033[1;34m│\033[0m \033[33mshowfuncs\033[0m : shows all stored functions \033[1;34m│\033[0m\n");
    printf("\033[1;34m│\033[0m \033[33mdel VAR\033[0m   : removes a variable        \033[1;34m │\033[0m\n");
    printf("\033[1;34m│\033[0m \033[33mfdel FUNC\033[0m : removes a function        \033[1;34m │\033[0m\n");
    printf("\033[1;34m│\033[0m \033[33mclear\033[0m     : clears the terminal       \033[1;34m │\033[0m\n");
    printf("\033[1;34m│\033[0m \033[33mexit\033[0m      : exits the engine          \033[1;34m │\033[0m\n");
    printf("\033[1;34m╰────────────────────────────────────────╯\033[0m\n\n");
}

int main(void) {
    VariableList list = {0};
    SymbolTable table = {0};
    FunctionTable ftable = {0};
    FunctionList flist = {0};

    while(1) {
        char buffer[VAL_SIZE];
        printf("\033[1;36m[math-engine]\033[0m\033[1;32m>\033[0m ");
        fgets(buffer,VAL_SIZE,stdin);
        buffer[strcspn(buffer,"\n")] = '\0';
        
        if(strcmp(buffer,"exit")==0) {
            functions_free(&flist);
            printf("\033[1;33mexiting ...\033[0m\n");
            return 0;
        }

        if(strcmp(buffer,"help")==0) {
            print_help();
            continue;
        }

        Parser ps = parser_init(buffer);
        Ast *program = parse_program(&ps);
        if(!program) continue;

        if(analyze_program(&table,&ftable,program)==-1) {
            ast_destroy(program);
            continue;
        }

        if(execute_program(&list,&flist,&table,&ftable,program)==-1) {
            ast_destroy(program);
            continue;
        }

        ast_destroy(program);
    }

    return 0;
}
