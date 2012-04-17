#include "meme.h"
#include <assert.h>
#include <string.h>

Node *multiply(Node *optr, Node *args[], int count, Env *env) {
    if (count < 2) abort();
    Int *opnd = (Int *)eval(args[0], env);
    if (!opnd) return NULL;
    if (opnd->type != &int_type) {
        node_unref(opnd);
        return NULL;
    }
    Int *ret = int_new(opnd->ll);
    node_unref(opnd);
    for (int i = 1; i < count; i++) {
        opnd = (Int *)eval(args[i], env);
        if (opnd->type != &int_type) abort();
        ret->ll *= opnd->ll;
        node_unref(opnd);
    }
    return ret;
}

Node *assign(Node *proc, Node *args[], int count, Env *env) {
    if (count != 2) return NULL;
    char const *key = var_get_name((Var *)args[0]);
    Node *value = eval(args[1], env);
    if (!value) return NULL;
    env_set(env, key, value);
    node_ref(true_node);
    return true_node;
}

typedef struct {
    Node;
    Node *expr;
    Env *env;
    int parmc;
    Var *parms[0]; 
} Closure;

void closure_print(Node const *_n, Printer *p) {
    Closure *n = (Closure *)_n;
    fprintf(stderr, "<closure ");
    node_print(n->expr, p);
    fputc('>', stderr);
}

static Node *closure_call(Node *_optr, Node *args[], int argc, Env *env) {
    Closure *optr = (Closure *)_optr;
    if (optr->parmc != argc) return NULL;
    Env *call_env = env_new(optr->env);
    for (int i = 0; i < argc; i++) {
        Node *v = eval(args[i], env);
        if (!v) {
            node_unref(call_env);
            return NULL;
        }
        env_set(call_env, var_get_name(optr->parms[i]), v);
    }
    Node *ret = eval(optr->expr, call_env);
    node_unref(call_env);
    return ret;
}

Type const closure_type = {
    .name = "closure",
    .call = closure_call,
    .print = closure_print,
};

Node *lambda(Node *proc, Node *args[], int count, Env *env) {
    for (int i = 0; i < count - 1; i++) {
        if (args[i]->type != &var_type) return NULL;
    }
    Closure *ret = malloc(sizeof *ret + (count - 1) * sizeof *ret->parms);
    node_init(ret, &closure_type);
    ret->env = env;
    node_ref(env);
    ret->parmc = count - 1;
    for (int i = 0; i < count - 1; i++) {
        if (args[i]->type != &var_type) abort();
        ret->parms[i] = (Var *)args[i];
        node_ref(args[i]);
    }
    ret->expr = args[count - 1];
    node_ref(ret->expr);
    return ret;
}

Node *if_form(Node *proc, Node *args[], int count, Env *env) {
    if (count != 3) {
        fprintf(stderr, "expected 3 arguments\n");
        return NULL;
    }
    Node *test = eval(args[0], env);
    if (!test) return NULL;
    int truth = node_truth(test);
    node_unref(test);
    if (truth < 0) return NULL;
    Node *ret = truth ? args[1] : args[2];
    node_ref(ret);
    return ret;
}

static Int *eval_to_int(Node *node, Env *env) {
    Int *ret = (Int *)eval(node, env);
    if (!ret) return NULL;
    if (ret->type != &int_type) {
        fprintf(stderr, "not an int: ");
        node_print(ret, &(Printer){.file=stderr});
        fputc('\n', stderr);
        node_unref(ret);
        return NULL;
    }
    return ret;
}

Node *subtract(Node *proc, Node *argexps[], int nargs, Env *env) {
    if (nargs == 0) {
        fprintf(stderr, "expected one or more arguments");
        return NULL;
    }
    Int *opnd = eval_to_int(argexps[0], env);
    if (!opnd) return NULL;
    if (nargs == 1) {
        Int *ret = int_new(-opnd->ll);
        node_unref(opnd);
        return ret;
    }
    Int *ret = int_new(opnd->ll);
    node_unref(opnd);
    for (int i = 1; i < nargs; i++) {
        opnd = eval_to_int(argexps[i], env);
        if (!opnd) {
            node_unref(ret);
            return NULL;
        }
        ret->ll -= opnd->ll;
        node_unref(opnd);
    }
    return ret;
}

Node *less_than(Node *proc, Node *args[], int count, Env *env) {
    if (count != 2) return NULL;
    Node *ret = NULL;
    Int *opnds[count];
    memset(opnds, 0, sizeof opnds);
    for (int i = 0; i < count; i++) {
        opnds[i] = (Int *)eval(args[i], env);
        if (!opnds[i]) goto done;
        if (opnds[i]->type != &int_type) goto done;
    }
    ret = (opnds[0]->ll < opnds[1]->ll) ? true_node : false_node;
    node_ref(ret);
done:
    for (int i = 0; i < count; i++) node_unref(opnds[i]);
    return ret;
}

typedef struct {
    Node;
    CallFunc call;
} Special;

extern Type const special_type;

Node *special_call(Node *_func, Node *args[], int count, Env *env) {
    assert(_func->type == &special_type);
    Special *func = (Special *)_func;
    return func->call(func, args, count, env);
}

static void special_print(Node const *_n, Printer *p) {
    fprintf(p->file, "<%p>", _n);
}

Type const special_type = {
    .name = "special",
    .call = special_call,
    .print = special_print,
};

Special *special_new(CallFunc call) {
    Special *ret = malloc(sizeof *ret);
    node_init(ret, &special_type);
    ret->call = call;
    return ret;
}

Env *top_env_new() {
    Env *ret = env_new(NULL);
    env_set(ret, "*", special_new(multiply));
    env_set(ret, "=", special_new(assign));
    env_set(ret, "^", special_new(lambda));
    env_set(ret, "if", special_new(if_form));
    env_set(ret, "<", special_new(less_than));
    env_set(ret, "-", special_new(subtract));
    return ret;
}
