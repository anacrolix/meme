#pragma once

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
    NODE_CMP_NOT_IMPLEMENTED,
} NodeCmp;

typedef Node *(*ApplyFunc)(Node *, Pair *, Node *, Env *);
typedef Node *(*EvalFunc)(Node *, Env *);
typedef void (*VisitProc)(Node *, void *);
typedef void (*TraverseProc)(Node *, VisitProc, void *);
typedef NodeCmp (*CompareFunc)(Node *, Node *);

typedef struct Type {
    char const *name;
    EvalFunc eval;
    int (*truth)(Node *);
    void (*dealloc)(Node *);
    void (*print)(Node *, Printer *);
    ApplyFunc apply;
    TraverseProc traverse;
    CompareFunc compare;
} Type;

