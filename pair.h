#pragma once

#include "node.h"

typedef struct Pair {
    Node;
    Node *addr;
    Pair *dec;
} Pair;

extern Type const pair_type;
extern Pair *const nil_node;

Pair *pair_new();
Pair *pair_check(Node *);
