#ifndef AST_H
#define AST_H

#define VAL_SIZE 256
#define MAX_ARGS 16

typedef enum {
    EXPR_NUM,
    EXPR_IDENT,
    EXPR_BIN,
    EXPR_UNARY,
    EXPR_CALL,
    EXPR_PI,
} ExprType;

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
} BinopType;

typedef struct Expr {
    ExprType type;
    union {
        float number;
        char ident[VAL_SIZE];
        struct {
            struct Expr *left;
            BinopType op;
            struct Expr *right;
        } bin;
        struct {
            struct Expr *ex;
            int negate;
        } unary;
        struct {
            char name[VAL_SIZE];
            struct Expr *args[MAX_ARGS];
            int count;
        } call;
    };
} Expr;

Expr *expr_new_num(float num);
Expr *expr_new_ident(char *ident);
Expr *expr_new_unary(Expr *ex, int negate);
Expr *expr_new_binary(Expr *left, BinopType op, Expr *right);
Expr *expr_new_call(char *name, Expr **args, int count);
Expr *expr_new_pi(void);
void expr_destroy(Expr *ex);
void expr_print(Expr *ex);

typedef enum {
    AST_EXPR,
    AST_VAR_DEC,
    AST_CLEAR,
    AST_DEL,
    AST_FDEL,
    AST_SHOWVARS,
    AST_SHOWFUNCS,
    AST_FUNC_DEF,
} AstType;

typedef struct Ast {
    AstType type;
    union {
        Expr *ex;
        struct {
            char varname[VAL_SIZE];
            Expr *ex;
        } var_dec;
        struct {
            char varname[VAL_SIZE];
        } del;
        struct {
            char funcname[VAL_SIZE];
        } fdel;
        struct {
            char name[VAL_SIZE];
            char params[MAX_ARGS][VAL_SIZE];
            int count;
            Expr *body;
        } func_def;
    } as;
    struct Ast *next;
} Ast;

Ast *ast_new_expr(Expr *ex);
Ast *ast_new_var_dec(char *varname, Expr *ex);
Ast *ast_new_clear(void);
Ast *ast_new_showvars(void);
Ast *ast_new_showfuncs(void);
Ast *ast_new_del(char *varname);
Ast *ast_new_fdel(char *funcname);
Ast *ast_new_func_def(char *name, char params[][VAL_SIZE], int count, Expr *body);

void ast_append(Ast **list, Ast *stmt);
void ast_destroy(Ast *list);
void ast_print(Ast *list);

#endif
