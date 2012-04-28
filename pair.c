#include "pair.h"
#include "meme.h"
#include <assert.h>
#include <stdlib.h>

Pair *pair_check(Node *node) {
    return node->type == &pair_type ? (Pair *)node : NULL;
}

static void pair_print(Node *node, Printer *p) {
    assert(node->type == &pair_type);
    Pair const *pair = (Pair const *)node;
    print_token(p, START);
    for (; pair->addr; pair = pair->dec) {
        node_print(pair->addr, p);
    }
    print_token(p, END);
}

static Node *pair_eval(Node *_pair, Env *env) {
    assert(_pair->type == &pair_type);
    Pair *pair = (Pair *)_pair;
    Node *proc = eval(pair->addr, env);
    if (!proc) return NULL;
    Pair *args;
    if (node_special(proc)) {
        args = pair->dec;
        node_ref(args);
    } else {
        args = eval_list(pair->dec, env);
        if (!args) {
            node_unref(proc);
            return NULL;
        }
    }
    Node *ret = node_apply(proc, args, env);
    node_unref(args);
    node_unref(proc);
    return ret;
}

static void pair_traverse(Node *_pair, VisitProc visit, void *data) {
    Pair *pair = (Pair *)_pair;
    if (pair == nil_node) return;
    visit(pair->addr, data);
    visit(pair->dec, data);
}

static NodeCmp pair_compare(Node *_left, Node *_right) {
    assert(_left->type == &pair_type);
    Pair *left = (Pair *)_left;
    Pair *right = pair_check(_right);
    if (!right) return NODE_CMP_NOIMPL;
    for (;; left = left->dec, right = right->dec) {
        if (is_null(left)) {
           if (is_null(right)) return NODE_CMP_EQ;
           else return NODE_CMP_LT;
        } else if (is_null(right)) return NODE_CMP_GT; 
        NodeCmp ret = node_compare(left->addr, right->addr);
        if (ret != NODE_CMP_EQ) return ret;
    }
}

Type const pair_type = {
    .name = "pair",
    .print = pair_print,
    .eval = pair_eval,
    .traverse = pair_traverse,
    .compare = pair_compare,
};

static Pair nil_node_storage = {
    {{
        .refs = 1,
        .type = &pair_type,
    }},
    .addr = NULL,
    .dec = NULL,
};

Pair *const nil_node = &nil_node_storage;

Pair *pair_new() {
    Pair *pair = calloc(1, sizeof *pair);
    node_init(pair, &pair_type);
    return pair;
}

Pair *pair_dec(Pair *pair) {
    return pair->dec;
}

Node *pair_addr(Pair *pair) {
    return pair->addr;
}
