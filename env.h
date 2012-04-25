#pragma once

#include "node.h"
#include <stdbool.h>

typedef struct Env {
    Node;
    void *table; // stores the frame's variables
    Env *outer;
} Env;

Env *env_new(Env *);
bool env_set(Env *, char const *, Node *);
bool env_define(Env *, char const *, Node *);
Node *env_find(Env *, char const *);
