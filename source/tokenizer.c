#include "../header/tokenizer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

typedef struct {
    char *name;
    TokenType type;
} Keyword_t;

Keyword_t keywords_list[] = {
    {"del",TOK_DEL},
    {"showvars",TOK_SHOWVARS},
    {"clear",TOK_CLEAR},
    {"showfuncs",TOK_SHOWFUNCS},
    {"fdel",TOK_FDEL},
    {"PI",TOK_PI},
    {NULL,TOK_ERR},
};

Lexer lexer_init(char *source) {
    Lexer new = {0};
    strncpy(new.source,source,SRC_SIZE);
    return new;
}

static char peek(Lexer *lx) {
    return lx->source[lx->pos];
}

static char advance(Lexer *lx) {
    char ch = peek(lx);
    lx->pos++;

    return ch;
}

static void skip_ws(Lexer *lx) {
    while(isspace(peek(lx))) {
        advance(lx);
    }
}

Token lexer_get_next(Lexer *lx) {
    skip_ws(lx);
    char current = peek(lx);

    if(current == '\0') return (Token){.type=TOK_EOF};
    
    if(isdigit(current)) {
        char buffer[VAL_SIZE];
        int pos = 0;

        while(isdigit(peek(lx)) || peek(lx) == '.')
            buffer[pos++] = advance(lx);
        buffer[pos] = '\0';

        char *endPtr;
        float number = strtof(buffer,&endPtr);
        if(*endPtr != '\0') {
            printf("\033[1;31m✘ error:\033[0m invalid number '%s'\n",buffer);
            return (Token){.type=TOK_ERR};
        }

        Token new = {.type=TOK_NUMBER};
        strncpy(new.value,buffer,VAL_SIZE);

        return new;
    }

    if(isalpha(current)) {
        char buffer[VAL_SIZE];
        int pos = 0;

        while(isalpha(peek(lx)))
            buffer[pos++] = advance(lx);
        buffer[pos] = '\0';

        for(int i=0; keywords_list[i].name; i++) {
            if(strcmp(keywords_list[i].name,buffer)==0) {
                return (Token){.type=keywords_list[i].type};
            }
        }

        Token new = {.type=TOK_IDENT};
        strncpy(new.value,buffer,VAL_SIZE);

        return new;
    }
    
    char sym = advance(lx);

    switch(sym) {
        case '+': return (Token){.type=TOK_PLUS};
        case '-': return (Token){.type=TOK_MINUS};
        case '*': return (Token){.type=TOK_STAR};
        case '/': return (Token){.type=TOK_SLASH};
        case '(': return (Token){.type=TOK_LPAR};
        case ')': return (Token){.type=TOK_RPAR};
        case '=': return (Token){.type=TOK_ASSIGN};
        case '%': return (Token){.type=TOK_MODULO};
        case ',': return (Token){.type=TOK_COMMA};
    }

    printf("\033[1;31m✘ error:\033[0m invalid character '%c'\n",sym);
    return (Token){.type=TOK_ERR};
}

void token_print(Token tk) {
    switch(tk.type) {
        case TOK_ASSIGN:

            printf("ASSIGN\n");
            break;
        case TOK_EOF:
            printf("EOF\n");
            break;
        case TOK_ERR:
            printf("ERR\n");
            break;
        case TOK_IDENT:
            printf("IDENT(%s)\n",tk.value);
            break;
        case TOK_LPAR:
            printf("LPAR\n");
            break;
        case TOK_MINUS:
            printf("MINUS\n");
            break;
        case TOK_MODULO:
            printf("MODULO\n");
            break;
        case TOK_NUMBER:
            printf("NUMBER(%s)\n",tk.value);
            break;
        case TOK_PLUS:
            printf("PLUS\n");
            break;
        case TOK_RPAR:
            printf("RPAR\n");
            break;
        case TOK_SLASH: 
            printf("SLASH\n");
            break;
        case TOK_STAR: 
            printf("STAR\n");
            break;
        case TOK_DEL: 
            printf("DEL\n");
            break;
        case TOK_SHOWVARS: 
            printf("SHOWVARS\n");
            break;
        case TOK_CLEAR:
            printf("CLEAR\n");
            break;
        case TOK_COMMA:
            printf("COMMA\n");
            break;
        case TOK_SHOWFUNCS:
            printf("SHOWFUNCS\n");
            break;
        case TOK_FDEL:
            printf("FDEL\n");
            break;
        case TOK_PI:
            printf("PI(3.14)\n");
            break;
    }
}
