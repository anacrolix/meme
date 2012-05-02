#include "true.h"
#include "meme.h"
#include "node.h"
#include "printer.h"
#include <assert.h>

static Type const true_type;

static void true_print(Node *node, Printer *p) {
    assert(node->type == &true_type);
    print_atom(p, "#t");
}

static Type const true_type = {
    .name = "true",
    .print = true_print,
    .eval = node_eval_self,
};

Node true_node_storage = {
    .refs = 1,
    .type = &true_type,
};

Node *true_node = &true_node_storage;

