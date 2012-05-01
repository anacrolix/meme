#include "env.h"
#include "glib.h"
#include "type.h"
#include "symbol.h"
#include <string.h>
#include <stdlib.h>

typedef struct {
    SymTab;
    GHashTable *table;
} GHashSymTab;

static bool lookup(void *_this, Symbol *key, Node **value) {
    GHashSymTab *this = _this;
    gpointer _value;
    bool found = g_hash_table_lookup_extended(this->table, key, NULL, &_value);
    *value = _value;
    return found;
}

static bool replace(void *_this, Symbol *key, Node *value) {
    GHashSymTab *this = _this;
    gpointer orig_key, old_value;
    // if the variable has not been defined, this is an error
    if (!g_hash_table_lookup_extended(this->table, key, &orig_key, &old_value)) return false;
    // free the existing value if it's been set
    if (old_value) node_unref(old_value);
    g_hash_table_replace(this->table, key, value);
    node_unref(orig_key);
    return true;
}

// this can probably accept NULL value "*unassigned*"
static bool insert(void *_this, Symbol *key, Node *value) {
    GHashSymTab *this = _this;
    if (g_hash_table_contains(this->table, key)) return false;
    g_hash_table_replace(this->table, key, value);
    return true;
}

static bool remove(void *_this, Symbol *key) {
    GHashSymTab *this = _this;
    gpointer orig_key, value;
    if (!g_hash_table_lookup_extended(this->table, key, &orig_key, &value)) return false;
    if (value) node_unref(value);
    if (!g_hash_table_remove(this->table, key)) abort();
    node_unref(orig_key);
    return true;
}

static bool contains(void *_this, Symbol *key) {
    GHashSymTab *this = _this;
    return g_hash_table_contains(this->table, key);
}

static void ghash_symtab_free(void *_this) {
    GHashSymTab *this = _this;
    g_hash_table_destroy(this->table);
    free(_this);
}

static void traverse(void *_this, VisitProc visit, void *arg) {
    GHashSymTab *this = _this;
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, this->table);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        visit(key, arg);
        visit(value, arg);
    }
}

static guint symbol_g_hash(gconstpointer _symbol) {
    Symbol const *symbol = _symbol;
    return g_str_hash(symbol_str(symbol));
}

static gboolean symbol_g_equal(gconstpointer _sym1, gconstpointer _sym2) {
    Symbol const *sym1 = _sym1, *sym2 = _sym2;
    if (sym1 == sym2) return true;
    return !strcmp(symbol_str(sym1), symbol_str(sym2));
}

static SymTabOps ghash_symtab_ops = {
    .lookup = lookup,
    .insert = insert,
    .replace = replace,
    .contains = contains,
    .remove = remove,
    .free = ghash_symtab_free,
    .traverse = traverse,
};

GHashSymTab *ghash_symtab_new(void) {
    GHashSymTab *ret = malloc(sizeof *ret);
    ret->table = g_hash_table_new(symbol_g_hash,
                                  symbol_g_equal);
    ret->ops = &ghash_symtab_ops;
    return ret;
}

