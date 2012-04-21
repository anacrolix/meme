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

void closure_dealloc(Node *node) {
    Closure *c = (Closure *)node;
    node_unref(c->body);
    node_unref(c->env);
    node_unref(c->vars);
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
    .dealloc = closure_dealloc,
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
    if (!args->addr || args->dec->addr) return NULL;
    Node *ret = pair_check(args->addr) ? true_node : false_node;
    node_ref(ret);
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

Env *top_env_new() {
    Env *ret = env_new(NULL);
    env_set(ret, "*", primitive_new(multiply));
    env_set(ret, "=", primitive_new(assign));
    env_set(ret, "^", primitive_new(lambda));
    env_set(ret, "if", primitive_new(apply_if));
    env_set(ret, "<", primitive_new(less_than));
    env_set(ret, "-", primitive_new(subtract));
    env_set(ret, "pair?", primitive_new(is_pair));
    return ret;
}
