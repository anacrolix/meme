#include <glib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    ATOM,
    START,
    END,
} TokenType;

typedef struct {
    TokenType type;
    char *value;
} Token;

typedef struct {
    GSList *indents;
    FILE *file;
    int cur_indent;
    bool new_line;
    GSList *queue;
} Lexer;

void read_atom(Lexer *toker, char **value) {
    char lval[0x100];
    int len = 0;
    for (; len < sizeof lval - 1; len++) {
        int c = fgetc(toker->file);
        if (c < 0) break;
        switch (c) {
        case ' ':
        case '(':
        case ')':
        case '\n':
            if (c != ungetc(c, toker->file)) abort();
            goto end_loop;
        }
        lval[len] = c;
    }
end_loop:
    if (!len) abort();
    lval[len] = '\0';
    *value = strdup(lval);
}

bool next_token(Lexer *lexer, Token *token) {
    if (lexer->queue) {
        *token = *(Token *)lexer->queue->data;
        lexer->queue = g_slist_delete_link(lexer->queue, lexer->queue);
        return true;
    }
    if (!lexer->file) return false;
    for (;;) {
        int c = fgetc(lexer->file);
        if (c < 0) {
            if (ferror(lexer->file)) abort();
            if (!lexer->indents) {
                lexer->file = NULL;
                token->type = END;
                return true;
            }
            lexer->indents = g_slist_remove_link(lexer->indents, lexer->indents);
            token->type = END;
            return true;
        }
        switch (c) {
        case ' ':
            break;
        case '\n':
            lexer->cur_indent = 0;
            lexer->new_line = true;
            continue;
        case '(':
        case ')':
            *token = (Token){
                .type = (c == '(') ? START : END,
            };
            lexer->cur_indent++;
            return true;
        default:
            if (c != ungetc(c, lexer->file)) abort();
            if (lexer->new_line) {
                if (lexer->indents && lexer->cur_indent <= GPOINTER_TO_INT(lexer->indents->data)) {
                    lexer->indents = g_slist_remove_link(lexer->indents, lexer->indents);

                    token->type = END;
                    return true;
                }
                lexer->new_line = false;
                token->type = START;
                lexer->indents = g_slist_prepend(lexer->indents, GINT_TO_POINTER(lexer->cur_indent));
                return true;
            }
            read_atom(lexer, &token->value);
            token->type = ATOM;
            return true;
        }
        lexer->cur_indent++;
    }
}

bool peek_token(Lexer *lexer, Token const **token) {
    if (lexer->queue) {
        *token = lexer->queue->data;
        return true;
    }
    Token *t = malloc(sizeof *t);
    if (!next_token(lexer, t)) return false;
    lexer->queue = g_slist_prepend(lexer->queue, t);
    *token = t;
    return true;
}

typedef enum {
    VARREF,
    DOUBLE,
    INTEGER,
    STRING,
    LIST,
    DEFINE,
} Form;

typedef struct Node {
    Form form;
    union {
        long long i;
        double f;
        char *s;
        GSList *list;
        struct {
            char *var;
            struct Node *exp;
        } define;
    };
} Node;

Node *parse(Lexer *lexer);

void burn_end_token(Lexer *lexer) {
    Token token[1];
    if (!next_token(lexer, token)) abort();
    if (token->type != END) {
        fprintf(stderr, "expected list end\n");
        abort();
    }
}

void drop_token(Lexer *lexer) {
    Token token[1];
    if (!next_token(lexer, token)) abort();
}

Node *parse_list(Lexer *lexer) {
    Token const *first;
    if (!peek_token(lexer, &first)) abort();
    Node *ret = malloc(sizeof *ret);
    ret->form = LIST;
    if (first->type == ATOM) {
        if (!strcmp(first->value, "define")) {
            ret->form = DEFINE;
            drop_token(lexer);
            Token var_token[1];
            if (!next_token(lexer, var_token)) abort();
            ret->define.var = var_token->value;
            ret->define.exp = parse(lexer);
        }
        if (ret->form != LIST) {
            burn_end_token(lexer);
            return ret;
        }
    }
    ret->list = NULL;
    for (;;) {
        Token const *token;
        if (!peek_token(lexer, &token)) abort();
        if (token->type == END) break;
        Node *node = parse(lexer);
        ret->list = g_slist_prepend(ret->list, node);
    }
    burn_end_token(lexer);
    ret->list = g_slist_reverse(ret->list);
    return ret;
}

