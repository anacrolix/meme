#pragma once

#include "node.h"
#include <stdlib.h>

struct Pair {
    Node;
    Node *addr;
    Pair *dec;
};

extern Type const pair_type;
extern Pair *const nil_node;

Pair *pair_new();
Pair *pair_check(Node *);
Pair *pair_dec(Pair *);
Node *pair_addr(Pair *);

size_t list_length(Pair *);

