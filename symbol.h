#pragma once

#include "node.h"
#include "type.h"

extern Type const symbol_type;

struct Symbol {
    Node node[1];
    char *s;
};

Symbol *symbol_new(char const *s);
char const *symbol_str(Symbol *);
Symbol *symbol_check(Node *);
