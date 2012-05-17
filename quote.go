package meme

type Quote struct {
	quoted Node
}

func (me Quote) Eval(env Env) Node {
	return me.quoted
}

func NewQuote(node Node) Quote {
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

func (me Quote) Expand(Env) Parseable {
	return me
}
