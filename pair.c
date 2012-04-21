#include "pair.h"
#include "meme.h"
#include <assert.h>

Pair *pair_check(Node *node) {
    return node->type == &pair_type ? (Pair *)node : NULL;
}

static void pair_print(Node *node, Printer *p) {
    assert(node->type == &pair_type);
    Pair const *pair = (Pair const *)node;
    fputc('(', p->file);
    p->just_atom = false;
    for (; pair->addr; pair = pair->dec) {
        node_print(pair->addr, p);
    }
    fputc(')', p->file);
}

static Node *pair_eval(Node *node, Env *env) {
    Pair *pair = pair_check(node);
    Node *proc = node_eval(pair->addr, env);
    if (!proc) return NULL;
    Node *ret = node_apply(proc, pair->dec, env);
    node_unref(proc);
    return ret;
}

static void pair_dealloc(Node *node) {
    Pair *pair = pair_check(node);
    node_unref(pair->addr);
    node_unref(pair->dec);
}

Type const pair_type = {
    .name = "pair",
    .dealloc = pair_dealloc,
    .print = pair_print,
    .eval = pair_eval,
};

static Pair nil_node_storage = {
    {
        .refs = 1,
        .type = &pair_type,
    },
    .addr = NULL,
    .dec = NULL,
};

Pair *const nil_node = &nil_node_storage;

Pair *pair_new() {
    Pair *pair = calloc(1, sizeof *pair);
    node_init(pair, &pair_type);
    return pair;
}
