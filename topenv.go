package meme

type GlobalEnv struct {
	vars map[string]*Var
}

func (me *GlobalEnv) Define(name string) *Var {
	if _, ok := me.vars[name]; ok {
		panic("already defined: " + name)
	}
	ret := &Var{}
	me.vars[name] = ret
	return ret
}

func (me *GlobalEnv) Find(name string) *Var {
	return me.vars[name]
}

func NewTopEnv() (ret *GlobalEnv) {
	ret = &GlobalEnv{
		vars: make(map[string]*Var, len(builtins)),
	}
	for k, v := range builtins {
		ret.Define(k).Set(v)
	}
	return
}
