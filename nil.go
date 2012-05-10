package meme

type nilType struct{}

var Nil nilType

func (nilType) Eval(Env) interface{} {
	panic("nope")
}

func (nilType) Car() Node {
	panic("car '()")
}

func (nilType) Cdr() List {
	panic("cdr '()")
}

func (nilType) IsNull() bool {
	return true
}

func (nilType) Index(uint) Node {
	panic("index exceeds list length")
}

func (nilType) Len() uint {
	return 0
}

func (nilType) Map(MapFunc) List {
	return Nil
}

func (nilType) Print(p *Printer) {
	p.SyntaxToken(ListStart)
	p.SyntaxToken(ListEnd)
}

func (nilType) String() string {
	return printString(Nil)
}

func (nilType) Analyze(Env, *Func) Evalable {
	return Nil
}

func (nilType) Expand(Env) Parseable {
	return Nil
}
