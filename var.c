#include "var.h"
#include "meme.h"
#include <assert.h>
#include <string.h>

typedef struct Var {
    Node;
    char *s;
} Var;

Node *var_eval(Node *node, Env *env) {
    assert(node->type == &var_type);
    Var *var = (Var *)node;
    return env_find(env, var->s);
}

char const *var_get_name(Var *v) {
    return v->s;
}

void var_print(Node const *n, Printer *p) {
    if (p->just_atom) fputc(' ', p->file);
    fputs(((Var *)n)->s, p->file);
    p->just_atom = true;
}

static void var_dealloc(Node *node) {
    Var *var = (Var *)node;
    free(var->s);
}

Type const var_type = {
    .name = "var",
    .print = var_print,
    .dealloc = var_dealloc,
    .eval = var_eval
};

Var *var_new(char const *s) {
    Var *ret = malloc(sizeof *ret);
    node_init(ret, &var_type);
    ret->s = strdup(s);
    return ret;
}
