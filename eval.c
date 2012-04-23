#include "eval.h"
#include "meme.h"

Node *eval(Node *node, Env *env) {
    Node *ret = node;
    node_ref(ret);
    if (ret->type->eval) {
        node = ret->type->eval(ret, env);
        if (!node) {
            fputs("error evaluating: ", stderr);
            node_print(ret, &(Printer){.file=stderr});
            fputc('\n', stderr);
            node_unref(ret);
            return NULL;
        }
#if 1
        node_print(ret, &(Printer){.file=stderr});
        fprintf(stderr, " ==> ");
        if (node != void_node) {
            node_print(node, &(Printer){.file=stderr});
        }
        fputc('\n', stderr);
#endif
        node_unref(ret);
        ret = node;
    }
    return ret;
}
