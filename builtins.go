package meme

var builtins map[string]Applicable

var specials map[string]func(List, Env)Node

func init() {
	specials = map[string]func(List, Env)Node {
		"define": applyDefine,
		"if": applyIf,
	}
	builtins = map[string]Applicable{
		"null?": NewPrimitive(applyNullQuery),
		"+": NewPrimitive(applyPlus),
		"cdr": NewPrimitive(applyCdr),
	}
}

func applyDefine(args List, env Env) Node {
	var name Symbol
	var value Node
	if fmls, ok := args.Car().(List); ok {
		name = fmls.Car().(Symbol)
		fmls = fmls.Cdr()
		var addr Node
		if fmls.Len() == 2 && fmls.Car().(Symbol).Value() == "." {
			addr = fmls.Index(1)
		} else {
			addr = fmls
		}
		value = applyLambda(NewPair(addr, args.Cdr()), env)
	} else {
		name = args.Car().(Symbol)
		switch args.Len() {
		case 1:
		case 2:
			value = args.Index(1).(Evalable).Eval(env)
		default:
			panic(nil)
		}
	}
	env.Define(name.Value(), value)
	return Void
}

func parseFormals(fmls Node) (fixed []string, rest *string) {
	if names, ok := fmls.(List); ok {
		for !names.IsNull() {
			val := names.Car().(Symbol).Value()
			if val == "." {
				tmpRest := names.Index(1).(Symbol).Value()
				rest = &tmpRest
				if names.Len() != 2 {
					panic(nil)
				}
				return
			}
			fixed = append(fixed, val)
			names = names.Cdr()
		}
		return
	}
	tmpRest := fmls.(Symbol).Value()
	rest = &tmpRest
	return
}

func applyBegin(args List, env Env) (ret Node) {
	for !args.IsNull() {
		ret = Eval(args.Car().(Evalable), env)
		args = args.Cdr()
	}
	return
}

func applyLambda(args List, env Env) Node {
	if args.Len() < 2 {
		panic(nil)
	}
	fixed, rest := parseFormals(args.Car())
	return NewClosure(NewFunc(fixed, rest, args.Cdr()), env)
}

func applyIf(args List, env Env) Node {
	if args.Len() > 3 {
		panic("too many arguments")
	}
	if Truth(Eval(args.Car().(Evalable), env)) {
		return Eval(args.Index(1).(Evalable), env)
	}
	if args.Len() == 3 {
		return Eval(args.Index(2).(Evalable), env)
	}
	return Void
}

/*
static Node *apply_car(Node *const args[], int count, Env *env) {
    if (count != 1) return NULL;
    Pair *pair = pair_check(*args);
    if (!pair) {
        fprintf(stderr, "expected a pair: ");
        node_print_file(*args, stderr);
        fputc('\n', stderr);
        return NULL;
    }
    Node *ret = pair->addr;
    // this would make the pair the nil type, which has no car
    if (ret) node_ref(ret);
    return ret;
}

*/
func applyCdr(args List, env Env) Node {
	if args.Len() != 1 {
		panic(nil)
	}
	return args.Car().(List).Cdr()
}
/*
static Node *apply_symbol_query(Node *const args[], int count, Env *env) {
    if (count != 1) return NULL;
    Node *ret = symbol_check(*args) ? true_node : false_node;
    node_ref(ret);
    return ret;
}

*/
func applyPlus(args List, env Env) Node {
	if args.Len() < 1 {
		panic(nil)
	}
	var ll int64
	for !args.IsNull() {
		ll += args.Car().(Int).Int64()
		args = args.Cdr()
	}
	return NewInt(ll)
}

func applyNullQuery(args List, env Env) Node {
	if args.Len() != 1 {
		panic(nil)
	}
	if args.Car().(List).IsNull() {
		return True
	}
	return False
}
/*

static Node *apply_cons(Node *const args[], int count, Env *env) {
    if (count != 2) return NULL;
    Pair *dec = pair_check(args[1]);
    if (!dec) return NULL;
    node_ref(args[0]);
    node_ref(dec);
    return pair_new(args[0], dec);
}

static Node *apply_eq_query(Node *const args[], int count, Env *env) {
    if (count != 2) return NULL;
    Node *ret;
    switch (node_compare(args[0], args[1])) {
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
*/

/*
// TODO test crash for (apply f)
static Node *apply_apply(Node *const args[const], int count, Env *env) {
    if (count < 2) return NULL;
    Pair *argn = pair_check(args[count-1]);
    if (!argn) return NULL;
    int argc1 = count - 2 + list_length(argn);
    Node *args1[argc1];
    memcpy(args1, args + 1, sizeof *args * (count - 2));
    Node **dest = args1 + count - 2;
    for (; !is_null(argn); argn = pair_dec(argn)) {
        *dest++ = argn->addr;
    }
    return node_apply(*args, args1, argc1, env);
}

static Node *apply_begin(Node *const args[], int count, Env *env) {
    if (count < 1) return NULL;
    node_ref(args[count-1]);
    return args[count-1];
}

static Node *apply_defined_query(Node *const args[], int count, Env *env) {
    if (count != 1) return NULL;
    Symbol *var = symbol_check(*args);
    if (!var) return NULL;
    Node *ret = env_is_defined(env, var) ? true_node : false_node;
    node_ref(ret);
    return ret;
}

static Node *apply_quote(Node *const args[], int count, Env *env) {
    if (count != 1) return NULL;
    Quote *quote = NODE_NEW(*quote);
    node_init(quote, &quote_type);
    quote->quoted = *args;
    node_ref(quote->quoted);
    return quote;
}


static Node *apply_undef(Node *const args[], int count, Env *env) {
    if (count != 1) return NULL;
    Symbol *sym = symbol_check(*args);
    if (!sym) return NULL;
    if (!env_undefine(env, sym)) return NULL;
    node_ref(void_node);
    return void_node;
}

static Node *apply_set_bang(Node *const args[], int count, Env *env) {
    if (count != 2) return NULL;
    Symbol *var = symbol_check(args[0]);
    if (!var) return NULL;
    Node *value = node_eval(args[1], env);
    if (!value) return NULL;
    if (!env_set(env, var, value)) {
        node_unref(value);
        return NULL;
    }
    node_ref(var);
    node_ref(void_node);
    return void_node;
}

typedef struct {
    char const *name;
    PrimitiveApplyFunc apply;
} PrimitiveType;

*/

/*
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
*/
