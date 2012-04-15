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
    LAMBDA,
    BOOLEAN,
} Form;

typedef struct Env {
    GHashTable *dict;
    struct Env *outer;
} Env;

typedef struct Node *(*NodeFunc)(struct Node *, struct Node **, int, Env *);

typedef struct Node {
    int refs;
    Form form;
    int line, col;
    union {
        bool b;
        long long i;
        double f;
        char *s;
        List list[1];
    };
    NodeFunc run;
} Node;

typedef struct {
    bool just_atom;
    FILE *file;
} Printer;

Node *node_new() {
    Node *ret = calloc(1, sizeof *ret);
    ret->refs = 1;
    return ret;
}

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
        free(lexer->token->value);
        lexer->token->value = read_atom(lexer);
        return true;
    }
    lexer->col++;
    return true;
}

void list_destroy(List *);

void node_ref(Node *n) {
    if (n->refs <= 0) abort();
    n->refs++;
}

void node_unref(Node *n) {
    if (n->refs <= 0) abort();
    if (n->refs > 1) {
        n->refs--;
        return;
    }
    switch (n->form) {
    case LIST:
    case LAMBDA:
        list_destroy(n->list);
        break;
    case IDENTIFIER:
    case STRING:
        free(n->s);
        break;
    default:
        break;
    }
}

void list_destroy(List *l) {
    for (int i = 0; i < l->len; i++) {
        node_unref(l->nodes[i]);
    }
    free(l->nodes);
    l->cap = 0;
    l->len = 0;
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
    node_ref(n);
}

Node *parse(Lexer *lexer);

Node *parse_list(Lexer *lexer) {
    Token *const t = lexer->token;
    Node *ret = node_new();
    ret->form = LIST;
    ret->line = t->line;
    ret->col = t->col;
    while (t->type != END) {
        Node *node = parse(lexer);
        if (!node) abort();
        list_append(ret->list, node);
    }
    next_token(lexer);
    return ret;
}

