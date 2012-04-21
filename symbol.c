#include "symbol.h"
#include "meme.h"
#include <assert.h>
#include <string.h>

typedef struct Symbol {
    Node;
    char *s;
} Symbol;

char const *symbol_str(Symbol *symbol) {
    return symbol->s;
}

Node *symbol_eval(Node *node, Env *env) {
    assert(node->type == &symbol_type);
    Symbol *symbol = (Symbol *)node;
    return env_find(env, symbol->s);
}

char const *symbol_get_name(Symbol *v) {
    return v->s;
}

static void symbol_print(Node *n, Printer *p) {
    if (p->just_atom) fputc(' ', p->file);
    fputs(((Symbol *)n)->s, p->file);
    p->just_atom = true;
}

static void symbol_dealloc(Node *node) {
    Symbol *symbol = (Symbol *)node;
    free(symbol->s);
}

Type const symbol_type = {
    .name = "symbol",
    .print = symbol_print,
    .dealloc = symbol_dealloc,
    .eval = symbol_eval
};

Symbol *symbol_new(char const *s) {
    Symbol *ret = malloc(sizeof *ret);
    node_init(ret, &symbol_type);
    ret->s = strdup(s);
    return ret;
}

Symbol *symbol_check(Node *node) {
    if (node->type != &symbol_type) return NULL;
    return (Symbol *)node;
}
