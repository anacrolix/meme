#pragma once

#include "type.h"
#include <stdbool.h>

typedef struct Node Node;
typedef struct Env Env;
typedef struct Type Type;
typedef struct Printer Printer;

typedef struct Node {
    int refs;
    Type const *type;
} Node;

void node_init(Node *, Type const *);
void node_print(Node *, Printer *p);
Node *node_apply(Node *, Pair *, Env *);
void node_ref(Node *);
void node_unref(Node *);
NodeTruth node_truth(Node *);
Node *node_eval(Node *, Env *);
bool node_special(Node *);
NodeCmp node_compare(Node *, Node *);

