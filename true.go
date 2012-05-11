package meme

type trueType struct{}

func (me trueType) Apply([]Node, Env) Node {
	panic("true is not an operator")
}

func (me trueType) Eval(Env) interface{} {
	return me
}

func (trueType) Expand(Env) Parseable {
	return True
}

func (trueType) Print(p *Printer) {
	p.Atom("#t")
}

func (trueType) String() string {
	return printString(True)
}

var True trueType
