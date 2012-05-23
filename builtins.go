package meme

import (
	"fmt"
	"io"
	"os"
)

var builtins = map[string]Node{}

func init() {
	for name, f := range map[string]func(List, mapEnv) Evalable{
		"__define": analyzeDefine,
		"if":       analyzeIf,
		"lambda":   analyzeLambda,
		"begin":    analyzeBegin,
		"set!":     analyzeSetBang,
	} {
		builtins[name] = builtinSpecial{name, f}
	}
	for name, f := range map[string]func(List, Env) Node{
		"+":       applyPlus,
		"-":       applyMinus,
		"*":       applySplat,
		"=":       applyEqQuery,
		"<":       applyLessThan,
		"eq?":     applyEqQuery,
		"cons":    applyCons,
		"null?":   applyNullQuery,
		"car":     applyCar,
		"cdr":     applyCdr,
		"pair?":   applyPairQuery,
		"apply":   applyApply,
		"__macro": applyMacro,
		"print":   applyPrint,
	} {
		builtins[name] = &primitiveApplier{name, f}
	}
	builtins["stdout"] = &primitiveEvaler{"stdout", evalStdout, os.Stderr}
}

type primitiveEvaler struct {
	name  string
	func_ func(Env) interface{}
	value interface{}
}

func (me *primitiveEvaler) Value() interface{} {
	return me.value
}

func (me primitiveEvaler) Print(p *Print) {
	p.Atom("#(" + me.name)
	p.ListEnd()
}

func evalStdout(Env) interface{} {
	return os.Stdout
}

type builtinSpecial struct {
	name    string
	analyze func(List, mapEnv) Evalable
}

func (me builtinSpecial) Print(p *Print) {
	p.Atom(fmt.Sprintf("#(%s", me.name))
	p.SyntaxToken(ListEnd)
}

func (me builtinSpecial) Analyze(args List, env mapEnv) Evalable {
	return me.analyze(args, env)
}

func applyMacro(args List, env Env) Node {
	if args.Len() != 1 {
		panic(nil)
	}
	return Macro{args.Car().(Applicable)}
}

func applyLessThan(args List, env Env) Node {
	for {
		a := args.Car()
		args = args.Cdr()
		b := args.Car()
		if less, _ := a.(Comparable).Less(b); !less {
			return False
		}
		if args.Cdr().IsNull() {
			break
		}
	}
	return True
}

type define struct {
	name string
}

var _ Evalable = define{}

func (me define) Print(p *Print) {
	p.Atom("#(define")
	p.Atom(me.name)
	p.ListEnd()
}

func (me define) Eval(env Env) Node {
	env.(mapEnv).Define(me.name)
	return Void
}

type setVar struct {
	var_ Evalable
	exp  Evalable
}

func (me setVar) Print(p *Print) {
	p.Atom("(set!")
	me.var_.Print(p)
	me.exp.Print(p)
	p.ListEnd()
}

type setBang struct {
	var_ Node
	exp  Evalable
}

var _ Rewritable = setBang{}

func (me setBang) Print(p *Print) {
	p.Atom("#(set!")
	me.var_.Print(p)
	me.exp.Print(p)
	p.ListEnd()
}

func (me setBang) Rewrite(f RewriteFunc) Node {
	me.var_ = f(me.var_)
	me.exp = f(me.exp).(Evalable)
	return me
}

func (me setBang) Eval(env Env) Node {
	var var_ *Var
	switch data := me.var_.(type) {
	case Symbol:
		var_ = env.(mapEnv).Find(data.Value())
	case fastVar:
		var_ = data.GetVar(env.(fastEnv))
	default:
		panic(data)
	}
	var_.Set(Eval(me.exp, env))
	//log.Println(me.var_, "set to", SprintNode(var_.Get()))
	return Void
}

func analyzeSetBang(args List, env mapEnv) Evalable {
	return setBang{
		args.Car().(Symbol),
		Analyze(args.Index(1).(Parseable), env),
	}
}

