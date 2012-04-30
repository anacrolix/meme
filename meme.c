#include "meme.h"
#include "glib.h"
#include "pair.h"
#include "int.h"
#include "printer.h"
#include "true.h"
#include "false.h"
#include "quote.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

static Node all_nodes[1] = {
    {
        .next = all_nodes,
        .prev = all_nodes,
    },
};

bool is_null(Pair *pair) {
    return pair == nil_node;
}

static size_t list_length(Pair *list);

static Node *apply_splat(Pair *args, Env *env) {
    if (list_length(args) < 2) return NULL;
    Int *opnd = int_check(pair_addr(args));
    if (!opnd) return NULL;
    long long ll = int_as_ll(opnd);
    for (;;) {
        args = args->dec;
        if (!args->addr) break;
        opnd = int_check(args->addr);
        if (!opnd) return NULL;
        ll *= opnd->ll;
    }
    return int_new(ll);
}

Node *apply_builtin_define(Pair *args, Env *env) {
    if (!args->addr) return NULL;
    Symbol *var = symbol_check(args->addr);
    if (!var) return NULL;
    args = args->dec;
    if (!args->addr) return NULL;
    Node *value = node_eval(args->addr, env);
    if (!value) return NULL;
    char const *key = symbol_str(var);
    // steals ref to value, copies key
    if (!env_define(env, key, value)) {
        node_unref(value);
        return NULL;
    }
    node_ref(void_node);
    return void_node;
}

typedef struct {
    Pair *fixed;
    Symbol *rest;
} Formals;

// parses formals of the kind (a b . c)
// returns the fixed list
// stores and refs *rest if it's included else sets it to NULL
// does NOT handle a flat symbol
// TODO: likely handles (. a) and (. .)
static Pair *parse_formals(Pair *vars, Symbol **rest) {
    if (vars == nil_node) {
        node_ref(nil_node);
        // the list finished without a .
        *rest = NULL;
        return nil_node;
    }
    Symbol *addr = symbol_check(vars->addr);
    // vars must be symbols
    if (!addr) return NULL;
    if (!strcmp(".", symbol_str(addr))) {
        // check the node after the . is a symbol and store it
        if (is_null(vars->dec)) return NULL;
        *rest = symbol_check(vars->dec->addr);
        if (!*rest) return NULL;
        // more than one symbol after the .
        if (vars->dec->dec != nil_node) return NULL;
        // terminate the fixed vars
        node_ref(*rest);
        node_ref(nil_node);
        return nil_node;
    }
    Pair *dec = parse_formals(vars->dec, rest);
    if (!dec) return NULL;
    Pair *pair = pair_new();
    pair->addr = addr;
    node_ref(addr);
    pair->dec = dec;
    return pair;
}

static bool formals_init(Formals *f, Node *node) {
    Pair *pair = pair_check(node); 
    if (pair) {
        f->fixed = parse_formals(pair, &f->rest);
        if (!f->fixed) return false;
    } else {
        f->rest = symbol_check(node);
        if (!f->rest) return false;
        node_ref(f->rest);
        f->fixed = nil_node;
        node_ref(nil_node);
    }
    return true;
}

static void formals_print(Formals const *f, Printer *p) {
    if (f->rest && f->fixed == nil_node) {
        node_print(f->rest, p);
        return;
    }
    print_token(p, START);
    Pair *args = f->fixed;
    for (; !is_null(args); args = args->dec) {
        node_print(args->addr, p);
    }
    if (f->rest) {
        print_atom(p, ".");
        node_print(f->rest, p);
    }
    print_token(p, END);
}

static void traverse_formals(Formals *f, VisitProc visit, void *arg) {
    visit(f->fixed, arg);
    if (f->rest) visit(f->rest, arg);
}

typedef struct {
    Node node[1];
    Node *body;
    Env *env;
    Formals formals[1];
} Closure;

static void closure_traverse(Node *_c, VisitProc visit, void *arg) {
    Closure *c = (Closure *)_c;
    visit(c->body, arg);
    visit(c->env, arg);
    traverse_formals(c->formals, visit, arg);
}

void closure_print(Node *_n, Printer *p) {
    Closure *n = (Closure *)_n;
    print_atom(p, "#(%s", n->node->type->name);
    formals_print(n->formals, p);
    node_print(n->body, p);
    print_token(p, END);
}

