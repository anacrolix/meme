#include "node.h"
#include "meme.h"
#include <assert.h>

void node_print(Node *n, Printer *p) {
    assert(n->refs > 0);
    n->type->print(n, p);
}

void node_init(Node *n, Type const *t) {
    *n = (Node){
        .refs = 1,
        .type = t,
    };
}

void node_ref(Node *n) {
    assert(n->refs > 0);
    n->refs++;
}

void node_unref(Node *n) {
    assert(n->refs > 0);
    if (n->refs > 1) {
        n->refs--;
        return;
    }
    if (n->type->dealloc) n->type->dealloc(n);
    free(n);
}

int node_truth(Node *node) {
    if (!node->type->truth) return 1;
    return node->type->truth(node);
}

Node *node_eval(Node *node, Env *env) {
    return node->type->eval(node, env);
}

Node *node_apply(Node *proc, Pair *args, Env *env) {
    return proc->type->apply(proc, args, env);
}
