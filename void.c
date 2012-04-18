#include "void.h"
#include "meme.h"

typedef Node Void;

Type const void_type = {
    .name = "void",
};

Void void_node_storage = {
    .refs = 1,
    .type = &void_type,
};

Node *void_node = &void_node_storage;

Void *void_check(Node *node) {
    if (node == void_node) return node;
    else return NULL;
}