Node *parse(Lexer *lexer) {
    Token tn[1];
    if (!next_token(lexer, tn)) return NULL;
    Node *node = malloc(sizeof *node);
    switch (tn->type) {
    case ATOM:
        if (tn->value[0] == '"') {
            node->form = STRING;
            node->s = strdup(tn->value + 1);
            return node;
        } else if (strchr(tn->value, '.')) {
            node->form = DOUBLE;
            if (1 != sscanf(tn->value, "%lf", &node->f)) abort();
        } else if (isdigit(tn->value[0])) {
            node->form = INTEGER;
            if (1 != sscanf(tn->value, "%lld", &node->i)) abort();
        } else {
            node->form = VARREF;
            node->s = strdup(tn->value);
        }
        break;
    case START:
        return parse_list(lexer);
        break;
    default:
        fprintf(stderr, "syntax error\n");
        abort();
        free(node);
        node = NULL;
    }
    return node;
}

void print_atom(Node const *node, bool *just_atom) {
    if (*just_atom) putchar(' ');
    switch (node->form) {
    case VARREF:
        fputs(node->s, stdout);
        break;
    case INTEGER:
        printf("%lld", node->i);
        break;
    default:
        fprintf(stderr, "can't print atom node form: %d\n", node->form);
        abort();
    }
    *just_atom = true;
}

void print_node(Node const *node, bool *just_atom) {
    switch (node->form) {
    case LIST:
        putchar('(');
        *just_atom = false;
        g_slist_foreach(node->list, (GFunc)print_node, just_atom);
        putchar(')');
        *just_atom = false;
        break;
    case DEFINE:
        printf("(define %s", node->define.var);
        *just_atom = false;
        print_node(node->define.exp, just_atom);
        putchar(')');
        break;
    default:
        print_atom(node, just_atom); 
    }
}

typedef struct Env {
    GHashTable *dict;
    struct Env *outer;
} Env;

Node *env_find(Env *env, char const *var) {
    if (env == NULL) {
        fprintf(stderr, "symbol not found: %s\n", var);
        return NULL;
    }
    void *value;
    if (!g_hash_table_lookup_extended(env->dict, var, NULL, &value)) {
        return env_find(env->outer, var);
    }
    if (!value) {
        fprintf(stderr, "uninitialized: %s\n", var);
        abort();
    }
    return value;
}

void env_init(Env *env, Env *outer) {
    *env = (Env){
        .dict = g_hash_table_new(g_str_hash, g_str_equal),
        .outer = outer,
    };
}

void env_destroy(Env *env) {
    g_hash_table_destroy(env->dict);
}

void bind_args(GHashTable *dict, GSList *names, GSList *values) {
    GSList *name = names, *value = values;
    for (;;) {
        if (!name || !value) break;
        g_hash_table_insert(dict, name, value);
        name = name->next;
        value = value->next;
    }
    if (!!name ^ !!value) {
        fprintf(stderr, "argument count mismatch\n");
        abort();
    }
}

Node *eval(Node *node, Env *env) {
    switch (node->form) {
    case VARREF:
        return env_find(env, node->s);
    case LIST:
        {
            GSList *exps = NULL;
            for (GSList *x = node->list; x; x = x->next) {
                exps = g_slist_prepend(exps, eval(x->data, env));
            }
            exps = g_slist_reverse(exps);
            Node *proc = exps->data;
            GSList *args = exps->next;
            GSList *arg_names = ((Node *)(proc->list->data))->list;
            Node *exp = proc->list->next->data;
            Env sub_env[1];
            env_init(sub_env, env);
            bind_args(sub_env->dict, arg_names, args);
            Node *ret = eval(exp, sub_env);
            env_destroy(sub_env);
            return ret;
        }
    case DEFINE:
        

    default:
        abort();
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s script\n", argv[0]);
        return 2;
    }
    FILE *token_file = fopen(argv[1], "rb");
    if (!token_file) {
        perror("fopen");
        return 1;
    }
    Lexer lexer = {
        .file = token_file,
        .new_line = true,
    };
    Token *first_token = malloc(sizeof *first_token);
    first_token->type = START;
    lexer.queue = g_slist_prepend(lexer.queue, first_token);
    Node *node = parse(&lexer);
    bool just_atom = false;
    print_node(node, &just_atom);
    putchar('\n');
    Env top_env[1];
    env_init(top_env, NULL);
    eval(node, top_env);
    env_destroy(top_env);
    return !node;
}
