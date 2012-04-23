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
    assert(!g_hash_table_contains(all_nodes, n));
    g_hash_table_add(all_nodes, n);
}

void node_ref(Node *n) {
    assert(n->refs > 0);
    n->refs++;
}

static void node_free_visit(Node *node, void *data) {
    node_unref(node);
}

void node_unref(Node *n) {
    if (n->refs > 1) {
        n->refs--;
        return;
    } else if (n->refs == 1) {
        if (n->type->traverse) {
            n->type->traverse(n, node_free_visit, NULL);
        }
        if (n->type->dealloc) n->type->dealloc(n);
        if (!g_hash_table_remove(all_nodes, n)) abort();
        free(n);
    } else abort();
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
