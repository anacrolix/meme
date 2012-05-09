package meme

import (
    "unicode"
    "strconv"
    "strings"
)

type parser struct {
    tok Token
    L *Lexer
}

func (me *parser) advance() {
    me.tok = me.L.NextToken()
}

func (me *parser) parseList() List {
    me.advance()
    if me.tok == nil {
        panic("unterminated list")
    }
	if st, ok := me.tok.(SyntaxToken); ok && st.Type == ListEnd {
		return Nil
    }
    return NewPair(me.parse(), me.parseList())
}

func (me *parser) parseAtom() Printable {
    s := me.tok.(Atom).Value
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
    return NewSymbol(s)
}

func (me *parser) parse() Printable {
    if _, ok := me.tok.(Atom); ok {
        return me.parseAtom()
    }
    syntaxType := me.tok.(SyntaxToken).Type
    switch syntaxType {
    case ListStart:
        return me.parseList()
    case QuoteType:
        me.advance()
        return NewQuote(me.parse())
    }
    panic("syntax error")
}

func Parse(l *Lexer) Printable {
    p := parser{
        L: l,
    }
	p.advance()
    return p.parse()
}
