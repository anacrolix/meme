package meme

type MapFunc func(Node) Node

var _ Parseable = List{}

type List struct {
	slice []Node
}

var Nil = List{[]Node{}}

func (me List) Print(p *Printer) {
	p.SyntaxToken(ListStart)
	for _, a := range me.slice {
		a.(Printable).Print(p)
	}
	p.SyntaxToken(ListEnd)
}

func (me List) Car() Node {
	return me.slice[0]
}

func (me List) Cdr() List {
	return List{me.slice[1:]}
}

func (me List) IsNull() bool {
	return len(me.slice) == 0
}

func (me *List) Index(i int) Node {
	return me.slice[i]
}

func (me *List) Len() int {
	return len(me.slice)
}

func (me List) Map(f MapFunc) List {
	ret := List{
		make([]Node, len(me.slice)),
	}
	for i, n := range me.slice {
		ret.slice[i] = f(n)
	}
	return ret
}

func (me List) Eval(env Env) Node {
	proc := Eval(me.slice[0].(Evalable), env).(Applicable)
	return Apply(proc, evalList(me.Cdr(), env), env)
}

func Cons(a Node, b List) List {
	ret := List{make([]Node, 1+len(b.slice))}
	ret.slice[0] = a
	if len(b.slice) != copy(ret.slice[1:], b.slice) {
		panic(nil)
	}
	return ret
}

func (me List) Less(other Node) bool {
	otherList, _ := other.(List)
	for i := 0; i < me.Len() && i < otherList.Len(); i++ {
		if !me.Index(i).(Comparable).Less(otherList.Index(i)) {
			return false
		}
	}
	return me.Len() < otherList.Len()
}

func (me List) String() string {
	return printString(me)
}