func analyzeDefine(args List, env mapEnv) Evalable {
	var value Evalable
	sym := args.Car().(Symbol)
	switch args.Len() {
	case 1:
	case 2:
		value = Analyze(args.Index(1).(Parseable), env)
	default:
		panic(nil)
	}
	if value == nil {
		return define{sym.Value()}
	}
	return begin{
		define{sym.Value()},
		setBang{
			sym,
			value,
		},
	}
}

func applyMinus(args List, env Env) Node {
	if args.Cdr().IsNull() {
		return NewInt(-args.Car().(*Int).Int64())
	}
	ll := args.Car().(*Int).Int64()
	args = args.Cdr()
	for !args.IsNull() {
		ll -= args.Car().(*Int).Int64()
		args = args.Cdr()
	}
	return NewInt(ll)
}

func applyPrint(args List, _ Env) Node {
	a := make([]interface{}, 0, len(args.Cdr()))
	for _, n := range args.Cdr() {
		a = append(a, n.(interface{}))
	}
	n, err := fmt.Fprintln(args.Car().(Capsule).Value().(io.Writer), a...)
	if err != nil {
		panic(err)
	}
	return NewInt(int64(n))
}

func applySplat(args List, env Env) Node {
	v := args.Car().(*Int).Int64()
	args = args.Cdr()
	for {
		v *= args.Car().(*Int).Int64()
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

type begin []Evalable

var _ Evalable = begin{}
var _ Rewritable = begin{}

func (me begin) Print(p *Print) {
	p.Atom("#(begin")
	for _, n := range me {
		n.Print(p)
	}
	p.ListEnd()
}

func (me begin) Rewrite(f RewriteFunc) Node {
	ret := make(begin, len(me))
	for i, a := range me {
		ret[i] = f(a).(Evalable)
	}
	return ret
}

func (me begin) Eval(env Env) (ret Node) {
	for _, stmt := range me {
		ret = Eval(stmt, env)
	}
	return
}

func analyzeBegin(args List, env mapEnv) Evalable {
	ret := begin{}
	for !args.IsNull() {
		ret = append(ret, Analyze(args.Car().(Parseable), env))
		args = args.Cdr()
	}
	return ret
}

func analyzeLambda(args List, env mapEnv) Evalable {
	if args.Len() < 2 {
		panic(nil)
	}
	fixed, rest := parseFormals(args.Car())
	return lambda{
		fixed: fixed,
		rest:  rest,
		body:  analyzeBegin(args.Cdr(), env),
	}
}

type if_ struct {
	test, conseq, alt Evalable
}

var _ Rewritable = if_{}

func (me if_) Rewrite(f RewriteFunc) Node {
	me.test = f(me.test).(Evalable)
	me.conseq = f(me.conseq).(Evalable)
	if me.alt != nil {
		me.alt = f(me.alt).(Evalable)
	}
	return me
}

func (me if_) Print(p *Print) {
	p.Atom("#(if")
	me.test.Print(p)
	me.conseq.Print(p)
	if me.alt != nil {
		me.alt.Print(p)
	}
	p.ListEnd()
}

func analyzeIf(args List, env mapEnv) Evalable {
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

func applyPlus(args List, env Env) Node {
	if args.Len() < 1 {
		panic(nil)
	}
	var ll int64
	for !args.IsNull() {
		ll += args.Car().(*Int).Int64()
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
	less, err := a.Less(b)
	if less {
		return False
	}
	switch err {
	case nil:
	case TypeError:
		return False
	default:
		panic(err)
	}
	less, err = b.Less(a)
	if less {
		return False
	}
	switch err {
	case nil:
	case TypeError:
		return False
	default:
		panic(err)
	}
	return True
}

func applyApply(args List, env Env) Node {
	rest := args[len(args)-1].(List)
	newArgs := append(args[1:len(args)-1], rest...)
	return Apply(args[0].(Applicable), newArgs, env)
}

func applyPairQuery(args List, env Env) Node {
	if !args.Cdr().IsNull() {
		panic(nil)
	}
	if _, ok := args.Car().(List); ok {
		return True
	}
	return False
}
