#include "env.h"
#include "meme.h"
#include <glib.h>
#include <string.h>

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
    if (g_hash_table_contains(env->table, key)) return false;
    g_hash_table_insert(env->table, strdup(key), value);
    return true;
}

// this can probably accept NULL value "*unassigned*"
bool env_define(Env *env, char const *key, Node *value) {
    if (g_hash_table_contains(env->table, key)) return false;
    g_hash_table_insert(env->table, strdup(key), value);
    return true;
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
    if (env->outer) visit(env->outer, arg);
}

static void env_print(Node *_env, Printer *p) {
    Env *env = (Env *)_env;
    fprintf(p->file, "; env {\n");
    GHashTableIter iter;
    gpointer _key, _node;
    g_hash_table_iter_init(&iter, env->table);
    while (g_hash_table_iter_next(&iter, &_key, &_node)) {
        char const *key = _key;
        fprintf(p->file, ";   %s ", key);
        node_print(_node, p);
        fputc('\n', p->file);
    }
    fprintf(p->file, "; }\n");
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
