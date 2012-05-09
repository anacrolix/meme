package meme

type falseType struct{}

var False falseType

func (me falseType) Eval(Env) interface{} {
    return me
}

func (falseType) Print(p *Printer) {
	p.Atom("#f")
}

func (falseType) Analyze(Env) interface{} {
	return False
}

func (falseType) String() string {
	return printString(False)
}