static bool extend_environment(Formals *formals, Pair *args, Env *env) {
    Pair *fixed = formals->fixed;
    for (; fixed->addr; fixed = fixed->dec, args = args->dec) {
        // not all fixed arguments were given
        if (!args->addr) return false;
        if (!env_define(env, symbol_str((Symbol *)fixed->addr), args->addr)) return false;
        node_ref(args->addr);
    }
    if (formals->rest) {
        if (!env_define(env, symbol_str(formals->rest), args)) return false;
        node_ref(args);
    } else if (args->addr) {
        fprintf(stderr, "too many arguments\n");
        return false;
    }
    return true;
}

static Node *closure_apply(Node *_proc, Pair *args, Env *env) {
    Closure *proc = (Closure *)_proc;
    Env *sub_env = env_new(proc->env);
    if (!extend_environment(proc->formals, args, sub_env)) {
        fprintf(stderr, "error extending environment\n  procedure: ");
        node_print_file(proc, stderr);
        fprintf(stderr, "\n  arguments: ");
        node_print_file(args, stderr);
        fputc('\n', stderr);
        node_unref(sub_env);
        return NULL;
    }
    Node *ret = node_eval(proc->body, sub_env);
    node_unref(sub_env);
    return ret;
}

Type const closure_type = {
    .name = "closure",
    .apply = closure_apply,
    .print = closure_print,
    .traverse = closure_traverse,
};

static Closure *closure_new(Node *vars, Node *body, Env *env) {
    Formals formals;
    if (!formals_init(&formals, vars)) return NULL;
    Closure *ret = malloc(sizeof *ret);
    node_init(ret, &closure_type);
    node_ref(env);
    ret->env = env;
    *ret->formals = formals;
    node_ref(body);
    ret->body = body;
    return ret;
}

static Node *apply_lambda(Pair *args, Env *env) {
    if (!args->addr) return NULL;
    Node *body = args->dec->addr;
    if (!body) return NULL;
    if (args->dec->dec->addr) return NULL;
    return closure_new(args->addr, body, env);
}

Node *apply_if(Pair *args, Env *env) {
    Node *test = args->addr;
    if (!test) return NULL;
    args = args->dec;
    Node *conseq = args->addr;
    if (!conseq) return NULL;
    args = args->dec;
    Node *alt;
    if (is_null(args)) alt = NULL;
    else {
        alt = args->addr;
        if (!is_null(args->dec)) return NULL;
    }
    Node *node = node_eval(test, env);
    if (!node) return NULL;
    int truth = node_truth(node);
    node_unref(node);
    if (truth < 0) return NULL;
    if (truth) return node_eval(conseq, env);
    if (!alt) {
        node_ref(void_node);
        return void_node;
    }
    return node_eval(alt, env);
}

Node *subtract(Pair *args, Env *env) {
    if (is_null(args)) return NULL;
    Int *operand = int_check(args->addr);
    if (!operand) return NULL;
    args = args->dec;
    if (is_null(args)) return int_new(-operand->ll);
    Int *ret = int_new(operand->ll);
    for (; args->addr; args = args->dec) {
        operand = int_check(args->addr);
        if (!operand) {
            node_unref(ret);
            return NULL;
        }
        ret->ll -= operand->ll;
    }
    return ret;
}

static NodeCmp invert_nodecmp(NodeCmp cmp) {
    switch (cmp) {
    case NODE_CMP_ERR:
        return NODE_CMP_ERR;
    case NODE_CMP_LT:
        return NODE_CMP_GT;
    case NODE_CMP_EQ:
        return NODE_CMP_NE;
    case NODE_CMP_GT:
        return NODE_CMP_LT;
    default:
        assert(false);
    }
}

void node_print_file(Node *node, FILE *file) {
    Printer p[1];
    printer_init(p, file);
    node_print(node, p);
    printer_destroy(p);
}

NodeCmp node_compare(Node *left, Node *right) {
    if (left == right) return NODE_CMP_EQ;
    else if (left->type->compare) return left->type->compare(left, right);
    else if (right->type->compare) return invert_nodecmp(right->type->compare(right, left));
    else return NODE_CMP_NE;
}

Node *less_than(Pair *args, Env *env) {
    if (list_length(args) != 2) return NULL;
    Node *left = args->addr;
    Node *right = args->dec->addr;
    Node *ret;
    switch (node_compare(left, right)) {
    case NODE_CMP_ERR:
        ret = NULL;
        break;
    case NODE_CMP_LT:
        ret = true_node;
        break;
    case NODE_CMP_NE:
    case NODE_CMP_EQ:
    case NODE_CMP_GT:
        ret = false_node;
        break;
    default:
        abort();
    }
    if (ret) node_ref(ret);
    return ret;
}

