#include "int.h"
#include "meme.h"

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

Type const int_type = {
    .name = "int",
    .print = int_print,
};

