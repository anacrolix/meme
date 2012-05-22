package meme

type trueType struct{}

func (me trueType) Apply([]Node, Env) Node {
	panic("true is not an operator")
}

func (me trueType) Eval(Env) Node {
	return me
}

func (trueType) Expand(Env) Parseable {
	return True
}

func (trueType) Print(p *Print) {
	p.Atom("#t")
}

var True trueType
