package meme

var _ Node = Symbol{}

type Symbol struct {
	val string
}

var _ Comparable = Symbol{}

func (me Symbol) Eval(env Env) (ret Node) {
	var_ := env.(mapEnv).Find(me.val)
	if var_ == nil {
		panic("symbol not found: " + me.val)
	}
	ret = var_.Get()
	if ret == nil {
		panic("var unset: " + me.val)
	}
	return
}

func (me Symbol) Value() string {
	return me.val
}

func (me Symbol) Print(p *Print) {
	p.Atom(me.val)
}

func (me Symbol) Less(other Node) (less bool, err error) {
	if sym, ok := other.(Symbol); ok {
		return me.val < sym.val, nil
	}
	return false, TypeError
}

func newSymbol(s string) Symbol {
	return Symbol{s}
}
