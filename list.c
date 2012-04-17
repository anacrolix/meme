#include "list.h"
#include "meme.h"
#include <assert.h>

void list_print(Node const *nn, Printer *p) {
    List const *n = (List const *)nn;
    fputc('(', p->file);
    p->just_atom = false;
    for (int i = 0; i < n->len; i++) {
        node_print(n->data[i], p);
    }
    fputc(')', p->file);
    p->just_atom = false;
}

static Node *list_eval(Node *node, Env *env) {
    assert(node->type = &list_type);
    List *list = (List *)node;
    Node *callable = eval(list->data[0], env);
    if (!callable) return NULL;
    Node *ret = call_node(callable, list->data + 1, list->len - 1, env);
    node_unref(callable);
    return ret;
}

static void list_dealloc(Node *_l) {
    List *l = (List *)_l;
    for (int i = 0; i < l->len; i++) {
        node_unref(l->data[i]);
    }
}

List *list_resize(List *l, int len) {
    if (l->refs != 1) abort();
    if (len <= l->len) abort();
    l = realloc(l, sizeof *l + len * sizeof *l->data);
    l->len = len;
    return l;
}
    
List *list_append(List *l, Node *n) {
    l = list_resize(l, l->len + 1);
    l->data[l->len-1] = n;
    return l;
}

Type const list_type = {
    .name = "list",
    .dealloc = list_dealloc,
    .print = list_print,
    .eval = list_eval,
};

List *list_new() {
    List *ret = calloc(1, sizeof *ret);
    node_init(ret, &list_type);
    return ret;
}
