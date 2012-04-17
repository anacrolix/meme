#pragma once

typedef struct Node Node;
typedef struct Env Env;
typedef struct Type Type;
typedef struct Printer Printer;

typedef struct Node {
    int refs;
    Type const *type;
} Node;

void node_init(Node *n, Type const *t);
void node_print(Node const *n, Printer *p);
Node *call_node(Node *node, Node *args[], int nargs, Env *env);
void node_ref(Node *n);
void node_unref(Node *n);
int node_truth(Node const *n);
Node *node_eval(Node *node, Env *env);

