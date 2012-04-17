#include "eval.h"
#include "meme.h"

Node *eval(Node *node, Env *env) {
    Node *ret = node;
    node_ref(ret);
    while (ret->type->eval) {
#if 0
        fprintf(stderr, "evaluating: ");
        node_print(ret, &(Printer){.file=stderr});
        fputc('\n', stderr);
#endif
        node = ret->type->eval(ret, env);
        if (!node) {
            fputs("error evaluating: ", stderr);
            node_print(ret, &(Printer){.file=stderr});
            fputc('\n', stderr);
            node_unref(ret);
            return NULL;
        }
        node_unref(ret);
        ret = node;
#if 0
        fprintf(stderr, "result: ");
        node_print(ret, &(Printer){.file=stderr});
        fputc('\n', stderr);
#endif
    }
    return ret;
}
