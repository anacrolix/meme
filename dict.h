#pragma once

#include "node.h"
#include "glib.h"

typedef struct {
    Node;
    GHashTable *table;
} Dict;

