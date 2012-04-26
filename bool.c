#include "bool.h"
#include "meme.h"

typedef struct {
    Node ;
    bool b;
} Bool;

static void bool_print(Node *n, Printer *p) {
    if (p->just_atom) fputc(' ', p->file);
    fputs(((Bool *)n)->b ? "#t" : "#f", p->file);
    p->just_atom = true;
}

Type const bool_type = {
    .name = "bool",
    .print = bool_print,
};

Bool true_node_storage = {
    {
        .refs = 1,
        .type = &bool_type,
    },
    .b = true,
};

Node *true_node = &true_node_storage;

Bool false_node_storage = {
    {
        .refs = 1,
        .type = &bool_type,
    },
    .b = false,
};

Node *false_node = &false_node_storage;
