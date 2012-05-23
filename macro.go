package meme

type Macro struct {
	body Applicable
}

var _ Analyzer = Macro{}
var _ Special = Macro{}

func (me Macro) Print(p *Print) {
	p.Atom("#(macro")
	me.body.Print(p)
	p.ListEnd()
}

func (me Macro) Analyze(args List, env mapEnv) Evalable {
	return Analyze(Apply(me.body, args, env).(Parseable), env)
}
