package meme

type Var struct {
	name string
	val  Node
}

var _ Evalable = &Var{}

func (me *Var) Eval(Env) Node {
	if me.val == nil {
		panic("var unset")
	}
	return me.val
}

func (me *Var) Get() Node {
	if me.val == nil {
		panic("unset var")
	}
	return me.val
}

func (me *Var) Set(val Node) {
	me.val = val
}

func (me *Var) Print(p *Printer) {
	p.Atom(me.name)
}
