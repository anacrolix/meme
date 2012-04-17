#include "eval.h"
#include "meme.h"

Node *eval(Node *node, Env *env) {
    node_ref(node);
    Node *ret = node;
    while (ret->type->eval) {
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
    }
    return ret;
}
