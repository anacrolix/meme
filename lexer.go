package meme

import (
    "bufio"
    "unicode"
)

type Token interface {
    Line() uint
    Col() uint
}

type TokenStruct struct {
    line, col uint
}

func (me TokenStruct) Line() uint {
    return me.line
}

func (me TokenStruct) Col() uint {
    return me.col
}

type Atom struct {
    TokenStruct
    Value string
}

type SyntaxToken struct {
    TokenStruct
    Type uint
}

const (
	InvalidToken = iota
    ListStart
    ListEnd
    QuoteType
	AtomType
)

type Lexer struct {
    R *bufio.Reader
    last rune
    Line, Col uint
    token *Token
}

func (me *Lexer) readRune() (r rune, err error) {
    r, _, err = me.R.ReadRune()
    if err == nil {
        return
    }
    switch me.last {
    case '\n':
        me.Line++
        me.Col = 1
    case -1:
    default:
        me.Col++
    }
    me.last = r
    return
}

func (me *Lexer) unreadRune() {
    me.R.UnreadRune()
}

func (me *Lexer) readAtom() (ret string) {
read:
    for {
        c, err := me.readRune()
        if err != nil {
            panic(err)
        }
        switch c {
        case ' ', '\t', '(', ')', '\r', '\n':
            me.unreadRune()
            break read
        }
        ret += string(c)
    }
    return
}

func (me *Lexer) discardLine() {
    for {
        c, err := me.readRune()
        if err != nil {
            panic(err)
        }
        if c == '\n' {
            break
        }
    }
}

func (me *Lexer) discardWhitespace() {
    for {
        c, err := me.readRune()
        if err != nil {
            panic(err)
        }
        if !unicode.IsSpace(c) {
            me.unreadRune()
            return
        }
    }
}

func (me *Lexer) newToken() TokenStruct {
    return TokenStruct{
        line: me.Line,
        col: me.Col,
    }
}

func (me *Lexer) newSyntaxToken(typ uint) SyntaxToken {
    return SyntaxToken{
        TokenStruct: me.newToken(),
        Type: typ,
    }
}

func (me *Lexer) newAtom(val string) Atom {
    return Atom{
        TokenStruct: me.newToken(),
        Value: val,
    }
}

func (me *Lexer) NextToken() Token {
    me.discardWhitespace()
    c, err := me.readRune()
    if err != nil {
        panic(err)
    }
    switch c {
    case '(':
        return me.newSyntaxToken(ListStart)
    case ')':
        return me.newSyntaxToken(ListEnd)
    case '\'':
        return me.newSyntaxToken(QuoteType)
    }
    me.unreadRune()
    return me.newAtom(me.readAtom())
}

func NewLexer(r *bufio.Reader) Lexer {
    return Lexer{
        R: r,
        last: -1,
        Line: 1,
        Col: 1,
    }
}
