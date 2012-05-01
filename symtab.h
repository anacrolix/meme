#pragma once

#include "type.h"

typedef struct {
    bool (*lookup)(void *, Symbol *, Node **);
    bool (*insert)(void *, Symbol *, Node *);
    bool (*replace)(void *, Symbol *, Node *);
    bool (*reserve)(void *, Symbol *);
    bool (*contains)(void *, Symbol *);
    bool (*remove)(void *, Symbol *);
    void (*free)(void *);
    void (*traverse)(void *, VisitProc, void *);
} SymTabOps;

typedef struct {
    SymTabOps const *ops;
} SymTab;

