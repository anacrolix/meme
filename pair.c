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
    Node *proc = eval(pair->addr, env);
    if (!proc) return NULL;
    Node *ret = node_apply(proc, pair->dec, NULL, env);
    node_unref(proc);
    return ret;
}

static void pair_traverse(Node *_pair, VisitProc visit, void *data) {
    Pair *pair = (Pair *)_pair;
    if (pair == nil_node) return;
    visit(pair->addr, data);
    visit(pair->dec, data);
}

Type const pair_type = {
    .name = "pair",
    .print = pair_print,
    .eval = pair_eval,
    .traverse = pair_traverse,
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
