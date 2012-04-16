#include "int.h"

Int *int_new() {
    Int *ret = malloc(sizeof *ret);
    node_init(ret, &int_type);
    return ret;
}

static void int_print(Node const *n, Printer *p) {
    if (p->just_atom) fputc(' ', p->file);
    fprintf(p->file, "%lld", ((Int *)n)->ll);
    p->just_atom = true;
}

Type const int_type = {
    .print = int_print,
};

