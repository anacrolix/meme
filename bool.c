#include "bool.h"


typedef struct {
    Node;
    bool b;
} Bool;

Type bool_type;

Bool *bool_node_new(bool b) {
    Bool *ret = malloc(sizeof *ret);
    node_init(ret, &bool_type);
    ret->b = b;
    return ret;
}

Node *true_node;
Node *false_node;
