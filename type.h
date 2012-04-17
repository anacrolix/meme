#pragma once

#include <stdlib.h>

typedef struct Node Node;
typedef struct Env Env;
typedef struct Printer Printer;

typedef Node *(*CallFunc)(Node *, Node *[], int count, Env *);
typedef Node *(*EvalFunc)(Node *, Env *);

typedef struct Type {
    char const *name;
    EvalFunc eval;
    int (*truth)(Node const *);
    void (*dealloc)(Node *);
    void (*print)(Node const *, Printer *);
    CallFunc call;
} Type;
