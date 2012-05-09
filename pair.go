package meme

type Pair struct {
	addr interface{}
	dec  List
}

func (me Pair) Car() interface{} {
	return me.addr
}

func (me Pair) Cdr() List {
	return me.dec
}

func (me Pair) IsNull() bool {
	return false
}

func NewPair(addr interface{}, dec List) Pair {
	return Pair{
		addr: addr,
		dec:  dec,
	}
}

func evalList(list List, env Env) List {
	if list.IsNull() {
		return list
	}
	return NewPair(list.Car().(Evalable).Eval(env), evalList(list.Cdr(), env))
}

func (me Pair) Eval(env Env) interface{} {
	proc := me.Car().(Evalable).Eval(env).(Applicable)
	args := me.Cdr()
	if !proc.Special() {
		args = evalList(args, env)
	}
	return proc.Apply(args, env)
}

func (me Pair) Print(p *Printer) {
	p.SyntaxToken(ListStart)
	var list List = me
	for !list.IsNull() {
		list.Car().(Printable).Print(p)
		list = list.Cdr()
	}
	p.SyntaxToken(ListEnd)
}

func (me Pair) String() string {
	return printString(me)
}

func (me Pair) Index(i uint) interface{} {
	if i == 0 {
		return me.Car()
	}
	return me.Cdr().Index(i - 1)
}

func (me Pair) Len() uint {
	if me.IsNull() {
		return 0
	}
	return 1 + me.Cdr().Len()
}

func (me Pair) Map(f func(interface{}) interface{}) List {
	return NewPair(f(me.Car()), me.Cdr().Map(f))
}

func (me Pair) Analyze(env Env) Evalable {
	proc := me.Car().(Node).Analyze(nil, env)
	if anal, ok := proc.(Analyzer); ok {
		return anal.Analyze(me.Cdr(), env).(Evalable)
	}
	/*
	if macro, ok := procVar.Eval(nil).(Macro); ok {
		return macro.Apply(me.Cdr(), env).(Evalable)
	}
	*/

	return NewPair(proc, me.Cdr().Map(func(arg interface{}) interface{} {
		return arg.(Node).Analyze(nil, env)
	}))
}
