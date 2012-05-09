package meme

type primitive struct {
	apply func(List, Env) interface{}
	special bool
}

func (me primitive) Apply(args List, env Env) interface{} {
	return me.apply(args, env)
}

func (me primitive) Special() bool {
	return me.special
}
