package meme

import (
	"bytes"
	"fmt"
)

type Printer struct {
	buf      *bytes.Buffer
	lastType uint
}

func NewPrinter() Printer {
	return Printer{
		buf:      &bytes.Buffer{},
		lastType: InvalidToken,
	}
}

func (me *Printer) startToken(tt uint) {
	switch me.lastType {
	case InvalidToken, ListStart, QuoteType:
	case ListEnd, AtomType:
		if tt != ListEnd {
			me.Space()
		}
	default:
		me.Space()
	}
	me.lastType = tt
}

func (me Printer) Space() {
	me.buf.WriteByte(' ')
}

func (me *Printer) Atom(a interface{}) {
	me.startToken(AtomType)
	fmt.Fprint(me.buf, a)
}

func (me *Printer) SyntaxToken(typ uint) {
	me.startToken(typ)
	switch typ {
	case ListStart:
		me.buf.WriteByte('(')
	case ListEnd:
		me.buf.WriteByte(')')
	case QuoteType:
		me.buf.WriteByte('\'')
	default:
		panic(typ)
	}
}

func (me *Printer) ListEnd() {
	me.SyntaxToken(ListEnd)
}

func (me *Printer) ListStart() {
	me.SyntaxToken(ListStart)
}
