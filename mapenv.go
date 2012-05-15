package meme

type MapEnv struct {
	vars  map[string]*Var
	outer Env
}

func NewMapEnv(outer Env) *MapEnv {
	return &MapEnv{
		make(map[string]*Var),
		outer,
	}
}

func (me *MapEnv) Define(name string) *Var {
	if _, ok := me.vars[name]; ok {
		panic("already defined: " + name)
	}
	ret := &Var{}
	me.vars[name] = ret
	return ret
}

func (me *MapEnv) Find(name string) *Var {
	if ret, ok := me.vars[name]; ok {
		return ret
	}
	return me.outer.Find(name)
}
