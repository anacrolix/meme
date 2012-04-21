#pragma once

#include "type.h"

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
Node *node_apply(Node *, Pair *args, Env *);
void node_ref(Node *);
void node_unref(Node *);
int node_truth(Node *);
Node *node_eval(Node *, Env *);

