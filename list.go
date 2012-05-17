package meme

type MapFunc func(Node) Node

var _ Parseable = List{}
var _ Comparable = List{}
var _ Rewritable = List{}

type List []Node

var Nil = List{}

func (me List) Rewrite(f RewriteFunc) Node {
	return me.Map(MapFunc(f))
}

func (me List) Print(p *Printer) {
	p.SyntaxToken(ListStart)
	for _, a := range me {
		a.Print(p)
	}
	p.SyntaxToken(ListEnd)
}

func (me List) Car() Node {
	return me[0]
}

func (me List) Cdr() List {
	return me[1:]
}

func (me List) IsNull() bool {
	return len(me) == 0
}

func (me List) Index(i int) Node {
	return me[i]
}

func (me List) Len() int {
	return len(me)
}

func (me List) Map(f MapFunc) List {
	ret := make(List, len(me))
	for i, n := range me {
		ret[i] = f(n)
	}
	return ret
}

func (me List) Eval(env Env) Node {
	proc := Eval(me[0].(Evalable), env).(Applicable)
	args := make(List, len(me)-1)
	for i, a := range me[1:] {
		args[i] = Eval(a.(Evalable), env)
	}
	return Apply(proc, args, env)
}

func Cons(a Node, b List) List {
	ret := make(List, 1+len(b))
	ret[0] = a
	if len(b) != copy(ret[1:], b) {
		panic(nil)
	}
	return ret
}

func (me List) Less(other Node) (less bool, err error) {
	if otherList, ok := other.(List); ok {
		for i := 0; i < me.Len() && i < otherList.Len(); i++ {
			if less, _ := me.Index(i).(Comparable).Less(otherList.Index(i)); !less {
				return false, nil
			}
		}
		return me.Len() < otherList.Len(), nil
	}
	return false, TypeError
}
