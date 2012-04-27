#include <glib.h>

#if !GLIB_CHECK_VERSION(2, 32, 0)
#   define g_hash_table_add(table, key) \
        g_hash_table_replace(table, key, key)
#   define g_hash_table_contains(table, key) \
        g_hash_table_lookup_extended(table, key, NULL, NULL)
#endif

