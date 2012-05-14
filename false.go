package meme

type falseType struct{}

var False falseType

func (me falseType) Eval(Env) Node {
	return me
}

func (falseType) Print(p *Printer) {
	p.Atom("#f")
}

func (falseType) Expand(Env) Parseable {
	return False
}

func (falseType) String() string {
	return printString(False)
}
