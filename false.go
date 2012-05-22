package meme

type falseType struct{}

var False falseType

func (me falseType) Eval(Env) Node {
	return me
}

func (falseType) Print(p *Print) {
	p.Atom("#f")
}

func (falseType) Expand(Env) Parseable {
	return False
}
