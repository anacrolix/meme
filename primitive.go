package meme

type primitiveApplierFunc func(List, Env) Node

type primitiveApplier struct {
	name  string
	func_ primitiveApplierFunc
}

var _ Applicable = &primitiveApplier{}

func (me *primitiveApplier) Apply(args List, env Env) Node {
	return me.func_(args, env)
}

func (me *primitiveApplier) Print(p *Print) {
	p.Atom("#(" + me.name)
	p.ListEnd()
}
