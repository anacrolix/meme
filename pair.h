#pragma once

#include "node.h"

struct Pair {
    Node node[1];
    Node *addr;
    Pair *dec;
};

extern Type const pair_type;
extern Pair *const nil_node;

Pair *pair_new();
Pair *pair_check(Node *);
Pair *pair_dec(Pair *);
Node *pair_addr(Pair *);
