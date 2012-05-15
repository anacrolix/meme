package meme

type Macro struct {
	body Applicable
}

var _ Analyzer = Macro{}
var _ Special = Macro{}

func (me Macro) Analyze(args List, env Env) Evalable {
	return Analyze(Apply(me.body, args, env).(Parseable), env)
}
