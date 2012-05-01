#pragma once

#include "types.h"
#include "type.h"

struct Node {
    Node *prev, *next;
    int refs;
    Type const *type;
};

void node_init(Node *, Type const *);
void node_print(Node *, Printer *p);
Node *node_apply(Node *, Node *const[], int, Env *);
void node_ref(Node *);
void node_unref(Node *);
NodeTruth node_truth(Node *);
Node *node_eval(Node *, Env *);
bool node_special(Node *);
NodeCmp node_compare(Node *, Node *);
void free_node(Node *);

