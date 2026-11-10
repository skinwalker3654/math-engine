#ifndef TOKENIZER_H
#define TOKENIZER_H

#define VAL_SIZE 256
#define SRC_SIZE 1024

typedef enum {
    TOK_PI,
    TOK_SHOWVARS,
    TOK_SHOWFUNCS,
    TOK_CLEAR,
    TOK_DEL,
    TOK_FDEL,
    TOK_NUMBER,
    TOK_IDENT,
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_MODULO,
    TOK_CARET,
    TOK_ASSIGN,
    TOK_LPAR,
    TOK_RPAR,
    TOK_COMMA,
    TOK_EOF,
    TOK_ERR,
} TokenType;

typedef struct {
    char value[VAL_SIZE];
    TokenType type;
} Token;

typedef struct {
    char source[SRC_SIZE];
    int pos;
} Lexer;

Lexer lexer_init(char *source);
Token lexer_get_next(Lexer *lx);
void token_print(Token tk);

#endif
