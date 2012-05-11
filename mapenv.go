package meme

type MapEnv struct {
	vars  map[string]Var
	outer Env
}

func NewMapEnv(outer Env) MapEnv {
	return MapEnv{
		make(map[string]Var),
		outer,
	}
}

func (me MapEnv) Define(name string, value interface{}) {
	if _, ok := me.vars[name]; ok {
		panic(nil)
	}
	me.vars[name] = Var{value}
}

func (me MapEnv) Find(name string) interface{} {
	if var_, ok := me.vars[name]; ok {
		return var_.val
	}
	return me.outer.Find(name)
}
