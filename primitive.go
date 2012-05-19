package meme

type PrimitiveApplyFunc func(List, Env) Node

type primitive struct {
	name  string
	apply PrimitiveApplyFunc
}

var _ Applicable = primitive{}

func (me primitive) Apply(args List, env Env) Node {
	return me.apply(args, env)
}

func (me primitive) Eval(Env) interface{} {
	return me
}

func (me primitive) Print(p *Printer) {
	p.Atom("#(" + me.name)
	p.ListEnd()
}

func NewPrimitive(apply PrimitiveApplyFunc) primitive {
	return primitive{
		apply: apply,
	}
}
