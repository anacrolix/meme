#include "list.h"
#include "meme.h"

void list_print(List const *n, Printer *p) {
    fputc('(', p->file);
    p->just_atom = false;
    for (int i = 0; i < n->len; i++) {
        print_node(n->data[i], p);
    }
    fputc(')', p->file);
    p->just_atom = false;
}

Node *list_eval(List *node, Env *env) {
    Node *callable = eval(*node->data, env);
    if (!callable) return NULL;
    Node *ret = call_node(callable, node->data + 1, node->len - 1, env);
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

Type list_type = {
    .dealloc = list_dealloc,
};

List *list_new() {
    List *ret = calloc(1, sizeof *ret);
    node_init(ret, &list_type);
    return ret;
}
