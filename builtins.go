package meme

var builtins map[string]Applicable

func init() {
	builtins = map[string]Applicable{
		"*": primitive{
			apply: func(args List, env Env) interface{} {
				opnd := args.Car().(Int)
				val := opnd.Int64()
				args = args.Cdr()
				for ; !args.IsNull(); args = args.Cdr() {
					val *= args.Car().(Int).Int64()
				}
				return NewInt(val)
			},
		},
		"__define": primitive{
			apply: func (args List, env Env) interface{} {
				var value interface{}
				switch args.Len() {
				case 2:
					value = args.Index(1)
				case 1:
				default:
					panic("invalid argument count")
				}
				env.Define(args.Car().(Symbol).Value(), value)
				return Void
			},
			special: true,
		},
	}
}

/*
func parseFormals(names []Node) (fixed []string, rest *string) {
	for i := 0; i < len(names); i++ {
		value := names[i].(Symbol).Value()
		if value == "." {
			*rest = names[i+1].(Symbol).Value()
			if i+2 != len(names) {
				panic("excess variadic names")
			}
			return
		}
		fixed = append(fixed, value)
	}
	return
}

func applyLambda(args []Node, env Env) interface{} {
	if len(args) > 2 {
		panic("invalid argument count")
	}
	fixed, rest := parseFormals(args)
	return Func{
		Body:          args[1].(Evalable),
		FixedVarNames: fixed,
		RestVarName:   rest,
	}
}

func applyIf(args []interface{}, env Env) interface{} {
	if len(args) > 3 {
		panic("too many arguments")
	}
	if Truth(args[0].(Evalable).Eval(env)) {
		return args[1].(Evalable).Eval(env)
	}
	return args[2].(Evalable).Eval(env)
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

static Node *apply_cdr(Node *const args[], int count, Env *env) {
    if (count != 1) return NULL;
    Pair *pair = pair_check(*args);
    if (!pair || pair == nil_node) return NULL;
    Pair *ret = pair->dec;
    node_ref(ret);
    return ret;
}

static Node *apply_symbol_query(Node *const args[], int count, Env *env) {
    if (count != 1) return NULL;
    Node *ret = symbol_check(*args) ? true_node : false_node;
    node_ref(ret);
    return ret;
}

static Node *apply_plus(Node *const args[], int count, Env *env) {
    if (count < 1) return NULL;
    long long ll = 0;
    for (; count; args++, count--) {
        Int *opnd = int_check(*args);
        if (!opnd) return NULL;
        ll += opnd->ll;
    }
    return int_new(ll);
}

static Node *apply_null_query(Node *const args[], int count, Env *env) {
    if (count != 1) return NULL;
    Pair *pair = pair_check(*args);
    if (!pair) return NULL;
    Node *ret = pair->addr ? false_node : true_node;
    node_ref(ret);
    return ret;
}

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
