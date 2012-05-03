#include "node.h"
#include "meme.h"
#include <assert.h>
#include <stdlib.h>

void node_print(Node *n, Printer *p) {
    assert(n->refs > 0);
    n->type->print(n, p);
}

void node_init(Node *n, Type const *t) {
    *n = (Node){
        .refs = 1,
        .type = t,
    };
    link_node(n);
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
    unlink_node(n);
    free_node(n);
}

NodeTruth node_truth(Node *node) {
    if (node == false_node) return NODE_TRUTH_FALSE;
    else if (node == void_node) return NODE_TRUTH_ERR;
    else return NODE_TRUTH_TRUE;
}

Node *node_apply(Node *proc, Node *const args[], int count, Env *env) {
    if (!proc->type->apply) {
        fprintf(stderr, "not applicable: ");
        node_print_file(proc, stderr);
        fputc('\n', stderr);
        return NULL;
    }
    return proc->type->apply(proc, args, count, env);
}

bool node_special(Node *node) {
    return node->type->special;
}

Node *node_eval(Node *node, Env *env) {
#if TRACE
    fprintf(stderr, "evaluating: ");
    node_print_file(node, stderr);
    fputc('\n', stderr);
#endif
    Node *ret = node->type->eval(node, env);
    if (!ret) {
        fputs("error evaluating: ", stderr);
        node_print_file(node, stderr);
        fputc('\n', stderr);
        return NULL;
    }
#if TRACE
    node_print_file(node, stderr);
    if (ret != void_node) {
        fprintf(stderr, " ===> ");
        node_print_file(ret, stderr);
    }
    fputc('\n', stderr);
#endif
    return ret;
}

void free_node(Node *node) {
#ifdef SLICE_NODES
    node->type->free(node);
#else
    free(node);
#endif
}
