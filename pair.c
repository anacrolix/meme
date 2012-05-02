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

static bool eval_list_to_array(Pair *list, Env *env, Node *array[const]) {
    Node **next = array;
    for (; !is_null(list); list = pair_dec(list), next++) {
        *next = node_eval(pair_addr(list), env);
        if (!*next) break;
    }
    if (is_null(list)) return true;
    while (--next >= array) {
        node_unref(*next);
    }
    return false;
}

static Node *pair_eval(Node *_pair, Env *env) {
    Pair *pair = (Pair *)_pair;
    Node *proc = node_eval(pair->addr, env);
    if (!proc) return NULL;
    int const argc = list_length(pair->dec);
    Node *args[argc];
    pair = pair->dec;
    bool special = node_special(proc);
    if (special) for (Node **dest = args; !is_null(pair); pair = pair_dec(pair), dest++) {
        *dest = pair_addr(pair);
    } else if (!eval_list_to_array(pair, env, args)) {
        node_unref(proc);
        return NULL;
    }
    Node *ret = node_apply(proc, args, argc, env);
    if (!special) for (int i = 0; i < argc; i++) {
        node_unref(args[i]);
    }
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

#ifdef SLICE_NODES
static void pair_free(Node *_pair) {
    Pair *pair = (Pair *)_pair;
    g_slice_free(Pair, pair);
}
#endif

Type const pair_type = {
    .name = "pair",
    .print = pair_print,
    .eval = pair_eval,
    .traverse = pair_traverse,
    .compare = pair_compare,
#ifdef SLICE_NODES
    .free = pair_free,
#endif
};

static Pair nil_node_storage = {
    {
        .refs = 1,
        .type = &pair_type,
    },
    .addr = NULL,
    .dec = NULL,
};

Pair *const nil_node = &nil_node_storage;

Pair *pair_new(Node *addr, Pair *dec) {
#ifdef SLICE_NODES
    Pair *pair = g_slice_new(Pair);
#else
    Pair *pair = malloc(sizeof *pair);
#endif
    node_init(pair, &pair_type);
    pair->addr = addr;
    pair->dec = dec;
    return pair;
}

Pair *pair_dec(Pair *pair) {
    return pair->dec;
}

Node *pair_addr(Pair *pair) {
    return pair->addr;
}

size_t list_length(Pair *list) {
    size_t ret = 0;
    for (; list->addr; ret++, list = list->dec);
    return ret;
}

