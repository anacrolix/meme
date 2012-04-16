#pragma once

#include "node.h"

typedef struct {
    Node;
    long len;
    Node *data[0];
} List;

List *list_new();
List *list_append(List *, Node *);
