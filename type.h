#pragma once

#include <stdlib.h>

typedef struct Node Node;
typedef struct Env Env;
typedef struct Printer Printer;
typedef struct Pair Pair;

typedef Node *(*ApplyFunc)(Node *, Pair *, Env *);
typedef Node *(*EvalFunc)(Node *, Env *);

typedef struct Type {
    char const *name;
    EvalFunc eval;
    int (*truth)(Node *);
    void (*dealloc)(Node *);
    void (*print)(Node *, Printer *);
    ApplyFunc apply;
} Type;
