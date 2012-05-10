package meme

var _ Node = Symbol{}

type Symbol struct {
    val string
}

func (me Symbol) Apply([]Node, Env) Node {
    panic("symbols are not operators")
}

func (me Symbol) Eval(env Env) (ret interface{}) {
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

func (me Symbol) Analyze(env Env, func_ *Func) Evalable {
	if func_ != nil {
		fv := func_.NewFastVar(me.val)
		if fv != nil {
			return fv
		}
	}
	return env.FindVar(me.val)
}

func (me Symbol) Expand(Env) Parseable {
	return me
}
