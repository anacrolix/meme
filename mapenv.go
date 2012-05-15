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

func (me *MapEnv) Define(name string, value Node) {
	if _, ok := me.vars[name]; ok {
		panic(nil)
	}
	me.vars[name] = &Var{value}
}

func (me *MapEnv) Find(name string) Node {
	if var_, ok := me.vars[name]; ok {
		return var_.val
	}
	return me.outer.Find(name)
}

func (me *MapEnv) Set(name string, value Node) {
	if var_, ok := me.vars[name]; ok {
		var_.Set(value)
		return
	}
	me.outer.Set(name, value)
}
