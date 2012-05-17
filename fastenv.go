package meme

type FastEnv struct {
	vars  map[string]*Var
	outer Env
}

func NewFastEnv(outer Env) FastEnv {
	return FastEnv{
		make(map[string]*Var),
		outer,
	}
}

func (me FastEnv) define(name string) *Var {
	if _, ok := me.vars[name]; ok {
		panic("already defined: " + name)
	}
	ret := &Var{}
	me.vars[name] = ret
	return ret
}

func (me FastEnv) Define(name string) *Var {
	panic("can't define in fast env: " + name)
}

func (me FastEnv) Find(name string) *Var {
	if ret, ok := me.vars[name]; ok {
		return ret
	}
	return me.outer.Find(name)
}
