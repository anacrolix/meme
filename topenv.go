package meme

type GlobalEnv struct {
	vars map[string]Var
}

func (me *GlobalEnv) Define(name string, value interface{}) {
	if _, ok := me.vars[name]; ok {
		panic("already defined: " + name)
	}
	me.vars[name] = Var{value}
}

func (me GlobalEnv) Find(name string) interface{} {
	return me.vars[name].val
}

func (me GlobalEnv) FindVar(name string) *Var {
	if ret, ok := me.vars[name]; ok {
		return &ret
	}
	panic("undefined: " + name)
}

func (me GlobalEnv) FindFast(*Func, int) *Var {
	panic("can't find fast in global env")
}

func NewTopEnv() (ret *GlobalEnv) {
	ret = &GlobalEnv{
		vars: make(map[string]Var, len(builtins)),
	}
	for k, v := range builtins {
		ret.Define(k, v)
	}
	return
}

func (GlobalEnv) SetFast(*Func, int, interface{}) {
	panic("can't setfast on globalenv")
}
