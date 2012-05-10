package meme

import ()

type primitive struct {
	apply func(List, Env) Node
}

func (me primitive) Apply(args List, env Env) Node {
	return me.apply(args, env)
}

func (me primitive) Special() bool {
	return false
}

func (me primitive) Eval(Env) interface{} {
	return me
}
