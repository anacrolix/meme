package meme

type Closure struct {
    Env Env
    Func Func
}

func (Closure) Eval(Env) Node {
    panic("cannot evaluate a closure")
}

func (me Closure) Apply(args List, env Env) interface{} {
	return me.Func.Run(args, me.Env)
}

func (me Closure) Special() bool {
	return false
}
