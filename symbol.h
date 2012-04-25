#pragma once

#include "node.h"
#include "type.h"

extern Type const symbol_type;

typedef struct Symbol {
    Node;
    char *s;
} Symbol;

Symbol *symbol_new(char const *s);
char const *symbol_str(Symbol *);
Symbol *symbol_check(Node *);
