#include "meme.h"
#include <glib.h>
#include <assert.h>
#include <string.h>

GHashTable *all_nodes;

static bool is_null(Pair *pair) {
    return pair == nil_node;
}

static Pair *eval_list(Pair *args, Env *env);
static Pair *eval_args(Pair *fixed, Node *rest, Env *env);

static Int *eval_to_int(Node *node, Env *env) {
    Node *castee = eval(node, env);
    if (!castee) return NULL;
    Int *ret = int_check(castee);
    if (!ret) node_unref(castee);
    return ret;
}

static Node *apply_splat(Pair *args, Env *env) {
    if (!args->addr) return NULL;
    if (!args->dec->addr) return NULL;
    Int *opnd = eval_to_int(args->addr, env);
    if (!opnd) return NULL;
    long long ll = opnd->ll;
    for (;;) {
        node_unref(opnd);
        args = args->dec;
        if (!args->addr) break;
        opnd = eval_to_int(args->addr, env);
        if (!opnd) return NULL;
        ll *= opnd->ll;
    }
    return int_new(ll);
}

Node *assign(Pair *args, Env *env) {
    if (!args->addr) return NULL;
    Symbol *var = symbol_check(args->addr);
    if (!var) return NULL;
    args = args->dec;
    if (!args->addr) return NULL;
    Node *value = eval(args->addr, env);
    if (!value) return NULL;
    char const *key = symbol_str(var);
    // steals ref to value, copies key
    env_set(env, key, value);
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

static void traverse_formals(Formals *f, VisitProc visit, void *arg) {
    visit(f->fixed, arg);
    if (f->rest) visit(f->rest, arg);
}

typedef struct {
    Node;
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
    fprintf(stderr, "<closure ");
    node_print(n->body, p);
    fputc('>', stderr);
}

static bool extend_environment(Formals *formals, Pair *args, Env *env) {
    Pair *fixed = formals->fixed;
    for (; fixed->addr; fixed = fixed->dec, args = args->dec) {
        // not all fixed arguments were given
        if (!args->addr) return false;
        node_ref(args->addr);
        env_set(env, symbol_str((Symbol *)fixed->addr), args->addr);
    }
    if (formals->rest) {
        node_ref(args);
        env_set(env, symbol_str(formals->rest), args);
    } else if (args->addr) return false;
    return true;
}

static Node *closure_apply(Node *_proc, Pair *args, Node *vargs, Env *env) {
    Closure *proc = (Closure *)_proc;
    Pair *values = eval_args(args, vargs, env);
    if (!values) return NULL;
    Env *sub_env = env_new(proc->env);
    if (!extend_environment(proc->formals, values, sub_env)) {
        node_unref(values);
        node_unref(sub_env);
        return NULL;
    }
    Node *ret = eval(proc->body, sub_env);
    node_unref(sub_env);
    node_unref(values);
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
    Node *node = eval(test, env);
    if (!node) return NULL;
    int truth = node_truth(node);
    node_unref(node);
    if (truth < 0) return NULL;
    if (truth) return eval(conseq, env);
    if (!alt) {
        node_ref(void_node);
        return void_node;
    }
    return eval(alt, env);
}

Node *subtract(Pair *args, Env *env) {
    if (!args->addr) return NULL;
    Node *node = eval(args->addr, env);
    if (!node) return NULL;
    Int *operand = int_check(node);
    if (!operand) {
        node_unref(node);
        return NULL;
    }
    args = args->dec;
    Int *ret;
    if (!args->addr) {
        ret = int_new(-operand->ll);
        node_unref(operand);
        return ret;
    }
    ret = int_new(operand->ll);
    node_unref(operand);
    for (; args->addr; args = args->dec) {
        node = eval(args->addr, env);
        if (!node) {
            node_unref(ret);
            return NULL;
        }
        operand = int_check(node);
        if (!operand) {
            node_unref(ret);
            node_unref(operand);
            return NULL;
        }
        ret->ll -= operand->ll;
        node_unref(operand);
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

static void node_print_file(Node *node, FILE *file) {
    node_print(node, &(Printer){.file=file});
}

static NodeCmp node_compare(Node *left, Node *right) {
    if (left == right) return NODE_CMP_EQ;
    else if (left->type->compare) return left->type->compare(left, right);
    else if (right->type->compare) return invert_nodecmp(right->type->compare(right, left));
    else return NODE_CMP_NE;
}

Node *less_than(Pair *args, Env *env) {
    Node *left = args->addr;
    if (!left) return NULL;
    args = args->dec;
    Node *right = args->addr;
    if (!right) return NULL;
    args = args->dec;
    if (args->addr) return NULL;
    left = eval(left, env);
    if (!left) return NULL;
    right = eval(right, env);
    if (!right) {
        node_unref(left);
        return NULL;
    }
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
    node_unref(left);
    node_unref(right);
    return ret;
}

Node *is_pair(Pair *args, Env *env) {
    // only take one arg
    if (args->dec->addr) return NULL;
    Node *node = eval(args->addr, env);
    if (!node) return NULL;
    Node *ret = pair_check(node) ? true_node : false_node;
    node_ref(ret);
    node_unref(node);
    return ret;
}

void quote_print(Node *_quote, Printer *p) {
    assert(_quote->type == &quote_type);
    Quote *quote = (Quote *)_quote;
    fputc('\'', p->file);
    p->just_atom = false;
    node_print(quote->quoted, p);
}

Node *quote_eval(Node *_quote, Env *env) {
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
    Node;
    Formals formals[1];
    Node *text;
} Macro;

extern Type const macro_type;

static void macro_traverse(Node *_macro, VisitProc visit, void *arg) {
    Macro *macro = (Macro *)_macro;
    visit(macro->text, arg);
    traverse_formals(macro->formals, visit, arg);
}

// doesn't fail
static Pair *append(Pair *first, Pair *second) {
    if (first == nil_node) {
        node_ref(second);
        return second;
    }
    Pair *dec = append(first->dec, second);
    Node *addr = first->addr;
    Pair *ret = pair_new();
    node_ref(addr);
    ret->addr = addr;
    ret->dec = dec;
    return ret;
}

static Node *macro_apply(Node *_macro, Pair *args, Node *vargs, Env *env) {
    assert(_macro->type == &macro_type);
    Macro *macro = (Macro *)_macro;
    Pair *vargs_pair = pair_check(vargs);
    Pair *flat_args = append(args, vargs_pair);
    Env *sub_env = env_new(env);
    if (!extend_environment(macro->formals, flat_args, sub_env)) {
        node_unref(sub_env);
        node_unref(flat_args);
        return NULL;
    }
    Node *code = eval(macro->text, sub_env);
    node_unref(flat_args);
    node_unref(sub_env);
    if (!code) return NULL;
    Node *ret = eval(code, env);
    node_unref(code);
    return ret;
}

static Pair *eval_list(Pair *args, Env *env) {
    if (!args->addr) {
        node_ref(nil_node);
        return nil_node;
    }
    Node *addr = eval(args->addr, env);
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
    return eval_list(args, env);
}

static void macro_print(Node *_macro, Printer *p) {
    assert(_macro->type == &macro_type);
    Macro *macro = (Macro *)_macro;
    fprintf(p->file, "(#macro ");
    p->just_atom = false;
    node_print(macro->text, p);
    fputc(')', p->file);
    p->just_atom = false;
}

Type const macro_type = {
    .name = "macro",
    .apply = macro_apply,
    .print = macro_print,
    .traverse = macro_traverse,
};

Node *macro(Pair *args, Env *env) {
    Node *formals_arg = args->addr;
    if (!formals_arg) return NULL;
    args = args->dec;
    Node *text = args->addr;
    if (!text) return NULL;
    args = args->dec;
    if (args->addr) return NULL;
    Formals formals;
    if (!formals_init(&formals, formals_arg)) return NULL;
    Macro *ret = malloc(sizeof *ret);
    node_init(ret, &macro_type);
    node_ref(text);
    ret->text = text;
    *ret->formals = formals;
    return ret;
}

typedef struct {
    Node;
    Node *(*apply)(Pair *args, Env *env);
    char const *name;
} Primitive;

typedef Primitive Special;

static Type const primitive_type;

typedef Node *(*PrimitiveApplyFunc)(Pair *, Env *);

static Pair *eval_args(Pair *fixed, Node *rest, Env *env) {
    Pair *fixed_eval = eval_list(fixed, env);
    if (!rest) return fixed_eval;
    if (!fixed_eval) return NULL;
    Node *rest_eval = eval(rest, env);
    if (!rest_eval) {
        node_unref(fixed_eval);
        return NULL;
    }
    Pair *rest_eval_pair = pair_check(rest_eval);
    if (!rest_eval_pair) {
        node_unref(fixed_eval);
        node_unref(rest_eval);
        return NULL;
    }
    Pair *ret = append(fixed_eval, rest_eval_pair);
    node_unref(fixed_eval);
    node_unref(rest_eval);
    return ret;
}

static Node *primitive_apply(Node *_proc, Pair *args, Node *vargs, Env *env) {
    Primitive *proc = (Primitive *)_proc;
    Pair *flat_args = eval_args(args, vargs, env);
    if (!flat_args) return NULL;
    Node *ret = proc->apply(flat_args, env);
    node_unref(flat_args);
    return ret;
}

static Node *special_apply(Node *_proc, Pair *args, Node *vargs, Env *env) {
    Primitive *proc = (Primitive *)_proc;
    Pair *flat_args;
    if (vargs) {
        Pair *vargs_pair = pair_check(vargs);
        if (!vargs_pair) return NULL;
        flat_args = append(args, vargs_pair);
    } else {
        flat_args = args;
        node_ref(flat_args);
    }
    Node *ret = proc->apply(flat_args, env);
    node_unref(flat_args);
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
    .apply = special_apply,
    .print = primitive_print,
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
    ret->type = &special_type;
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
    if (!args->addr) return NULL;
    Node *operand = eval(args->addr, env);
    if (!operand) return NULL;
    Node *ret = symbol_check(args->addr) ? true_node : false_node;
    node_unref(operand);
    node_ref(ret);
    return ret;
}

static Node *apply_plus(Pair *args, Env *env) {
    if (!args->addr) return NULL;
    long long ll = 0;
    for (; args->addr; args = args->dec) {
        Int *opnd = eval_to_int(args->addr, env);
        if (!opnd) return NULL;
        ll += opnd->ll;
        node_unref(opnd);
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

// TODO test crash for (apply f)
static Node *apply_apply(Pair *args, Env *env) {
    if (list_length(args) != 2) return NULL;
    Node *proc = eval(args->addr, env);
    if (!proc) return NULL;
    Node *ret = node_apply(proc, nil_node, args->dec->addr, env);
    node_unref(proc);
    return ret;
}

static Node *apply_define(Pair *args, Env *env) {
    if (list_length(args) != 2) return NULL;
    Pair *pair = pair_check(args->addr);
    if (!pair) return assign(args, env);
    Symbol *name = symbol_check(pair->addr);
    if (!name) return NULL;
    Closure *closure = NULL;
    if (list_length(pair->dec) == 2) {
        Symbol *dot = symbol_check(pair->dec->addr);
        if (!dot) return NULL;
        if (!strcmp(".", symbol_str(dot))) {
            closure = closure_new(pair->dec->dec->addr, args->dec->addr, env);
            if (!closure) return NULL; // Y U NO CLOSURE?
        }
    }
    if (!closure) closure = closure_new(pair->dec, args->dec->addr, env);
    if (!closure) return NULL;
    env_set(env, symbol_str(name), closure);
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
    {"macro", macro},
    {"define", apply_define},
    {"apply", apply_apply},
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

void meme_init() {
    assert(!all_nodes);
    all_nodes = g_hash_table_new(g_direct_hash, g_direct_equal);
    g_hash_table_add(all_nodes, nil_node);
    g_hash_table_add(all_nodes, true_node);
    g_hash_table_add(all_nodes, false_node);
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
    //print_roots(roots);
    GHashTable *reachable = get_reachable(roots);
    GHashTable *unreachable = node_set_difference(all_nodes, reachable);
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
    if (nil_node->refs != 1) abort();
    if (!g_hash_table_remove(all_nodes, nil_node)) abort();
    if (!g_hash_table_remove(all_nodes, true_node)) abort();
    if (!g_hash_table_remove(all_nodes, false_node)) abort();
    if (g_hash_table_size(all_nodes)) abort();
    g_hash_table_destroy(all_nodes);
    all_nodes = NULL;
}
