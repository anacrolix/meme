#pragma once

#include "types.h"
#include "type.h"

struct Node {
    int refs;
    Type const *type;
};

void node_init(Node *, Type const *);
void node_print(Node *, Printer *p);
Node *node_apply(Node *, Pair *, Env *);
void node_ref(Node *);
void node_unref(Node *);
NodeTruth node_truth(Node *);
Node *node_eval(Node *, Env *);
bool node_special(Node *);
NodeCmp node_compare(Node *, Node *);

