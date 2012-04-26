#include "bool.h"
#include "env.h"
#include "eval.h"
#include "int.h"
#include "lexer.h"
#include "misc.h"
#include "node.h"
#include "pair.h"
#include "parse.h"
#include "quote.h"
#include "symbol.h"
#include "void.h"
#include <glib.h>
#include <stdlib.h>

typedef struct Env Env;
Env *top_env_new();
extern GHashTable *all_nodes;
void meme_init();
void meme_final();
void collect_cycles();
void node_print_file(Node *, FILE *);
Pair *eval_list(Pair *args, Env *env);
