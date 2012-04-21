#include "eval.h"
#include "meme.h"

Node *eval(Node *node, Env *env) {
    Node *ret = node;
    node_ref(ret);
    if (ret->type->eval) {
#if 1
        fprintf(stderr, "evaluating: ");
        node_print(ret, &(Printer){.file=stderr});
        fputc('\n', stderr);
#endif
        node = node_eval(ret, env);
        if (!node) {
            fputs("error evaluating: ", stderr);
            node_print(ret, &(Printer){.file=stderr});
            fputc('\n', stderr);
            node_unref(ret);
            return NULL;
        }
        node_unref(ret);
        ret = node;
#if 1
        fprintf(stderr, "result: ");
        if (ret != void_node) {
            node_print(ret, &(Printer){.file=stderr});
        }
        fputc('\n', stderr);
#endif
    }
    return ret;
}
