#include "node.h"
#include <assert.h>

void node_print(Node const *n, Printer *p) {
    assert(n->refs > 0);
    n->type->print(n, p);
}

void node_init(Node *n, Type const *t) {
    *n = (Node){
        .refs = 1,
        .type = t,
    };
}

Node *call_node(Node *node, Node *args[], int nargs, Env *env) {
    return node->type->call(node, args, nargs, env);
}

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
    if (n->type->dealloc) n->type->dealloc(n);
    free(n);
}

int node_truth(Node const *n) {
    if (!n->type->truth) return 1;
    return n->type->truth(n);
}

Node *node_eval(Node *node, Env *env) {
    return node->type->eval(node, env);
}

