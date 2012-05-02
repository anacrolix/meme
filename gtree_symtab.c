#include "env.h"
#include "glib.h"
#include "type.h"
#include "symbol.h"
#include <string.h>
#include <stdlib.h>

typedef struct {
    SymTab;
    GTree *tree;
} GTreeSymTab;

static bool lookup(void *_this, Symbol *key, Node **value) {
    GTreeSymTab *this = _this;
    return g_tree_lookup_extended(this->tree, key, NULL, (gpointer *)value);
}

static bool replace(void *_this, Symbol *key, Node *value) {
    GTreeSymTab *this = _this;
    gpointer orig_key, old_value;
    // if the variable has not been defined, this is an error
    if (!g_tree_lookup_extended(this->tree, key, &orig_key, &old_value)) return false;
    // free the existing value if it's been set
    if (old_value) node_unref(old_value);
    g_tree_replace(this->tree, key, value);
    node_unref(orig_key);
    return true;
}

// this can probably accept NULL value "*unassigned*"
static bool insert(void *_this, Symbol *key, Node *value) {
    GTreeSymTab *this = _this;
    if (g_tree_lookup_extended(this->tree, key, NULL, NULL)) return false;
    g_tree_replace(this->tree, key, value);
    return true;
}

static bool remove(void *_this, Symbol *key) {
    GTreeSymTab *this = _this;
    gpointer orig_key, value;
    if (!g_tree_lookup_extended(this->tree, key, &orig_key, &value)) return false;
    if (value) node_unref(value);
    if (!g_tree_remove(this->tree, key)) abort();
    node_unref(orig_key);
    return true;
}

static bool contains(void *_this, Symbol *key) {
    GTreeSymTab *this = _this;
    return g_tree_lookup_extended(this->tree, key, NULL, NULL);
}

static void gtree_symtab_free(void *_this) {
    GTreeSymTab *this = _this;
    g_tree_destroy(this->tree);
    free(_this);
}

static void traverse(void *_this, VisitProc visit, void *arg) {
    GTreeSymTab *this = _this;
    gboolean func(gpointer key, gpointer value, gpointer data) {
        visit(key, arg);
        visit(value, arg);
        return FALSE;
    }
    g_tree_foreach(this->tree, func, NULL);
}

static gint symbol_g_compare(gconstpointer _sym1, gconstpointer _sym2) {
    Symbol const *sym1 = _sym1, *sym2 = _sym2;
#if 1
    return strcmp(sym1->s, sym2->s);
#else
    if (sym1->len < sym2->len) {
        int ret = memcmp(sym1->s, sym2->s, sym1->len);
        if (ret) return ret;
        return -1;
    } else if (sym1->len > sym2->len) {
        int ret = memcmp(sym1->s, sym2->s, sym2->len);
        if (ret) return ret;
        return 1;
    } else return memcmp(sym1->s, sym2->s, sym1->len);
#endif
}

static SymTabOps gtree_symtab_ops = {
    .lookup = lookup,
    .insert = insert,
    .replace = replace,
    .contains = contains,
    .remove = remove,
    .free = gtree_symtab_free,
    .traverse = traverse,
};

GTreeSymTab *gtree_symtab_new(void) {
    GTreeSymTab *ret = malloc(sizeof *ret);
    ret->tree = g_tree_new(symbol_g_compare);
    ret->ops = &gtree_symtab_ops;
    return ret;
}

