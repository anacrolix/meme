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
    if (env == NULL) {
        fprintf(stderr, "symbol not found: ");
        node_print_file(key, stderr);
        fputc('\n', stderr);
        return NULL;
    }
    void *value;
    if (!g_hash_table_lookup_extended(env->table, key, NULL, &value)) {
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
    gpointer orig_key, old_value;
    // if the variable has not been defined, this is an error
    if (!g_hash_table_lookup_extended(env->table, key, &orig_key, &old_value)) {
        fprintf(stderr, "can't set an undefined variable: ");
        node_print_file(key, stderr);
        fputc('\n', stderr);
        return false;
    }
    // free the existing value if it's been set
    if (old_value) node_unref(old_value);
    g_hash_table_replace(env->table, key, value);
    node_unref(orig_key);
    return true;
}

// this can probably accept NULL value "*unassigned*"
bool env_define(Env *env, Symbol *key, Node *value) {
    if (g_hash_table_contains(env->table, key)) {
        fprintf(stderr, "variable ");
        node_print_file(key, stderr);
        fputs(" already defined\n", stderr);
        return false;
    }
    g_hash_table_replace(env->table, key, value);
    return true;
}

bool env_undefine(Env *env, Symbol *key) {
    gpointer orig_key, value;
    if (!g_hash_table_lookup_extended(env->table, key, &orig_key, &value)) return false;
    if (value) node_unref(value);
    if (!g_hash_table_remove(env->table, key)) abort();
    node_unref(orig_key);
    return true;
}

bool env_is_defined(Env *env, Symbol *key) {
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
    gpointer key, value;
    g_hash_table_iter_init(&iter, env->table);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        visit(key, arg);
        visit(value, arg);
    }
    if (env->outer) visit(env->outer, arg);
}

static void env_print(Node *_env, Printer *p) {
    Env *env = (Env *)_env;
    print_token(p, START);
    print_atom(p, "#(%s", env_type.name);
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, env->table);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        print_token(p, START);
        node_print(key, p);
        node_print(value, p);
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

static guint symbol_g_hash(gconstpointer _symbol) {
    Symbol const *symbol = _symbol;
    return g_str_hash(symbol_str(symbol));
}

static gboolean symbol_g_equal(gconstpointer _sym1, gconstpointer _sym2) {
    Symbol const *sym1 = _sym1, *sym2 = _sym2;
    if (sym1 == sym2) return true;
    return !strcmp(symbol_str(sym1), symbol_str(sym2));
}

Env *env_new(Env *outer) {
    Env *ret = malloc(sizeof *ret);
    node_init(ret, &env_type);
    ret->table = g_hash_table_new(symbol_g_hash,
                                  symbol_g_equal);
    ret->outer = outer;
    if (outer) node_ref(outer);
    return ret;
}

