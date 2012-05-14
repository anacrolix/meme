package meme

type Closure struct {
	Env  Env
	Func *Func
}

var _ Applicable = &Closure{}

func (me *Closure) Apply(args List, env Env) Node {
	return me.Func.Run(args, me.Env)
}

func NewClosure(func_ *Func, env Env) *Closure {
	return &Closure{
		Env:  env,
		Func: func_,
	}
}
