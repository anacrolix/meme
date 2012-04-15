#include <glib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    INVALID,
    ATOM,
    START,
    END,
} TokenType;

typedef struct {
    int line, col;
    TokenType type;
    char *value;
} Token;

typedef struct {
    FILE *file;
    int line, col;
    Token token[1];
} Lexer;

char *read_atom(Lexer *lexer) {
    // TODO remove atom length limit
    char lval[0x100];
    int len = 0;
    for (; len < sizeof lval - 1; len++) {
        int c = fgetc(lexer->file);
        if (c < 0) break;
        switch (c) {
        case ' ':
        case '\t':
        case '(':
        case ')':
        case '\n':
            if (c != ungetc(c, lexer->file)) abort();
            goto end_loop;
        }
        lval[len] = c;
        lexer->col++;
    }
end_loop:
    if (!len) abort();
    lval[len] = '\0';
    return strdup(lval);
}

void discard_whitespace(Lexer *lexer) {
    for (;;) {
        int c = fgetc(lexer->file);
        if (c < 0) return;
        switch (c) {
        case ' ':
        case '\t':
            lexer->col++;
            break;
        case '\n':
            lexer->line++;
            lexer->col = 1;
            break;
        default:
            if (c != ungetc(c, lexer->file)) abort();
            return;
        }
    }
}

void init_token(Lexer *lexer, TokenType type) {
    Token *t = lexer->token;
    t->type = type;
    t->line = lexer->line;
    t->col = lexer->col;
}

bool next_token(Lexer *lexer) {
    discard_whitespace(lexer);
    int c = fgetc(lexer->file);
    if (c < 0) {
        init_token(lexer, INVALID);
        return false;
    }
    switch (c) {
    case '(':
        init_token(lexer, START);
        break;
    case ')':
        init_token(lexer, END);
        break;
    default:
        if (c != ungetc(c, lexer->file)) abort();
        init_token(lexer, ATOM);
        lexer->token->value = read_atom(lexer);
        return true;
    }
    lexer->col++;
    return true;
}

typedef enum {
    VARREF,
    DOUBLE,
    INTEGER,
    STRING,
    LIST,
    DEFINE,
    LAMBDA,
    BEGIN,
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
            struct Node *expr;
        } define;
        struct {
            struct Node *vars;
            struct Node *expr;
        } lambda;
    };
} Node;

Node *parse(Lexer *lexer);

Node *parse_define(Lexer *lexer) {
    Node *ret = malloc(sizeof *ret);
    Token *t = lexer->token;
    ret->form = DEFINE;
    if (t->type != ATOM) abort();
    ret->define.var = t->value;
    t->value = NULL;
    next_token(lexer);
    ret->define.expr = parse(lexer);
    if (t->type != END) abort();
    next_token(lexer);
    return ret;
}

Node *parse_lambda(Lexer *lexer) {
    Node *ret = malloc(sizeof *ret);
    ret->form = LAMBDA;
    ret->lambda.vars = parse(lexer);
    ret->lambda.expr = parse(lexer);
    if (lexer->token->type != END) abort();
    next_token(lexer);
    return ret;
}

Node *parse_list(Lexer *lexer) {
    Token *const t = lexer->token;
    if (t->type == ATOM) {
        if (!strcmp(t->value, "define")) {
            next_token(lexer);
            return parse_define(lexer);
        } else if (!strcmp(t->value, "lambda")) {
            if (!next_token(lexer)) abort();
            return parse_lambda(lexer);
        }
    }
    Node *ret = malloc(sizeof *ret);
    if (t->type == ATOM && !strcmp(t->value, "begin")) {
        ret->form = BEGIN;
        if (!next_token(lexer)) abort();
    } else ret->form = LIST;
    ret->list = NULL;
    while (t->type != END) {
        ret->list = g_slist_prepend(ret->list, parse(lexer));
    }
    ret->list = g_slist_reverse(ret->list);
    next_token(lexer);
    return ret;
}

Node *parse_atom(Lexer *lexer) {
    Token const *tn = lexer->token;
    Node *node = malloc(sizeof *node);
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
    next_token(lexer);
    return node;
}

Node *parse(Lexer *lexer) {
    switch (lexer->token->type) {
    case ATOM:
        return parse_atom(lexer);
    case START:
        if (!next_token(lexer)) abort();
        return parse_list(lexer);
    default:
        abort();
    }
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
    case BEGIN:
        fputs("(begin", stdout);
        *just_atom = true;
        g_slist_foreach(node->list, (GFunc)print_node, just_atom);
        putchar(')');
        *just_atom = false;
        break;
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
        print_node(node->define.expr, just_atom);
        putchar(')');
        break;
    case LAMBDA:
        fputs("(lambda ", stdout);
        *just_atom = false;
        print_node(node->lambda.vars, just_atom);
        print_node(node->lambda.expr, just_atom);
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
            if (proc->form != LAMBDA) abort();
            GSList *args = exps->next;
            GSList *arg_names = proc->lambda.vars->list;
            Env sub_env[1];
            env_init(sub_env, env);
            bind_args(sub_env->dict, arg_names, args);
            Node *ret = eval(proc->lambda.expr, sub_env);
            env_destroy(sub_env);
            return ret;
        }
    case DEFINE:
        {
            Node *value = eval(node->define.expr, env);
            g_hash_table_insert(env->dict, node->define.var, value);
            return NULL;
        }
    case LAMBDA:
        return node;
    case BEGIN:
        {
            Node *ret = NULL;
            for (GSList *list = node->list; list; list = list->next) {
                ret = eval(list->data, env);
            }
            return ret;
        }
    case INTEGER:
        return node;
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
        .line = 1,
        .col = 1,
    };
    next_token(&lexer);
    Node *node = parse(&lexer);
    bool just_atom = false;
    print_node(node, &just_atom);
    putchar('\n');
    Env top_env[1];
    env_init(top_env, NULL);
    Node *result = eval(node, top_env);
    env_destroy(top_env);
    just_atom = false;
    print_node(result, &just_atom);
    return !node;
}
