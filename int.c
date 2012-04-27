#include "int.h"
#include "meme.h"
#include <assert.h>

Int *int_check(Node *node) {
    return node->type == &int_type ? (Int *)node : NULL;
}

Int *int_new(long long ll) {
    Int *ret = malloc(sizeof *ret);
    node_init(ret, &int_type);
    ret->ll = ll;
    return ret;
}

static void int_print(Node *n, Printer *p) {
    if (p->just_atom) fputc(' ', p->file);
    fprintf(p->file, "%lld", ((Int *)n)->ll);
    p->just_atom = true;
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

Type const int_type = {
    .name = "int",
    .print = int_print,
    .compare = int_compare,
};

