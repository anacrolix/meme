#include "int.h"
#include "node.h"
#include "printer.h"
#include "symbol.h"
#include "glib.h"
#include <assert.h>
#include <stdlib.h>

Int *int_check(Node *node) {
    return node->type == &int_type ? (Int *)node : NULL;
}

Int *int_new(long long ll) {
#ifdef SLICE_NODES
    Int *ret = g_slice_new(Int);
#else
    Int *ret = malloc(sizeof *ret);
#endif
    node_init(ret, &int_type);
    ret->ll = ll;
    return ret;
}

static void int_print(Node *n, Printer *p) {
    print_atom(p, "%lld", ((Int *)n)->ll);
}

static NodeCmp int_compare(Node *_left, Node *_right) {
    assert(_left->type == &int_type);
    Int *left = (Int *)_left;
    if (_right->type != &int_type) return NODE_CMP_NOIMPL;
    Int *right = (Int *)_right;
    if (left->ll < right->ll) return NODE_CMP_LT;
    else if (left->ll > right->ll) return NODE_CMP_GT;
    else return NODE_CMP_EQ;
}

static Node *int_eval(Node *node, Env *env) {
    node_ref(node);
    return node;
}

#ifdef SLICE_NODES
static void int_free(Node *node) {
    g_slice_free(Int, (Int *)node);
}
#endif

Type const int_type = {
    .name = "int",
    .print = int_print,
    .compare = int_compare,
    .eval = int_eval,
#ifdef SLICE_NODES
    .free = int_free,
#endif
};

long long int_as_ll(Int *i) {
    return i->ll;
}
