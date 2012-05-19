package meme

type TopEnv struct {
	vars map[string]*Var
}

func (me *TopEnv) Define(name string) *Var {
	if _, ok := me.vars[name]; ok {
		panic("already defined: " + name)
	}
	ret := &Var{name: name}
	me.vars[name] = ret
	return ret
}

func (me *TopEnv) Find(name string) (ret *Var) {
	return me.vars[name]
}

func NewTopEnv() (ret *TopEnv) {
	ret = &TopEnv{
		vars: make(map[string]*Var, len(builtins)),
	}
	for k, v := range builtins {
		ret.Define(k).Set(v)
	}
	return
}
