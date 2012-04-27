#pragma once

#include <stdbool.h>

typedef struct Node Node;
typedef struct Env Env;
typedef struct Printer Printer;
typedef struct Pair Pair;

typedef enum {
    NODE_CMP_ERR,
    NODE_CMP_LT,
    NODE_CMP_GT,
    NODE_CMP_EQ,
    NODE_CMP_NE,
    NODE_CMP_NOIMPL,
} NodeCmp;
typedef NodeCmp (*CompareFunc)(Node *, Node *);

typedef enum {
    NODE_TRUTH_ERR = -1,
    NODE_TRUTH_FALSE,
    NODE_TRUTH_TRUE,
} NodeTruth;

typedef Node *(*ApplyFunc)(Node *, Pair *, Env *);
typedef Node *(*EvalFunc)(Node *, Env *);
typedef void (*VisitProc)(Node *, void *);
typedef void (*TraverseProc)(Node *, VisitProc, void *);

typedef struct Type {
    char const *name;
    EvalFunc eval;
    TraverseProc traverse;
    void (*dealloc)(Node *);
    ApplyFunc apply;
    void (*print)(Node *, Printer *);
    CompareFunc compare;
    bool special;
} Type;

