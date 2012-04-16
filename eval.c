#include "eval.h"
#include "meme.h"

Node *eval(Node *node, Env *env) {
    Node *ret = node;
    while (ret->type->eval) {
        node = ret->type->eval(ret, env);
        node_unref(ret);
        ret = node;
    }
    return ret;
}
