package meme

import (
	"fmt"
)

type Applier interface {
	Apply(List, Env) Node
}

type Applicable Applier

type Expandable interface {
	Expand(Env) Parseable
}

type Evalable interface {
	Node
	Eval(Env) Node
}

type Parseable interface {
	Evalable
	Printable
}

type Comparable interface {
	Node
	Less(Node) (bool, error)
}

type Printable interface {
	Print(*Printer)
	fmt.Stringer
}

type Node interface {
}

func printString(p Printable) string {
	np := NewPrinter()
	p.Print(&np)
	return np.buf.String()
}
