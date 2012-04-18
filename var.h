#pragma once

#include "type.h"

extern Type const var_type;

typedef struct Var Var;

Var *var_new(char const *s);
char const *var_get_name(Var *);
Var *var_check(Node *);
