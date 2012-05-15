package meme

var _ Node = Symbol{}

type Symbol struct {
	val string
}

var _ Comparable = Symbol{}

func (me Symbol) Apply([]Node, Env) Node {
	panic("symbols are not operators")
}

func (me Symbol) Eval(env Env) (ret Node) {
	ret = env.Find(me.val)
	if ret == nil {
		panic("symbol not found: " + me.val)
	}
	return
}

func (me Symbol) Value() string {
	return me.val
}

func NewSymbol(s string) Symbol {
	return Symbol{
		val: s,
	}
}

func (me Symbol) String() string {
	return printString(me)
}

func (me Symbol) Print(p *Printer) {
	p.Atom(me.val)
}

func (me Symbol) Less(other Node) (less bool, err error) {
	if sym, ok := other.(Symbol); ok {
		return me.val < sym.val, nil
	}
	return false, TypeError
}
