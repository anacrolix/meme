package meme

var builtins map[string]Applicable

var specials map[string]func(List, Env) Evalable

func init() {
	specials = map[string]func(List, Env) Evalable{
		"define":   analyzeDefine,
		"if":       analyzeIf,
		"lambda":   analyzeLambda,
		"begin":    analyzeBegin,
		"defmacro": analyzeMacro,
		"set!":     analyzeSetBang,
	}
	builtins = map[string]Applicable{
		"+":     NewPrimitive(applyPlus),
		"-":     NewPrimitive(applyMinus),
		"*":     NewPrimitive(applySplat),
		"=":     NewPrimitive(applyEqQuery),
		"<":     NewPrimitive(applyLessThan),
		"eq?":   NewPrimitive(applyEqQuery),
		"cons":  NewPrimitive(applyCons),
		"null?": NewPrimitive(applyNullQuery),
		"car":   NewPrimitive(applyCar),
		"cdr":   NewPrimitive(applyCdr),
		"pair?": NewPrimitive(applyPairQuery),
		"apply": NewPrimitive(applyApply),
	}
}

func analyzeMacro(args List, env Env) Evalable {
	def := analyzeDefine(args, env).(define)
	body := Eval(def.value, env).(Applicable)
	if _, ok := specials[def.name]; ok {
		panic(nil)
	}
	specials[def.name] = func(args List, env Env) Evalable {
		return Analyze(Apply(body, args, env).(Parseable), env)
	}
	return NewQuote(Void)
}

func applyLessThan(args List, env Env) Node {
	for {
		a := args.Car()
		args = args.Cdr()
		b := args.Car()
		if !a.(Comparable).Less(b) {
			return False
		}

		if args.Cdr().IsNull() {
			break
		}
	}
	return True
}

type define struct {
	name  string
	value Evalable
}

var _ Evalable = define{}

/*
func (me define) Print(p *Printer) {
	p.Atom("#(define")
	p.Atom(me.name)
	me.value.Print(p)
	p.SyntaxToken(ListEnd)
}
*/

func (me define) Eval(env Env) Node {
	env.Define(me.name, Eval(me.value, env))
	return Void
}

type setBang struct {
	define
}

func (me *setBang) Eval(env Env) Node {
	env.Set(me.name, Eval(me.value, env))
	return Void
}

func analyzeSetBang(args List, env Env) Evalable {
	return &setBang{
		define{
			args.Car().(Symbol).Value(),
			Analyze(args.Index(1).(Parseable), env),
		},
	}
}

func analyzeDefine(args List, env Env) Evalable {
	var nameSym Symbol
	var value Evalable
	if fmls, ok := args.Car().(List); ok {
		nameSym = fmls.Car().(Symbol)
		fmls = fmls.Cdr()
		var addr Node
		if fmls.Len() == 2 && fmls.Car().(Symbol).Value() == "." {
			addr = fmls.Index(1)
		} else {
			addr = fmls
		}
		value = analyzeLambda(Cons(addr, args.Cdr()), env)
	} else {
		nameSym = args.Car().(Symbol)
		switch args.Len() {
		case 1:
		case 2:
			value = Analyze(args.Index(1).(Parseable), env)
		default:
			panic(nil)
		}
	}
	return define{
		nameSym.Value(),
		value,
	}
}

func applyMinus(args List, env Env) Node {
	if args.Cdr().IsNull() {
		return NewInt(-args.Car().(Int).Int64())
	}
	ll := args.Car().(Int).Int64()
	args = args.Cdr()
	for !args.IsNull() {
		ll -= args.Car().(Int).Int64()
		args = args.Cdr()
	}
	return NewInt(ll)
}

