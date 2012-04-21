#pragma once

#include "node.h"

typedef struct Env {
    Node;
    void *table; // stores the frame's variables
    Env *outer;
} Env;

Env *env_new(Env *);
void env_set(Env *, char const *, Node *);
Node *env_find(Env *, char const *);
