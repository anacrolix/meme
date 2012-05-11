package meme

type Applier interface {
	Apply(List, Env) Node
}

type Applicable Applier

type Expandable interface {
	Expand(Env) Parseable
}

type Evaler interface {
	Eval(Env) interface{}
}

type Parseable interface {
	Printable
	Evalable
}

type Evalable Evaler

type Printable interface {
	Print(*Printer)
}

type Node interface {
}

func printString(p Printable) string {
	np := NewPrinter()
	p.Print(&np)
	return np.buf.String()
}
