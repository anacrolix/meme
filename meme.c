#include "meme.h"
#include <glib.h>
#include <assert.h>

GHashTable *all_nodes;

void meme_init() {
    assert(!all_nodes);
    all_nodes = g_hash_table_new(g_direct_hash, g_direct_equal);
    g_hash_table_add(all_nodes, nil_node);
}

static void inc_node_refs_in_table(Node *node, void *_table) {
    GHashTable *table = _table;
    gpointer _refs;
    if (!g_hash_table_lookup_extended(table, node, NULL, &_refs)) abort();
    int refs = GPOINTER_TO_INT(_refs);
    g_hash_table_replace(table, node, GINT_TO_POINTER(refs+1));
}

static GHashTable *get_internal_ref_counts() {
    GHashTable *ret = g_hash_table_new(g_direct_hash, g_direct_equal);
    GHashTableIter iter;
    gpointer _node;
    g_hash_table_iter_init(&iter, all_nodes);
    while (g_hash_table_iter_next(&iter, &_node, NULL)) {
        g_hash_table_insert(ret, _node, GINT_TO_POINTER(0));
    }
    assert(g_hash_table_size(ret) == g_hash_table_size(all_nodes));
    g_hash_table_iter_init(&iter, all_nodes);
    while (g_hash_table_iter_next(&iter, &_node, NULL)) {
        Node *node = _node;
        if (!node->type->traverse) continue;
        node->type->traverse(node, inc_node_refs_in_table, ret);
    }
    assert(g_hash_table_size(ret) == g_hash_table_size(all_nodes));
    return ret;
}

static GSList *get_root_nodes() {
    GHashTable *internal_refs = get_internal_ref_counts();
    GSList *ret = NULL;
    GHashTableIter iter;
    gpointer _node, _refs;
    g_hash_table_iter_init(&iter, internal_refs);
    while (g_hash_table_iter_next(&iter, &_node, &_refs)) {
        Node *node = _node;
        int refs = GPOINTER_TO_INT(_refs);
        assert(0 <= refs && refs <= node->refs);
        if (refs < node->refs) ret = g_slist_prepend(ret, node);
    }
    g_hash_table_destroy(internal_refs);
    return ret;
}

static void add_reachable(Node *node, void *_table) {
    GHashTable *table = _table;
    if (g_hash_table_contains(table, node)) return;
    g_hash_table_add(table, node);
    if (node->type->traverse) node->type->traverse(node, add_reachable, table);
}

static GHashTable *get_reachable(GSList *roots) {
    GHashTable *ret = g_hash_table_new(g_direct_hash, g_direct_equal);
    for (; roots; roots = roots->next) {
        Node *node = roots->data;
        add_reachable(node, ret);
    }
    return ret;
}

static void print_node_set(GHashTable *node_set) {
    GHashTableIter iter;
    g_hash_table_iter_init(&iter, node_set);
    gpointer _node;
    while (g_hash_table_iter_next(&iter, &_node, NULL)) {
        node_print(_node, &(Printer){.file=stderr});
        fputc('\n', stderr);
    }
}

static void print_roots(GSList *roots) {
    for (; roots; roots = roots->next) {
        fprintf(stderr, "root node: ");
        node_print(roots->data, &(Printer){.file=stderr});
        fputc('\n', stderr);
    }
}

static GHashTable *node_set_difference(GHashTable *minuend, GHashTable *subtrahend) {
    GHashTable *ret = g_hash_table_new(g_direct_hash, g_direct_equal);
    GHashTableIter iter;
    g_hash_table_iter_init(&iter, minuend);
    gpointer _node;
    while (g_hash_table_iter_next(&iter, &_node, NULL)) {
        if (g_hash_table_contains(subtrahend, _node)) continue;
        g_hash_table_add(ret, _node);
    }
    return ret;
}

static void unref_if_reachable(Node *node, void *unreachable) {
    if (g_hash_table_contains(unreachable, node)) return;
    node_unref(node);
}

static void collect_unreachable(GHashTable *unreachable) {
    GHashTableIter iter;
    g_hash_table_iter_init(&iter, unreachable);
    gpointer _node;
    while (g_hash_table_iter_next(&iter, &_node, NULL)) {
        Node *node = _node;
        if (node->type->traverse) node->type->traverse(node, unref_if_reachable, unreachable);
        if (node->type->dealloc) node->type->dealloc(node);
        if (!g_hash_table_remove(all_nodes, _node)) abort();
        free(_node);
    }
}

void collect_cycles() {
    GSList *roots = get_root_nodes();
    print_roots(roots);
    GHashTable *reachable = get_reachable(roots);
    GHashTable *unreachable = node_set_difference(all_nodes, reachable);
    fprintf(stderr, "*** UNREACHABLE ***\n");
    print_node_set(unreachable);
    fprintf(stderr, "*** /UNREACHABLE ***\n");
    g_hash_table_destroy(reachable);
    g_slist_free(roots);
    collect_unreachable(unreachable);
    g_hash_table_destroy(unreachable);
}

void meme_final() {
    collect_cycles();
    if (nil_node->refs != 1) abort();
    if (!g_hash_table_remove(all_nodes, nil_node)) abort();
    if (g_hash_table_size(all_nodes)) abort();
    g_hash_table_destroy(all_nodes);
    all_nodes = NULL;
}
