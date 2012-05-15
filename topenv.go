package meme

type GlobalEnv struct {
	vars map[string]*Var
}

func (me *GlobalEnv) Define(name string, value Node) {
	if _, ok := me.vars[name]; ok {
		panic("already defined: " + name)
	}
	me.vars[name] = &Var{value}
}

func (me *GlobalEnv) Find(name string) Node {
	if var_, ok := me.vars[name]; ok {
		return var_.val
	}
	panic("not defined: " + name)
}

func (me *GlobalEnv) Set(name string, value Node) {
	if var_, ok := me.vars[name]; ok {
		var_.Set(value)
		return
	}
	panic(nil)
}

func NewTopEnv() (ret *GlobalEnv) {
	ret = &GlobalEnv{
		vars: make(map[string]*Var, len(builtins)),
	}
	for k, v := range builtins {
		ret.Define(k, v)
	}
	return
}
