package meme

import (
	"fmt"
)

var _ Node = String{}

type String struct {
	val string
}

var _ Comparable = String{}

func (me String) Eval(Env) Node {
	return me
}

func (me String) Value() string {
	return me.val
}

func (me String) Print(p *Print) {
	p.Atom(fmt.Sprintf("\"%s\"", me.val))
}

func (me String) Less(other Node) (less bool, err error) {
	if sym, ok := other.(String); ok {
		return me.val < sym.val, nil
	}
	return false, TypeError
}

func newString(s string) String {
	return String{s}
}

func (me String) String() string {
	return me.val
}
