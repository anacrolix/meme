#include "var.h"
#include "meme.h"
#include <string.h>

typedef struct Var {
    Node;
    char *s;
} Var;

Node *var_eval(Var *node, Env *env) {
    return env_find(env, node->s);
}

char const *var_get_name(Var *v) {
    return v->s;
}

void var_print(Node const *n, Printer *p) {
    if (p->just_atom) fputc(' ', p->file);
    fputs(((Var *)n)->s, p->file);
    p->just_atom = true;
}

Type const var_type;

Var *var_new(char const *s) {
    Var *ret = malloc(sizeof *ret);
    node_init(ret, &var_type);
    ret->s = strdup(s);
    return ret;
}
