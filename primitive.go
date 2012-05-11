package meme

type PrimitiveApplyFunc func(List, Env) Node

type primitive struct {
	apply PrimitiveApplyFunc
}

var _ Applicable = primitive{}

func (me primitive) Apply(args List, env Env) Node {
	return me.apply(evalList(args, env), env)
}

func (me primitive) Eval(Env) interface{} {
	return me
}

func NewPrimitive(apply PrimitiveApplyFunc) primitive {
	return primitive{
		apply: apply,
	}
}
