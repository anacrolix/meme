#pragma once

#include "env.h"
#include "int.h"
#include "pair.h"
#include "quote.h"
#include "symbol.h"
#include "true.h"
#include "types.h"
#include "glib.h"
#include "false.h"
#include "lexer.h"
#include "node.h"
#include "parse.h"
#include "void.h"
#include "printer.h"
#include <stdio.h>

Env *top_env_new();
void link_node(Node *);
void unlink_node(Node *);
void meme_init();
void meme_final();
void collect_cycles();
void node_print_file(Node *, FILE *);
Pair *eval_list(Pair *args, Env *env);
bool is_null(Pair *pair);
