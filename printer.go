package meme

import (
	"bytes"
	"fmt"
)

type Print struct {
	buf      *bytes.Buffer
	lastType uint
}

func NewPrint() Print {
	return Print{
		buf:      &bytes.Buffer{},
		lastType: InvalidToken,
	}
}

func (me *Print) startToken(tt uint) {
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

func (me Print) Space() {
	me.buf.WriteByte(' ')
}

func (me *Print) Atom(a interface{}) {
	me.startToken(AtomType)
	fmt.Fprint(me.buf, a)
}

func (me *Print) SyntaxToken(typ uint) {
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

func (me *Print) ListEnd() {
	me.SyntaxToken(ListEnd)
}

func (me *Print) ListStart() {
	me.SyntaxToken(ListStart)
}