Node *parse_atom(Lexer *lexer) {
    Token *const tn = lexer->token;
    Node *node = node_new();
    node->line = tn->line;
    node->col = tn->col;
    if (tn->value[0] == '"') {
        node->form = STRING;
        node->s = strdup(tn->value + 1);
    } else if (strchr(tn->value, '.')) {
        node->form = DOUBLE;
        if (1 != sscanf(tn->value, "%lf", &node->f)) abort();
    } else if (isdigit(tn->value[0])) {
        node->form = INTEGER;
        if (1 != sscanf(tn->value, "%lld", &node->i)) abort();
    } else if (tn->value[0] == '#') {
        node->form = BOOLEAN;
        switch (tn->value[1]) {
        case 't':
            node->b = true;
            break;
        case 'f':
            node->b = false;
            break;
        default:
            node_unref(node);
            return NULL;
        }
        if (tn->value[2]) {
            node_unref(node);
            return NULL;
        }
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

int node_truth(Node const *n) {
    switch (n->form) {
    case DOUBLE:
        return !!n->f;
    case INTEGER:
        return !!n->i;
    case STRING:
        return !!*n->s;
    case LIST:
        return !!n->list->len;
    case LAMBDA:
        return !!n->run;
    case BOOLEAN:
        return !!n->b;
    default:
        fprintf(stderr, "truth value not defined for form %d\n", n->form);
        return -1;
    }
}

void print_atom(Node const *node, Printer *printer) {
    if (printer->just_atom) fputc(' ', printer->file);
    switch (node->form) {
    case IDENTIFIER:
        fputs(node->s, printer->file);
        break;
    case INTEGER:
        fprintf(printer->file, "%lld", node->i);
        break;
    case BOOLEAN:
        fputs(node->b ? "#t" : "#f", printer->file);
        break;
    default:
        fprintf(stderr, "can't print atom node form: %d\n", node->form);
        abort();
    }
    printer->just_atom = true;
}

void print_node(Node const *node, Printer *printer) {
    if (!node) {
        fputs("(null)", printer->file);
        printer->just_atom = true;
        return;
    }
    switch (node->form) {
    case LIST:
        fputc('(', printer->file);
        printer->just_atom = false;
        for (int i = 0; i < node->list->len; i++) {
            print_node(node->list->nodes[i], printer);
        }
        fputc(')', printer->file);
        printer->just_atom = false;
        break;
    default:
        print_atom(node, printer); 
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
    node_ref(value);
    return value;
}

void env_init(Env *env, Env *outer) {
    *env = (Env){
        .dict = g_hash_table_new_full(g_str_hash,
                                      g_str_equal,
                                      free,
                                      (GDestroyNotify)node_unref),
        .outer = outer,
    };
}

void env_destroy(Env *env) {
    g_hash_table_destroy(env->dict);
}

void env_insert(Env *env, char *key, Node *value) {
    node_ref(value);
    g_hash_table_insert(env->dict, strdup(key), value);
}

Node *eval(Node *node, Env *env) {
    switch (node->form) {
    case IDENTIFIER:
        {
            Node *ret = env_find(env, node->s);
            if (!ret) {
                fprintf(stderr, "line %d, col %d: symbol not found: %s\n",
                        node->line, node->col, node->s);
                return NULL;
            }
            return ret;
        }
    case LIST:
        {
            Node *proc = eval(*node->list->nodes, env);
            if (!proc) {
                fprintf(stderr, "error evaluating ");
                print_node(*node->list->nodes, &(Printer){.file=stderr});
                fputc('\n', stderr);
                return NULL;
            }
            Node *ret = proc->run(proc, node->list->nodes, node->list->len, env);
            node_unref(proc);
            return ret;
        }
    case BOOLEAN:
    case INTEGER:
        return node;
    default:
        fprintf(stderr, "can't eval form: %d\n", node->form);
        abort();
    }
}

Node *multiply(Node *proc, Node *args[], int count, Env *env) {
    if (count < 3) abort();
    Node *ret = node_new();
    ret->form = INTEGER;
    Node *op = eval(args[1], env);
    if (op->form != INTEGER) abort();
    ret->i = op->i;
    node_unref(op);
    for (int i = 2; i < count; i++) {
        op = eval(args[i], env);
        ret->i *= op->i;
        node_unref(op);
    }
    return ret;
}

Node *assign(Node *proc, Node *args[], int count, Env *env) {
    if (count != 3) {
        fprintf(stderr, "line %d, col %d: expected 2 arguments, %d were given\n",
                args[0]->line, args[1]->col, count - 1);
        return NULL;
    }
    Node *var = args[1];
    if (var->form != IDENTIFIER) abort();
    Node *value = eval(args[2], env);
    env_insert(env, var->s, value);
    return NULL;
}

Node *lambda_call(Node *proc, Node *args[], int count, Env *env) {
    if (proc->list->len != count) abort();
    Env sub_env[1];
    env_init(sub_env, env);
    for (size_t i = 0; i < count - 1; i++) {
        Node *v = eval(args[i + 1], env);
        if (!v) {
            env_destroy(sub_env);
            return NULL;
        }
        env_insert(sub_env, proc->list->nodes[i]->s, v);
    }
    Node *ret = eval(proc->list->nodes[proc->list->len-1], sub_env);
    env_destroy(sub_env);
    return ret;
}

Node *lambda(Node *proc, Node *args[], int count, Env *env) {
    Node *ret = node_new();
    ret->form = LAMBDA;
    ret->run = lambda_call;
    ret->list->nodes = malloc((count-1)*sizeof(Node *));
    ret->list->cap = ret->list->len = count - 1;
    memcpy(ret->list->nodes, args+1, sizeof *args * (count-1));
    return ret;
}

Node *if_form(Node *proc, Node *args[], int count, Env *env) {
    if (count != 4) {
        fprintf(stderr, "wrong argument count\n");
        return NULL;
    }
    Node *test = eval(args[1], env);
    if (!test) return NULL;
    int truth = node_truth(test);
    if (truth < 0) return NULL;
    if (truth) return eval(args[2], env);
    else return eval(args[3], env);
}

Node *subtract(Node *proc, Node *args[], int count, Env *env) {
    if (count != 3) return NULL;
    Node *left = eval(args[1], env);
    Node *right = eval(args[2], env);
    Node *ret = node_new();
    ret->form = INTEGER;
    ret->i = left->i - right->i;
    return ret;

}

Node *less_than(Node *proc, Node *args[], int count, Env *env) {
    if (count != 3) {
        fprintf(stderr, "wrong number of args\n");
        return NULL;
    }
    Node *left = eval(args[1], env);
    if (!left) return NULL;
    Node *right = eval(args[2], env);
    if (!right) return NULL;
    if (left->form != INTEGER || right->form != INTEGER) {
        fprintf(stderr, "unsupported types\n");
        return NULL;
    }
    Node *ret = malloc(sizeof *ret);
    *ret = (Node){
        .line = proc->line,
        .col = proc->col,
        .form = BOOLEAN,
        .b = left->i < right->i,
    };
    return ret;
}

Node *new_callable_node(NodeFunc run) {
    Node *ret = node_new();
    ret->form = LAMBDA;
    ret->run = run;
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
    env_insert(top_env, "*", new_callable_node(multiply));
    env_insert(top_env, "=", new_callable_node(assign));
    env_insert(top_env, "^", new_callable_node(lambda));
    env_insert(top_env, "if", new_callable_node(if_form));
    env_insert(top_env, "<", new_callable_node(less_than));
    env_insert(top_env, "-", new_callable_node(subtract));
    next_token(&lexer);
    Node *result = NULL;
    while (lexer.token->type != INVALID) {
        if (result) node_unref(result);
        Node *node = parse(&lexer);
        print_node(node, &(Printer){.file=stderr});
        putchar('\n');
        result = eval(node, top_env);
        node_unref(node);
        print_node(result, &(Printer){.file=stderr});
        putchar('\n');
    }
    env_destroy(top_env);
    if (fclose(token_file)) abort();
    return !result;
}
