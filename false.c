#include "false.h"
#include "type.h"
#include "node.h"
#include "printer.h"
#include <assert.h>

static Type const false_type;

static void false_print(Node *node, Printer *p) {
    assert(node->type == &false_type);
    print_atom(p, "#f");
}

static Type const false_type = {
    .name = "false",
    .print = false_print,
};

Node false_node_storage = {
    .refs = 1,
    .type = &false_type,
};

Node *false_node = &false_node_storage;

