package meme

import (
	"fmt"
	"io"
	"strconv"
	"strings"
	"unicode"
)

type parser struct {
	tok Token
	L   *Lexer
}

func (me *parser) advance() (err error) {
	me.tok, err = me.L.NextToken()
	return
}

func (me *parser) parseList() List {
	if err := me.advance(); err != nil {
		panic(err)
	}
	if st, ok := me.tok.(SyntaxToken); ok && st.Type == ListEnd {
		return Nil
	}
	return Cons(me.parse(), me.parseList())
}

func (me *parser) parseAtom() Parseable {
	atom := me.tok.(Atom)
	s := atom.Value.(string)
	switch atom.Type {
	case SymbolToken:
		r, _, err := strings.NewReader(s).ReadRune()
		if err != nil {
			panic(err)
		}
		switch {
		case unicode.IsDigit(r):
			i, err := strconv.ParseInt(s, 0, 64)
			if err != nil {
				panic(err)
			}
			return NewInt(i)
		case s == "#t":
			return True
		case s == "#f":
			return False
		}
		return newSymbol(s)
	case StringToken:
		return newString(s)
	}
	panic(nil)
}

func (me *parser) parse() Parseable {
	if _, ok := me.tok.(Atom); ok {
		return me.parseAtom()
	}
	syntaxType := me.tok.(SyntaxToken).Type
	switch syntaxType {
	case ListStart:
		return me.parseList()
	case QuoteType:
		if err := me.advance(); err != nil {
			panic(err)
		}
		return NewQuote(me.parse())
	default:
		panic(fmt.Sprint(syntaxType))
	}
	panic(me.tok)
}

func Parse(l *Lexer) Parseable {
	p := parser{
		L: l,
	}
	if err := p.advance(); err != nil {
		if err != io.EOF {
			panic(err)
		}
		return nil
	}
	return p.parse()
}
