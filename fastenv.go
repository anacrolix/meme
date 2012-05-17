package meme

type FastEnv struct {
	vars    []*Var
	outer   Env
	closure *Closure
}

func NewFastEnv(outer Env, locals int, closure *Closure) FastEnv {
	ret := FastEnv{
		make([]*Var, locals),
		outer,
		closure,
	}
	return ret
}

func (me FastEnv) Define(name string) *Var {
	panic("can't define in fast env: " + name)
}

func (me FastEnv) Find(name string) *Var {
	if index, ok := me.closure.names[name]; ok {
		return me.vars[index]
	}
	return me.outer.Find(name)
}
