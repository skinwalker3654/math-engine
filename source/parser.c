#include "../header/parser.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

Parser parser_init(char *source) {
    Parser new = {0};
    new.lexer = lexer_init(source);
    new.current = lexer_get_next(&new.lexer);
    new.next = lexer_get_next(&new.lexer);
    return new;
}

static int match(Token tk, TokenType type) {
    return tk.type == type;
}

static void consume(Parser *ps) {
    ps->current = ps->next;
    ps->next = lexer_get_next(&ps->lexer);
}

static int expect(Parser *ps, TokenType type, char *msg) {
    if(match(ps->current,type)) {
        consume(ps);
        return 0;
    }

    printf("\033[1;31m✘ error:\033[0m %s\n",msg);
    return -1;
}

Expr *parse_expr(Parser *ps);
static Expr *parse_primary(Parser *ps) {
    if(match(ps->current,TOK_IDENT)) {
        char name[VAL_SIZE];
        strncpy(name,ps->current.value,VAL_SIZE);
        consume(ps);

        /* parsing call */
        if(match(ps->current,TOK_LPAR)) {
            consume(ps);

            Expr *args[MAX_ARGS];
            int count = 0;

            if(!match(ps->current,TOK_RPAR)) {
                do {
                    Expr *arg = parse_expr(ps);
                    if(!arg) goto cleanup;

                    if(count >= MAX_ARGS) {
                        printf("\033[1;31m✘ error:\033[0m too many arguments\n");
                        expr_destroy(arg);
                        goto cleanup;
                    }

                    args[count++] = arg;
                    if(match(ps->current,TOK_COMMA)) consume(ps);
                    else break;
                } while(1);
            }
            
            if(expect(ps,TOK_RPAR,"expected closing parenthesis after arguments")==-1)
                goto cleanup;

            return expr_new_call(name,args,count);

            cleanup:
                for(int i=0; i<count; i++) expr_destroy(args[i]);
                return NULL;
        }
        
        return expr_new_ident(name);
    }

    if(match(ps->current,TOK_PI)) {
        if(match(ps->next,TOK_ASSIGN)) {
            printf("\033[1;31m✘ error:\033[0m cannot assign values to constant PI\n");
            return NULL;
        }

        Expr *new = expr_new_pi();
        if(!new) return NULL;
        consume(ps);
        return new;
    }

    if(match(ps->current,TOK_NUMBER)) {
        Expr *new = expr_new_num(atof(ps->current.value));
        if(!new) return NULL;
        consume(ps);
        return new;
    }

    if(match(ps->current,TOK_LPAR)) {
        consume(ps);

        Expr *ex = parse_expr(ps);
        if(!ex) return NULL;

        if(expect(ps,TOK_RPAR,"expected closing parenthesis")==-1) {
            expr_destroy(ex);
            return NULL;
        }

        return ex;
    }

    printf("\033[1;31m✘ error:\033[0m invalid expression\n");
    return NULL;
}

static Expr *parse_unary(Parser *ps) {
    if(match(ps->current,TOK_PLUS)) {
        consume(ps);
        Expr *ex = parse_unary(ps);
        if(!ex) return NULL;
        return expr_new_unary(ex,0);
    }

    if(match(ps->current,TOK_MINUS)) {
        consume(ps);
        Expr *ex = parse_unary(ps);
        if(!ex) return NULL;
        return expr_new_unary(ex,1);
    }

    return parse_primary(ps);
}

static Expr *parse_multiplicative(Parser *ps) {
    Expr *left = parse_unary(ps);
    if(!left) return NULL;

    while(match(ps->current,TOK_STAR) || match(ps->current,TOK_SLASH) || match(ps->current,TOK_MODULO)) {
        BinopType op;
        if(match(ps->current,TOK_SLASH))
            op = OP_DIV;
        else if(match(ps->current,TOK_STAR)) 
            op = OP_MUL;
        else
            op = OP_MOD;
        consume(ps);

        Expr *right = parse_unary(ps);
        if(!right) {
            expr_destroy(left);
            return NULL;
        }

        left = expr_new_binary(left,op,right);
        if(!left) {
            expr_destroy(right);
            return NULL;
        }
    }

    return left;
}

Expr *parse_expr(Parser *ps) {
    Expr *left = parse_multiplicative(ps);
    if(!left) return NULL;

    while(match(ps->current,TOK_PLUS) || match(ps->current,TOK_MINUS)) {
        BinopType op;
        if(match(ps->current,TOK_PLUS))
            op = OP_ADD;
        else 
            op = OP_SUB;
        consume(ps);

        Expr *right = parse_multiplicative(ps);
        if(!right) {
            expr_destroy(left);
            return NULL;
        }

        left = expr_new_binary(left,op,right);
        if(!left) {
            expr_destroy(right);
            return NULL;
        }
    }

    return left;
}

static Ast *parse_expr_stmt(Parser *ps) {
    Expr *ex = parse_expr(ps);
    if(!ex) return NULL;

    if(!match(ps->current,TOK_EOF)) {
        printf("\033[1;31m✘ error:\033[0m invalid epxression\n");
        expr_destroy(ex);
        return NULL;
    }

    return ast_new_expr(ex);
}

