package meme

import (
	"fmt"
)

type Evalable interface {
    Eval(Env) interface{}
}

type Printable interface {
	fmt.Stringer
	Print(*Printer)
}

type Analyzer interface {
	Analyze(List, Env) Evalable
}

type Node interface{
	Evalable
	Printable
	Analyzer
}

type NodeBase struct{}


func printString(p Printable) string {
	np := NewPrinter()
	p.Print(&np)
	return np.buf.String()
}
