#pragma once

#include "type.h"

extern Type const symbol_type;

typedef struct Symbol Symbol;

Symbol *symbol_new(char const *s);
char const *symbol_str(Symbol *);
Symbol *symbol_check(Node *);