func applySplat(args List, env Env) Node {
	v := args.Car().(Int).Int64()
	args = args.Cdr()
	for {
		v *= args.Car().(Int).Int64()
		args = args.Cdr()
		if args.IsNull() {
			break
		}
	}
	return NewInt(v)
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

type begin struct {
	exps List
}

var _ Evalable = begin{}

/*
func (me begin) Print(p *Printer) {
	p.SyntaxToken(ListStart)
	p.Atom("#(begin)")
	exps := me.exps
	for !exps.IsNull() {
		exps.Car().Print(p)
		exps = exps.Cdr()
	}
	p.SyntaxToken(ListEnd)
}
*/

func (me begin) Eval(env Env) (ret Node) {
	exps := me.exps
	for !exps.IsNull() {
		ret = Eval(exps.Car().(Evalable), env)
		exps = exps.Cdr()
	}
	return
}

func analyzeBegin(args List, env Env) Evalable {
	return begin{
		args.Map(func(a Node) Node {
			return Analyze(a.(Parseable), env)
		}),
	}
}

func analyzeLambda(args List, env Env) Evalable {
	if args.Len() < 2 {
		panic(nil)
	}
	fixed, rest := parseFormals(args.Car())
	return NewFunc(fixed, rest, analyzeBegin(args.Cdr(), env))
}

type if_ struct {
	test, conseq, alt Evalable
}

/*
func (me if_) Print(p *Printer) {
	p.Atom("#(if")
	me.test.Print(p)
	me.conseq.Print(p)
	if me.alt != nil {
		me.alt.Print(p)
	}
	p.SyntaxToken(ListEnd)
}
*/

func analyzeIf(args List, env Env) Evalable {
	var ret if_
	ret.test = Analyze(args.Car().(Parseable), env)
	args = args.Cdr()
	ret.conseq = Analyze(args.Car().(Parseable), env)
	args = args.Cdr()
	if !args.IsNull() {
		ret.alt = Analyze(args.Car().(Parseable), env)
		args = args.Cdr()
	}
	if !args.IsNull() {
		panic("too many args to if")
	}
	return ret
}

func (me if_) Eval(env Env) Node {
	if Truth(Eval(me.test, env)) {
		return Eval(me.conseq, env)
	}
	if me.alt != nil {
		return Eval(me.alt, env)
	}
	return Void
}

func applyCar(args List, env Env) Node {
	if args.Len() != 1 {
		panic(nil)
	}
	return args.Car().(List).Car()
}

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

func applyCons(args List, env Env) Node {
	if args.Len() != 2 {
		panic(nil)
	}
	return Cons(args.Car(), args.Index(1).(List))
}

func applyEqQuery(args List, env Env) Node {
	a := args.Car().(Comparable)
	args = args.Cdr()
	b := args.Car().(Comparable)
	args = args.Cdr()
	if !args.IsNull() {
		panic(nil)
	}
	if a.Less(b) || b.Less(a) {
		return False
	}
	return True
}

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
*/

func applyApply(args List, env Env) Node {
	rest := args.Index(args.Len() - 1).(List)
	newArgs := append(args.slice[1:len(args.slice)-1], rest.slice...)
	return Apply(args.Car().(Applicable), List{newArgs}, env)
}

/*
static Node *apply_defined_query(Node *const args[], int count, Env *env) {
    if (count != 1) return NULL;
    Symbol *var = symbol_check(*args);
    if (!var) return NULL;
    Node *ret = env_is_defined(env, var) ? true_node : false_node;
    node_ref(ret);
    return ret;
}

static Node *apply_undef(Node *const args[], int count, Env *env) {
    if (count != 1) return NULL;
    Symbol *sym = symbol_check(*args);
    if (!sym) return NULL;
    if (!env_undefine(env, sym)) return NULL;
    node_ref(void_node);
    return void_node;
}

static PrimitiveType primitives[] = {
    {"+", apply_plus},
    {"symbol?", apply_symbol_query},
};
*/

func applyPairQuery(args List, env Env) Node {
	if !args.Cdr().IsNull() {
		panic(nil)
	}
	if _, ok := args.Car().(List); ok {
		return True
	}
	return False
}
