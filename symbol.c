#include "symbol.h"
#include "meme.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

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
    print_atom(p, "%s", ((Symbol *)n)->s);
}

static void symbol_dealloc(Node *node) {
    Symbol *symbol = (Symbol *)node;
    free(symbol->s);
}

static NodeCmp symbol_compare(Node *_left, Node *_right) {
    assert(_left->type == &symbol_type);
    Symbol *left = (Symbol *)_left;
    if (_right->type != &symbol_type) return NODE_CMP_NE;
    Symbol *right = (Symbol *)_right;
    int cmp = strcmp(left->s, right->s);
    if (cmp < 0) return NODE_CMP_LT;
    else if (cmp > 0) return NODE_CMP_GT;
    else return NODE_CMP_EQ;
}

Type const symbol_type = {
    .name = "symbol",
    .print = symbol_print,
    .dealloc = symbol_dealloc,
    .eval = symbol_eval,
    .compare = symbol_compare,
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
