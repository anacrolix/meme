#pragma once

#include "node.h"

typedef struct {
    Node;
    long long ll;
} Int;

extern Type const int_type;

Int *int_new(long long);