Node *is_pair(Pair *args, Env *env) {
    if (is_null(args) || !is_null(args->dec)) return NULL;
    Node *ret = pair_check(args->addr) ? true_node : false_node;
    node_ref(ret);
    return ret;
}

static void quote_print(Node *_quote, Printer *p) {
    assert(_quote->type == &quote_type);
    Quote *quote = (Quote *)_quote;
    print_token(p, QUOTE);
    node_print(quote->quoted, p);
}

static Node *quote_eval(Node *_quote, Env *env) {
    Quote *quote = (Quote *)_quote;
    Node *ret = quote->quoted;
    node_ref(ret);
    return ret;
}

static void quote_traverse(Node *_q, VisitProc visit, void *arg) {
    Quote *q = (Quote *)_q;
    visit(q->quoted, arg);
}

Type const quote_type = {
    .name = "quote",
    .print = quote_print,
    .eval = quote_eval,
    .traverse = quote_traverse,
};

typedef struct Macro {
    Node node[1];
    Node *text;
} Macro;

extern Type const macro_type;

static void macro_traverse(Node *_macro, VisitProc visit, void *arg) {
    Macro *macro = (Macro *)_macro;
    visit(macro->text, arg);
}

static Node *macro_apply(Node *_macro, Pair *args, Env *env) {
    assert(_macro->type == &macro_type);
    Macro *macro = (Macro *)_macro;
    Node *code = node_apply(macro->text, args, env);
    if (!code) return NULL;
#if 0
    fprintf(stderr, "macro produced: ");
    node_print_file(code, stderr);
    fputc('\n', stderr);
#endif
    Node *ret = node_eval(code, env);
    node_unref(code);
    return ret;
}

Pair *eval_list(Pair *args, Env *env) {
    if (!args->addr) {
        node_ref(nil_node);
        return nil_node;
    }
    Node *addr = node_eval(args->addr, env);
    if (!addr) return NULL;
    Pair *dec = eval_list(args->dec, env);
    if (!dec) {
        node_unref(addr);
        return NULL;
    }
    Pair *ret = pair_new();
    ret->addr = addr;
    ret->dec = dec;
    return ret;
}

static Node *apply_list(Pair *args, Env *env) {
    node_ref(args);
    return args;
}

static void macro_print(Node *_macro, Printer *p) {
    assert(_macro->type == &macro_type);
    Macro *macro = (Macro *)_macro;
    print_atom(p, "#(%s", macro_type.name);
    node_print(macro->text, p);
    print_token(p, END);
}

Type const macro_type = {
    .name = "macro",
    .apply = macro_apply,
    .print = macro_print,
    .traverse = macro_traverse,
    .special = true,
};

Node *macro_new(Pair *args, Env *env) {
    if (is_null(args)) return NULL;
    if (!is_null(args->dec)) return NULL;
    Macro *ret = malloc(sizeof *ret);
    node_init(ret, &macro_type);
    ret->text = args->addr;
    node_ref(ret->text);
    return ret;
}

typedef struct {
    Node node[1];
    Node *(*apply)(Pair *args, Env *env);
    char const *name;
} Primitive;

typedef Primitive Special;

static Type const primitive_type;

typedef Node *(*PrimitiveApplyFunc)(Pair *, Env *);

static Node *primitive_apply(Node *_proc, Pair *args, Env *env) {
    Primitive *proc = (Primitive *)_proc;
    Node *ret = proc->apply(args, env);
    if (!ret) {
        fprintf(stderr, "error applying ");
        node_print_file(proc, stderr);
        fprintf(stderr, " to ");
        node_print_file(args, stderr);
        fputc('\n', stderr);
    }
    return ret;
}

static void primitive_print(Node *_n, Printer *p) {
    Primitive *prim = (Primitive *)_n;
    fprintf(p->file, "#(%s)", prim->name);
}

static Type const primitive_type = {
    .name = "primitive",
    .apply = primitive_apply,
    .print = primitive_print,
};

static Type const special_type = {
    .name = "special",
    .apply = primitive_apply,
    .print = primitive_print,
    .special = true,
};

static Primitive *primitive_new(PrimitiveApplyFunc func, char const *name) {
    Primitive *ret = malloc(sizeof *ret);
    node_init(ret, &primitive_type);
    ret->apply = func;
    ret->name = name;
    return ret;
}

