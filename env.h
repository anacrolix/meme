#pragma once

#include "node.h"
#include "symtab.h"
#include <stdbool.h>

struct Env {
    Node;
    SymTab *symtab;
    Env *outer;
};

Env *env_new(Env *, SymTab *(*)());
bool env_set(Env *, Symbol *, Node *);
__attribute__((warn_unused_result))
bool env_define(Env *, Symbol *, Node *);
Node *env_find(Env *, Symbol *);
bool env_is_defined(Env *, Symbol *);
bool env_undefine(Env *, Symbol *);
