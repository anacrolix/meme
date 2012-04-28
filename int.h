#pragma once

#include "types.h"
#include "node.h"

struct Int {
    Node node[1];
    long long ll;
};

extern Type const int_type;

Int *int_new(long long);
Int *int_check(Node *);
long long int_as_ll(Int *);
