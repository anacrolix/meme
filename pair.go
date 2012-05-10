package meme

import (
	"fmt"
)

type Pair struct {
	addr Node
	dec  List
}

func (me Pair) Car() Node {
	return me.addr
}

func (me Pair) Cdr() List {
	return me.dec
}

func (me Pair) IsNull() bool {
	return false
}

func NewPair(addr Node, dec List) Pair {
	return Pair{
		addr: addr,
		dec:  dec,
	}
}

func evalList(list List, env Env) List {
	if list.IsNull() {
		return list
	}
	return NewPair(list.Car().(Evaler).Eval(env), evalList(list.Cdr(), env))
}

func (me Pair) Eval(env Env) interface{} {
	proc := Eval(me.Car().(Evalable), env).(Applier)
	return proc.Apply(me.Cdr(), env)
}

func (me Pair) Print(p *Printer) {
	p.SyntaxToken(ListStart)
	var list List = me
	for !list.IsNull() {
		if canPrint, ok := list.Car().(Printable); ok {
			canPrint.Print(p)
		} else {
			p.Atom(fmt.Sprint(list.Car()))
		}
		list = list.Cdr()
	}
	p.SyntaxToken(ListEnd)
}

func (me Pair) String() string {
	return printString(me)
}

func (me Pair) Index(i uint) Node {
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

func (me Pair) Map(f MapFunc) List {
	return NewPair(f(me.Car()), me.Cdr().Map(f))
}
