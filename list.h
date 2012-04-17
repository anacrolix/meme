#pragma once

#include "node.h"

typedef struct {
    Node;
    long len;
    Node *data[0];
} List;

extern Type const list_type;

List *list_new();
List *list_append(List *, Node *);
