#pragma once
#include "type.h"
typedef struct Var Var;
extern Type const var_type;
Var *var_new(char const *s);
char const *var_get_name(Var *);
