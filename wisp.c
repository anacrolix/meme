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

typedef struct {
    struct Node **nodes;
    long len;
    long cap;
} List;

typedef enum {
    IDENTIFIER,
    DOUBLE,
    INTEGER,
    STRING,
    LIST,
    BEGIN,
    LAMBDA,
} Form;

typedef struct Env {
    GHashTable *dict;
    struct Env *outer;
} Env;

typedef struct Node *(*NodeFunc)(struct Node *, struct Node **, int, Env *);

typedef struct Node {
    Form form;
    union {
        long long i;
        double f;
        char *s;
        List list[1];
    };
    NodeFunc run;
} Node;

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

void list_init(List *l) {
    memset(l, 0, sizeof *l);
}

void list_append(List *l, Node *n) {
    if (l->len == l->cap) {
        if (l->cap == 0) l->cap = 5;
        else l->cap *= 2;
        l->nodes = realloc(l->nodes, l->cap * sizeof *l->nodes);
    } else if (l->len > l->cap) abort();
    l->nodes[l->len++] = n;
}

Node *parse(Lexer *lexer);

Node *parse_list(Lexer *lexer) {
    Token *const t = lexer->token;
    Node *ret = malloc(sizeof *ret);
    memset(ret, 0, sizeof *ret);
    ret->form = LIST;
    while (t->type != END) {
        list_append(ret->list, parse(lexer));
    }
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
        node->form = IDENTIFIER;
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
    case IDENTIFIER:
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
    if (!node) {
        fputs("(null)", stdout);
        *just_atom = true;
        return;
    }
    switch (node->form) {
    case LIST:
        putchar('(');
        *just_atom = false;
        for (int i = 0; i < node->list->len; i++) {
            print_node(node->list->nodes[i], just_atom);
        }
        putchar(')');
        *just_atom = false;
        break;
    default:
        print_atom(node, just_atom); 
    }
}

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

void env_replace(Env *env, char *key, Node *value) {
    g_hash_table_replace(env->dict, key, value);
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
    case IDENTIFIER:
        return env_find(env, node->s);
    case LIST:
        {
            Node *proc = eval(*node->list->nodes, env);
            return proc->run(proc, node->list->nodes, node->list->len, env);
        }
    case INTEGER:
        return node;
    default:
        fprintf(stderr, "can't eval form: %d\n", node->form);
        abort();
    }
}

Node *multiply(Node *proc, Node *args[], int count, Env *env) {
    if (count < 3) abort();
    Node *ret = malloc(sizeof *ret);
    ret->form = INTEGER;
    Node *op = eval(args[1], env);
    if (op->form != INTEGER) abort();
    ret->i = op->i;
    for (int i = 2; i < count; i++) {
        op = eval(args[i], env);
        ret->i *= op->i;
    }
    return ret;
}

Node *assign(Node *proc, Node *args[], int count, Env *env) {
    if (count != 3) abort();
    Node *var = args[1];
    if (var->form != IDENTIFIER) abort();
    Node *value = eval(args[2], env);
    env_replace(env, var->s, value);
    return NULL;
}

Node *lambda_call(Node *proc, Node *args[], int count, Env *env) {
    if (proc->list->len != count) abort();
    Env sub_env[1];
    env_init(sub_env, env);
    for (size_t i = 0; i < count - 1; i++) {
        env_replace(sub_env, proc->list->nodes[i]->s, eval(args[i+1], env));
    }
    return eval(proc->list->nodes[proc->list->len-1], sub_env);
}

Node *lambda(Node *proc, Node *args[], int count, Env *env) {
    Node *ret = malloc(sizeof *ret);
    ret->form = LAMBDA;
    ret->run = lambda_call;
    ret->list->nodes = malloc((count-1)*sizeof(Node *));
    ret->list->cap = ret->list->len = count - 1;
    memcpy(ret->list->nodes, args+1, sizeof *args * (count-1));
    return ret;
}

Node *new_callable_node(NodeFunc run) {
    Node *ret = malloc(sizeof *ret);
    *ret = (Node){
        .form = LAMBDA,
        .run = run,
    };
    return ret;
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
    Env top_env[1];
    env_init(top_env, NULL);
    env_replace(top_env, "*", new_callable_node(multiply));
    env_replace(top_env, "=", new_callable_node(assign));
    env_replace(top_env, "^", new_callable_node(lambda));
    next_token(&lexer);
    Node *result = NULL;
    while (lexer.token->type != INVALID) {
        Node *node = parse(&lexer);
        bool just_atom = false;
        print_node(node, &just_atom);
        putchar('\n');
        result = eval(node, top_env);
        just_atom = false;
        print_node(result, &just_atom);
        putchar('\n');
    }
    env_destroy(top_env);
    if (fclose(token_file)) abort();
    return !result;
}