static Special *special_new(PrimitiveApplyFunc func, char const *name) {
    Special *ret = primitive_new(func, name);
    ret->node->type = &special_type;
    return ret;
}

static Node *apply_car(Pair *args, Env *env) {
    // 0 args
    if (!args->addr) return NULL;
    // more than 1 arg
    if (args->dec->addr) return NULL;
    Node *node = args->addr;
    // argument is not a pair
    Pair *pair = pair_check(node);
    if (!pair) {
        fprintf(stderr, "expected a pair: ");
        node_print_file(node, stderr);
        fputc('\n', stderr);
        return NULL;
    }
    Node *ret = pair->addr;
    // this would make the pair the nil type, which has no car
    if (ret) node_ref(ret);
    return ret;
}

static Node *apply_cdr(Pair *args, Env *env) {
    if (!args->addr) return NULL;
    if (args->dec->addr) return NULL;
    Pair *pair = pair_check(args->addr);
    if (!pair || pair == nil_node) return NULL;
    Pair *ret = pair->dec;
    node_ref(ret);
    return ret;
}

static Node *apply_symbol_query(Pair *args, Env *env) {
    if (!is_null(args->dec)) return NULL;
    Node *ret = symbol_check(args->addr) ? true_node : false_node;
    node_ref(ret);
    return ret;
}

static Node *apply_plus(Pair *args, Env *env) {
    if (!args->addr) return NULL;
    long long ll = 0;
    for (; args->addr; args = args->dec) {
        Int *opnd = int_check(args->addr);
        if (!opnd) return NULL;
        ll += opnd->ll;
    }
    return int_new(ll);
}

static Node *apply_null_query(Pair *args, Env *env) {
    if (!args->addr || args->dec->addr) return NULL;
    Pair *pair = pair_check(args->addr);
    if (!pair) return NULL;
    Node *ret = pair->addr ? false_node : true_node;
    node_ref(ret);
    return ret;
}

static size_t list_length(Pair *list) {
    size_t ret = 0;
    for (; list->addr; ret++, list = list->dec);
    return ret;
}

static Node *apply_cons(Pair *args, Env *env) {
    if (list_length(args) != 2) return NULL;
    Pair *dec = pair_check(args->dec->addr);
    if (!dec) return NULL;
    Pair *ret = pair_new();
    ret->addr = args->addr;
    node_ref(ret->addr);
    ret->dec = dec;
    node_ref(ret->dec);
    return ret;
}

static Node *apply_eq_query(Pair *args, Env *env) {
    if (list_length(args) != 2) return NULL;
    Node *ret;
    switch (node_compare(args->addr, args->dec->addr)) {
    case NODE_CMP_ERR:
        ret = NULL;
        break;
    case NODE_CMP_EQ:
        ret = true_node;
        break;
    default:
        ret = false_node;
    }
    if (ret) node_ref(ret);
    return ret;
}

static Pair *flatten_vargs(Pair *args) {
    if (is_null(args->dec)) {
        Pair *pair = pair_check(args->addr);
        if (!pair) return NULL;
        node_ref(pair);
        return pair;
    }
    Pair *dec = flatten_vargs(args->dec);
    if (!dec) return NULL;
    Node *addr = args->addr;
    Pair *ret = pair_new();
    ret->addr = addr;
    ret->dec = dec;
    node_ref(addr);
    return ret;
}

// TODO test crash for (apply f)
static Node *apply_apply(Pair *args, Env *env) {
    if (list_length(args) < 2) return NULL;
    Pair *flat_args = flatten_vargs(args->dec);
    if (!flat_args) return NULL;
    Node *ret = node_apply(args->addr, flat_args, env);
    node_unref(flat_args);
    return ret;
}

static Node *apply_begin(Pair *args, Env *env) {
    for (; !is_null(args->dec); args = args->dec);
    node_ref(args->addr);
    return args->addr;
}

static Node *apply_defined_query(Pair *args, Env *env) {
    if (list_length(args) != 1) return NULL;
    Symbol *var = symbol_check(args->addr);
    if (!var) return NULL;
    Node *ret = env_is_defined(env, symbol_str(var)) ? true_node : false_node;
    node_ref(ret);
    return ret;
}

static Node *apply_quote(Pair *args, Env *env) {
    if (list_length(args) != 1) return NULL;
    Quote *quote = malloc(sizeof *quote);
    node_init(quote, &quote_type);
    quote->quoted = args->addr;
    node_ref(quote->quoted);
    return quote;
}


