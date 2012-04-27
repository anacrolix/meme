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
    assert(n->refs > 0);
    if (--n->refs > 0) return;
    if (n->type->traverse) {
        n->type->traverse(n, node_free_visit, NULL);
    }
    if (n->type->dealloc) n->type->dealloc(n);
    if (!g_hash_table_remove(all_nodes, n)) assert(false);
    free(n);
}

NodeTruth node_truth(Node *node) {
    if (node == false_node) return NODE_TRUTH_FALSE;
    else if (node == void_node) return NODE_TRUTH_ERR;
    else return NODE_TRUTH_TRUE;
}

Node *node_apply(Node *proc, Pair *args, Env *env) {
    if (!proc->type->apply) {
        fprintf(stderr, "not applicable: ");
        node_print_file(proc, stderr);
        fputc('\n', stderr);
        return NULL;
    }
    return proc->type->apply(proc, args, env);
}

bool node_special(Node *node) {
    return node->type->special;
}

