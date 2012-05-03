#include "env.h"
#include "meme.h"
#include "glib.h"
#include "printer.h"
#include "type.h"
#include "symbol.h"
#include <string.h>
#include <stdlib.h>

static Type env_type;

Node *env_find(Env *env, Symbol *key) {
    Node *value;
    if (!env->symtab->ops->lookup(env->symtab, key, &value)) {
        if (env->outer == NULL) {
            fprintf(stderr, "symbol not found: ");
            node_print_file(key, stderr);
            fputc('\n', stderr);
            return NULL;
        }
        return env_find(env->outer, key);
    }
    if (!value) {
        fprintf(stderr, "uninitialized: ");
        node_print_file(key, stderr);
        fputc('\n', stderr);
        return NULL; // TODO test this
    }
    node_ref(value);
    return value;
}

bool env_set(Env *env, Symbol *key, Node *value) {
    if (!env->symtab->ops->replace(env->symtab, key, value)) {
        fprintf(stderr, "can't set an undefined variable: ");
        node_print_file(key, stderr);
        fputc('\n', stderr);
        return false;
    }
    return true;
}

// this can probably accept NULL value "*unassigned*"
bool env_define(Env *env, Symbol *key, Node *value) {
    if (!env->symtab->ops->insert(env->symtab, key, value)) {
        fprintf(stderr, "variable ");
        node_print_file(key, stderr);
        fputs(" already defined\n", stderr);
        return false;
    }
    return true;
}

bool env_undefine(Env *env, Symbol *key) {
    return env->symtab->ops->remove(env->symtab, key);
}

bool env_is_defined(Env *env, Symbol *key) {
    return env->symtab->ops->contains(env->symtab, key);
}

Env *env_check(Node *node) {
    return node->type == &env_type ? (Env *)node : NULL;
}

static void env_dealloc(Node *node) {
    Env *env = (Env *)node;
    env->symtab->ops->free(env->symtab);
}

static void env_traverse(Node *_env, VisitProc visit, void *arg) {
    Env *env = (Env *)_env;
    env->symtab->ops->traverse(env->symtab, visit, arg);
    if (env->outer) visit(env->outer, arg);
}

static void env_print(Node *_env, Printer *p) {
    //Env *env = (Env *)_env;
    print_token(p, START);
    print_atom(p, "#(%s", env_type.name);
#if 0
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, env->table);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        print_token(p, START);
        node_print(key, p);
        node_print(value, p);
        print_token(p, END);
    }
#endif
    print_token(p, END);
}

static void env_free(Node *node) {
    NODE_FREE((Env *)node);
}

static Type env_type = {
    .name = "env",
    .dealloc = env_dealloc,
    .traverse = env_traverse,
    .print = env_print,
    .free = env_free,
};

Env *env_new(Env *outer, SymTab *(*symtab_new)(void)) {
    Env *ret = NODE_NEW(*ret);
    node_init(ret, &env_type);
    ret->symtab = symtab_new();
    ret->outer = outer;
    if (outer) node_ref(outer);
    return ret;
}

