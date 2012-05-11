package meme

type Quote struct {
	quoted interface{}
}

func (me Quote) Eval(env Env) interface{} {
	return me.quoted
}

func NewQuote(node interface{}) Quote {
	return Quote{
		quoted: node,
	}
}

func (me Quote) Analyze(Env, *Func) Evalable {
	return me
}

func (me Quote) Print(p *Printer) {
	p.SyntaxToken(QuoteType)
	me.quoted.(Printable).Print(p)
}

func (me Quote) String() string {
	return printString(me)
}

func (me Quote) Expand(Env) Parseable {
	return me
}
