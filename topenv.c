#include "meme.h"
#include <assert.h>
#include <string.h>

Node *multiply(Pair *args, Env *env) {
    if (!args->addr) return NULL;
    if (!args->dec->addr) return NULL;
    Node *node = eval(args->addr, env);
    if (!node) return NULL;
    Int *ret = int_check(node);
    if (!ret) {
        node_unref(node);
        return NULL;
    }
    args = args->dec;
    for (; args->addr; args = args->dec) {
        node = eval(args->addr, env);
        Int *int_node = int_check(node);
        if (!int_node) {
            node_unref(node);
            node_unref(ret);
            return NULL;
        }
        ret->ll *= int_node->ll;
        node_unref(int_node);
    }
    return ret;
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
    Node;
    Node *body;
    Env *env;
    Pair *vars;
} Closure;

static void closure_traverse(Node *_c, VisitProc visit, void *arg) {
    Closure *c = (Closure *)_c;
    visit(c->body, arg);
    visit(c->env, arg);
    visit(c->vars, arg);
}

void closure_print(Node *_n, Printer *p) {
    Closure *n = (Closure *)_n;
    fprintf(stderr, "<closure ");
    node_print(n->body, p);
    fputc('>', stderr);
}

static Node *closure_apply(Node *_proc, Pair *args, Env *env) {
    Closure *proc = (Closure *)_proc;
    Env *sub_env = env_new(proc->env);
    Pair *vars = proc->vars;
    for (;; args = args->dec, vars = vars->dec) {
        if (!args->addr) {
            if (vars->addr) return NULL;
            else break;
        } else if (!vars->addr) return NULL;
        Symbol *var = symbol_check(vars->addr);
        if (!var) {
            node_unref(sub_env);
            return NULL;
        }
        Node *val = eval(args->addr, env);
        if (!val) {
            node_unref(sub_env);
            return NULL;
        }
        env_set(sub_env, symbol_str(var), val);
    }
    Node *ret = eval(proc->body, sub_env);
    node_unref(sub_env);
    return ret;
}

Type const closure_type = {
    .name = "closure",
    .apply = closure_apply,
    .print = closure_print,
    .traverse = closure_traverse,
};

Node *lambda(Pair *args, Env *env) {
    Pair *vars = pair_check(args->addr);
    if (!vars) return NULL;
    Node *body = args->dec->addr;
    Closure *ret = malloc(sizeof *ret);
    node_init(ret, &closure_type);
    node_ref(env);
    ret->env = env;
    node_ref(vars);
    ret->vars = vars;
    node_ref(body);
    ret->body = body;
    return ret;
}

Node *apply_if(Pair *args, Env *env) {
    Node *test = args->addr;
    if (!test) return NULL;
    args = args->dec;
    Node *conseq = args->addr;
    if (!conseq) return NULL;
    args = args->dec;
    Node *alt = args->addr;
    if (!alt) return NULL;
    args = args->dec;
    if (args->addr) return NULL;
    Node *node = eval(test, env);
    if (!node) return NULL;
    int truth = node_truth(node);
    node_unref(node);
    if (truth < 0) return NULL;
    return eval(truth ? conseq : alt, env);
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

typedef enum {
    NODE_CMP_ERR,
    NODE_CMP_LT,
    NODE_CMP_GT,
    NODE_CMP_EQ,
} NodeCmp;

static NodeCmp node_compare(Node *_left, Node *_right) {
    Int *left = int_check(_left);
    Int *right = int_check(_right);
    if (!left || !right) return NODE_CMP_ERR;
    if (left->ll < right->ll) return NODE_CMP_LT;
    if (left->ll > right->ll) return NODE_CMP_GT;
    else return NODE_CMP_EQ;
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
    Pair *vars;
    Node *text;
} Macro;

extern Type const macro_type;

static void macro_traverse(Node *_macro, VisitProc visit, void *arg) {
    Macro *macro = (Macro *)_macro;
    visit(macro->vars, arg);
    visit(macro->text, arg);
}

static Node *macro_apply(Node *_macro, Pair *args, Env *env) {
    assert(_macro->type == &macro_type);
    Macro *macro = (Macro *)_macro;
    Env *sub_env = env_new(env);
    Pair *vars = macro->vars;
    for (;; args = args->dec, vars = vars->dec) {
        if (!args->addr) {
            if (vars->addr) {
                node_unref(sub_env);
                return NULL;
            }
            else break;
        } else if (!vars->addr) {
            node_unref(sub_env);
            return NULL;
        }
        Symbol *var = symbol_check(vars->addr);
        if (!var) {
            node_unref(sub_env);
            return NULL;
        }
        node_ref(args->addr);
        env_set(sub_env, symbol_str(var), args->addr);
    }
    Node *code = eval(macro->text, sub_env);
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
    Pair *vars = pair_check(args->addr);
    if (!vars) return NULL;
    args = args->dec;
    Node *text = args->addr;
    if (!text) return NULL;
    Macro *ret = malloc(sizeof *ret);
    node_init(ret, &macro_type);
    node_ref(text);
    ret->text = text;
    node_ref(vars);
    ret->vars = vars;
    return ret;
}

typedef struct {
    Node;
    Node *(*apply)(Pair *args, Env *env);
} Primitive;

static Type const primitive_type;

typedef Node *(*PrimitiveApplyFunc)(Pair *, Env *);

static Node *primitive_apply(Node *_proc, Pair *args, Env *env) {
    Primitive *proc = (Primitive *)_proc;
    return proc->apply(args, env);
}

static void primitive_print(Node *_n, Printer *p) {
    fprintf(p->file, "<%p>", _n);
}

static Type const primitive_type = {
    .name = "primitive",
    .apply = primitive_apply,
    .print = primitive_print,
};

static Primitive *primitive_new(PrimitiveApplyFunc func) {
    Primitive *ret = malloc(sizeof *ret);
    node_init(ret, &primitive_type);
    ret->apply = func;
    return ret;
}

static Node *apply_car(Pair *args, Env *env) {
    // 0 args
    if (!args->addr) return NULL;
    // more than 1 arg
    if (args->dec->addr) return NULL;
    Node *node = eval(args->addr, env);
    if (!node) return NULL;
    // argument is not a pair
    Pair *pair = pair_check(node);
    if (!pair) {
        node_unref(node);
        return NULL;
    }
    Node *ret = pair->addr;
    // this would make the pair the nil type, which has no car
    if (ret) node_ref(ret);
    node_unref(node);
    return ret;
}

static Node *apply_cdr(Pair *args, Env *env) {
    if (!args->addr) return NULL;
    if (args->dec->addr) return NULL;
    Node *node = eval(args->addr, env);
    if (!node) return NULL;
    if (node->type != &pair_type) {
        node_unref(node);
        return NULL;
    }
    Pair *pair = (Pair *)node;
    Pair *ret = pair->dec;
    node_ref(ret);
    node_unref(pair);
    return ret;
}

Env *top_env_new() {
    Env *ret = env_new(NULL);
    env_set(ret, "*", primitive_new(multiply));
    env_set(ret, "=", primitive_new(assign));
    env_set(ret, "^", primitive_new(lambda));
    env_set(ret, "?", primitive_new(apply_if));
    env_set(ret, "<", primitive_new(less_than));
    env_set(ret, "-", primitive_new(subtract));
    env_set(ret, "pair?", primitive_new(is_pair));
    env_set(ret, "#", primitive_new(macro));
    env_set(ret, "list", primitive_new(apply_list));
    env_set(ret, "car", primitive_new(apply_car));
    env_set(ret, "cdr", primitive_new(apply_cdr));
    return ret;
}
