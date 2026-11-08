#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "tokenizer.h"

typedef struct {
    Lexer lexer;
    Token current;
    Token next;
} Parser;

Parser parser_init(char *source);
Ast *parse_program(Parser *ps);

#endif
