#pragma once

#include "node.h"
#include <stdbool.h>

struct Env {
    Node;
    void *table; // stores the frame's variables
    Env *outer;
};

Env *env_new(Env *);
bool env_set(Env *, char const *, Node *);
__attribute__((warn_unused_result))
bool env_define(Env *, char const *, Node *);
Node *env_find(Env *, char const *);
bool env_is_defined(Env *, char const *);
bool env_undefine(Env *, char const *);
