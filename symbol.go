package meme

type Symbol struct {
    val string
}

func (me Symbol) Apply([]Node, Env) Node {
    panic("symbols are not operators")
}

func (me Symbol) Eval(env Env) interface{} {
    return env.Find(me.val)
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

func (me Symbol) Analyze(env Env) Evalable {
	return env.FindVar(me.val)
}

