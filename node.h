#pragma once

#include "type.h"

#include <stdbool.h>
#include <stdio.h>

typedef struct Node Node;
typedef struct Env Env;

typedef struct Printer {
    bool just_atom;
    FILE *file;
} Printer;

typedef struct Node {
    int refs;
    Type const *type;
} Node;

static inline void node_init(Node *n, Type const *t) {
    *n = (Node){
        .refs = 1,
        .type = t,
    };
}

static inline void print_node(Node const *n, Printer *p) {
    n->type->print(n, p);
}

static inline Node *call_node(Node *node, Node *args[], int nargs, Env *env) {
    return node->type->call(node, args, nargs, env);
}

static inline void node_ref(Node *n) {
    if (n->refs <= 0) abort();
    n->refs++;
}

static inline void node_unref(Node *n) {
    if (n->refs <= 0) abort();
    if (n->refs > 1) {
        n->refs--;
        return;
    } else n->type->dealloc(n);
}

static inline int node_truth(Node const *n) {
    if (!n->type->truth) return 1;
    return n->type->truth(n);
}

inline Node *node_eval(Node *node, Env *env) {
    return node->type->eval(node, env);
}

