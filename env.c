#include "env.h"
#include "meme.h"
#include <glib.h>
#include <string.h>

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

void env_set(Env *env, char const *key, Node *value) {
    g_hash_table_insert(env->table, strdup(key), value);
}

Type env_type;

Env *env_new(Env *outer) {
    Env *ret = malloc(sizeof *ret);
    node_init(ret, &env_type);
    ret->table = g_hash_table_new_full(g_str_hash,
                                      g_str_equal,
                                      free,
                                      (GDestroyNotify)node_unref),
    ret->outer = outer;
    if (outer) node_ref(outer);
    return ret;
}