static Ast *parse_var_dec(Parser *ps) {
    char varname[VAL_SIZE];
    strncpy(varname,ps->current.value,VAL_SIZE);
    consume(ps);

    if(expect(ps,TOK_ASSIGN,"expected '=' after variable name")==-1) return NULL;

    Expr *ex = parse_expr(ps);
    if(!ex) return NULL;

    if(!match(ps->current,TOK_EOF)) {
        printf("\033[1;31m✘ error:\033[0m invalid variable decleration\n");
        expr_destroy(ex);
        return NULL;
    }

    return ast_new_var_dec(varname,ex);
}

static Ast *parse_var_dec_or_func_def(Parser *ps) {
    char name[VAL_SIZE];
    strncpy(name, ps->current.value, VAL_SIZE);

    if(match(ps->current,TOK_ASSIGN)) return parse_var_dec(ps);

    if(match(ps->next, TOK_LPAR)) {
        int save_pos = ps->lexer.pos;
        Token save_cur = ps->current;
        Token save_next = ps->next;

        consume(ps);        
        consume(ps);

        char params[MAX_ARGS][VAL_SIZE];
        int count = 0;

        if(!match(ps->current, TOK_RPAR)) {
            while(1) {
                if(!match(ps->current, TOK_IDENT)) 
                    goto rewind;

                if(count >= MAX_ARGS)
                    goto rewind;

                strncpy(params[count++],ps->current.value,VAL_SIZE);
                consume(ps);

                if(match(ps->current,TOK_COMMA)) {
                    consume(ps); 
                    continue; 
                }

                break;
            }
        }

        if(!match(ps->current,TOK_RPAR)) goto rewind;
        consume(ps);

        if(!match(ps->current,TOK_ASSIGN)) goto rewind;
        consume(ps);

        Expr *body = parse_expr(ps);
        if(!body) goto rewind;

        if(!match(ps->current, TOK_EOF)) {
            expr_destroy(body);
            goto rewind;
        }

        return ast_new_func_def(name,params,count,body);

        rewind:
            ps->lexer.pos = save_pos;
            ps->current   = save_cur;
            ps->next      = save_next;
            return NULL;   
    }

    return NULL;
}

static Ast *parse_clear(Parser *ps) {
    consume(ps);
    if(!match(ps->current,TOK_EOF)) {
        printf("\033[1;31m✘ error:\033[0m invalid clear syntax\n");
        return NULL;
    }

    return ast_new_clear();
}

static Ast *parse_del(Parser *ps) {
    consume(ps);

    if(!match(ps->current,TOK_IDENT)) {
        printf("\033[1;31m✘ error:\033[0m expected variable name to delete\n");
        return NULL;
    }

    char varname[VAL_SIZE];
    strncpy(varname,ps->current.value,VAL_SIZE);
    consume(ps);

    if(!match(ps->current,TOK_EOF)) {
        printf("\033[1;31m✘ error:\033[0m invalid del syntax\n");
        return NULL;
    }

    return ast_new_del(varname);
}

static Ast *parse_fdel(Parser *ps) {
    consume(ps);

    if(!match(ps->current,TOK_IDENT)) {
        printf("\033[1;31m✘ error:\033[0m expected function name to delete\n");
        return NULL;
    }

    char funcname[VAL_SIZE];
    strncpy(funcname,ps->current.value,VAL_SIZE);
    consume(ps);

    if(!match(ps->current,TOK_EOF)) {
        printf("\033[1;31m✘ error:\033[0m invalid fdel syntax\n");
        return NULL;
    }

    return ast_new_fdel(funcname);
}

static Ast *parse_showvars(Parser *ps) {
    consume(ps);
    if(!match(ps->current,TOK_EOF)) {
        printf("\033[1;31m✘ error:\033[0m invalid showvars syntax\n");
        return NULL;
    }

    return ast_new_showvars();
}

static Ast *parse_showfuncs(Parser *ps) {
    consume(ps);
    if(!match(ps->current,TOK_EOF)) {
        printf("\033[1;31m✘ error:\033[0m invalid showfuncs syntax\n");
        return NULL;
    }

    return ast_new_showfuncs();
}

static Ast *parse_stmt(Parser *ps) {
    if(match(ps->current,TOK_IDENT)) {
        Ast *fd = parse_var_dec_or_func_def(ps);
        if(fd) return fd;
        if(match(ps->next,TOK_ASSIGN)) return parse_var_dec(ps);
        return parse_expr_stmt(ps);
    } else if(match(ps->current,TOK_NUMBER) ||
              match(ps->current,TOK_LPAR) ||
              match(ps->current,TOK_MINUS) ||
              match(ps->current,TOK_PI) ||
              match(ps->current,TOK_PLUS)) {
        return parse_expr_stmt(ps);
    } else if(match(ps->current,TOK_SHOWVARS)) {
        return parse_showvars(ps);
    } else if(match(ps->current,TOK_SHOWFUNCS)) {
        return parse_showfuncs(ps);
    } else if(match(ps->current,TOK_CLEAR)) {
        return parse_clear(ps);
    } else if(match(ps->current,TOK_DEL)) {
        return parse_del(ps);
    } else if(match(ps->current,TOK_FDEL)) {
        return parse_fdel(ps);
    } else {
        printf("\033[1;31m✘ error:\033[0m invalid statement starting with ");
        token_print(ps->current);
        return NULL;
    }
}

Ast *parse_program(Parser *ps) {
    Ast *program = NULL;
    while(!match(ps->current,TOK_EOF)) {
        Ast *stmt = parse_stmt(ps);
        if(!stmt) {
            ast_destroy(program);
            return NULL;
        }
        ast_append(&program,stmt);
    }

    return program;
}
