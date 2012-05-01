#include "env.h"
#include "glib.h"
#include "printer.h"
#include "type.h"
#include <string.h>
#include <stdlib.h>

static Type env_type;

Node *env_find(Env *env, char const *key) {
    if (env == NULL) {
        fprintf(stderr, "symbol not found: %s\n", key);
        return NULL;
    }
    void *value;
    if (!g_hash_table_lookup_extended(env->table, key, NULL, &value)) {
        return env_find(env->outer, key);
    }
    if (!value) {
        fprintf(stderr, "uninitialized: %s\n", key);
        abort();
    }
    node_ref(value);
    return value;
}

bool env_set(Env *env, char const *key, Node *value) {
    gpointer old_value;
    // if the variable has not been defined, this is an error
    if (!g_hash_table_lookup_extended(env->table, key, NULL, &old_value)) {
        fprintf(stderr, "can't set an undefined variable: %s\n", key);
        return false;
    }
    // free the existing value if it's been set
    if (old_value) node_unref(old_value);
    g_hash_table_insert(env->table, strdup(key), value);
    return true;
}

// this can probably accept NULL value "*unassigned*"
bool env_define(Env *env, char const *key, Node *value) {
    if (g_hash_table_contains(env->table, key)) {
        fprintf(stderr, "variable %s already defined\n", key);
        return false;
    }
    g_hash_table_insert(env->table, strdup(key), value);
    return true;
}

bool env_undefine(Env *env, char const *key) {
    gpointer value;
    if (!g_hash_table_lookup_extended(env->table, key, NULL, &value)) return false;
    if (value) node_unref(value);
    if (!g_hash_table_remove(env->table, key)) abort();
    return true;
}

bool env_is_defined(Env *env, char const *key) {
    return g_hash_table_contains(env->table, key);
}

Env *env_check(Node *node) {
    return node->type == &env_type ? (Env *)node : NULL;
}

static void env_dealloc(Node *node) {
    Env *env = env_check(node);
    g_hash_table_destroy(env->table);
}

static void env_traverse(Node *_env, VisitProc visit, void *arg) {
    Env *env = (Env *)_env;
    GHashTableIter iter;
    gpointer _node;
    g_hash_table_iter_init(&iter, env->table);
    while (g_hash_table_iter_next(&iter, NULL, &_node)) {
        Node *node = _node;
        visit(node, arg);
    }
    if (env->outer) visit((Node *)env->outer, arg);
}

static void env_print(Node *_env, Printer *p) {
    Env *env = (Env *)_env;
    print_token(p, START);
    print_atom(p, "#(%s", env_type.name);
    GHashTableIter iter;
    gpointer _key, _node;
    g_hash_table_iter_init(&iter, env->table);
    while (g_hash_table_iter_next(&iter, &_key, &_node)) {
        char const *key = _key;
        print_token(p, START);
        print_atom(p, "%s", key);
        node_print(_node, p);
        print_token(p, END);
    }
    print_token(p, END);
}

static Type env_type = {
    .name = "env",
    .dealloc = env_dealloc,
    .traverse = env_traverse,
    .print = env_print,
};

Env *env_new(Env *outer) {
    Env *ret = malloc(sizeof *ret);
    node_init(ret, &env_type);
    ret->table = g_hash_table_new_full(g_str_hash,
                                       g_str_equal,
                                       free,
                                       NULL),
    ret->outer = outer;
    if (outer) node_ref(outer);
    return ret;
}