static Node *apply_undef(Pair *args, Env *env) {
    if (list_length(args) != 1) return NULL;
    Symbol *sym = symbol_check(args->addr);
    if (!sym) return NULL;
    if (!env_undefine(env, symbol_str(sym))) return NULL;
    node_ref(void_node);
    return void_node;
}

static Node *apply_set_bang(Pair *args, Env *env) {
    if (list_length(args) != 2) return NULL;
    Symbol *var = symbol_check(args->addr);
    if (!var) return NULL;
    Node *value = node_eval(args->dec->addr, env);
    if (!value) return NULL;
    if (!env_set(env, symbol_str(var), value)) {
        node_unref(value);
        return NULL;
    }
    node_ref(void_node);
    return void_node;
}

typedef struct {
    char const *name;
    PrimitiveApplyFunc apply;
} PrimitiveType;

static PrimitiveType special_forms[] = {
    {"lambda", apply_lambda},
    {"if", apply_if},
    {"__define", apply_builtin_define},
    {"__set!", apply_set_bang},
    {"__undef", apply_undef},
    {"__defined?", apply_defined_query},
};

static PrimitiveType primitives[] = {
    {"*", apply_splat},
    {"+", apply_plus},
    {"symbol?", apply_symbol_query},
    {"<", less_than},
    {"-", subtract},
    {"pair?", is_pair},
    {"list", apply_list},
    {"car", apply_car},
    {"cdr", apply_cdr},
    {"null?", apply_null_query},
    {"cons", apply_cons},
    {"eq?", apply_eq_query},
    {"=", apply_eq_query}, // TODO split =/ eqv? eq
    {"begin", apply_begin},
    {"apply", apply_apply},
    {"macro", macro_new},
    {"__quote", apply_quote},
};

Env *top_env_new() {
    Env *ret = env_new(NULL);
    PrimitiveType *prim = special_forms;
    size_t count = sizeof special_forms / sizeof *special_forms;
    for (; count; prim++, count--) {
        if (!env_define(ret, prim->name, special_new(prim->apply, prim->name))) {
            node_unref(ret);
            return NULL;
        }
    }
    prim = primitives;
    count = sizeof primitives / sizeof *primitives;
    for (; count; prim++, count--) {
        if (!env_define(ret, prim->name, primitive_new(prim->apply, prim->name))) {
            node_unref(ret);
            return NULL;
        }
    }
    return ret;
}

void link_node(Node *node) {
    node->next = all_nodes->next;
    node->prev = all_nodes;
    all_nodes->next = node;
    node->next->prev = node;
}

void unlink_node(Node *node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

void meme_init() {
    assert(all_nodes->next == all_nodes && all_nodes->prev == all_nodes);
    link_node(nil_node);
    link_node(true_node);
    link_node(false_node);
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
    for (Node *node = all_nodes->next; node != all_nodes; node = node->next) {
        assert(!g_hash_table_contains(ret, node));
        g_hash_table_insert(ret, node, GINT_TO_POINTER(0));
    }
    for (Node *node = all_nodes->next; node != all_nodes; node = node->next) {
        if (!node->type->traverse) continue;
        node->type->traverse(node, inc_node_refs_in_table, ret);
    }
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
        unlink_node(node);
        free(_node);
    }
}

void collect_cycles() {
    GSList *roots = get_root_nodes();
    //print_roots(roots);
    GHashTable *reachable = get_reachable(roots);
    GHashTable *unreachable = g_hash_table_new(g_direct_hash, g_direct_equal);
    for (Node *node = all_nodes->next; node != all_nodes; node = node->next) {
        if (g_hash_table_contains(reachable, node)) continue;
        g_hash_table_add(unreachable, node);
    }
    //fprintf(stderr, "*** UNREACHABLE ***\n");
    //print_node_set(unreachable);
    //fprintf(stderr, "*** /UNREACHABLE ***\n");
    g_hash_table_destroy(reachable);
    g_slist_free(roots);
    collect_unreachable(unreachable);
    g_hash_table_destroy(unreachable);
}

void meme_final() {
    collect_cycles();
    if (nil_node->node->refs != 1) abort();
    unlink_node(nil_node);
    unlink_node(true_node);
    unlink_node(false_node);
    assert(all_nodes->next == all_nodes && all_nodes->prev == all_nodes);
}
