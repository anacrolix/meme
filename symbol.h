#pragma once

#include "node.h"
#include "type.h"

extern Type const symbol_type;

struct Symbol {
    Node;
    char *s;
};

Symbol *symbol_new(char const *s);
char const *symbol_str(Symbol const *);
Symbol *symbol_check(Node *);
