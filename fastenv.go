package meme

type fastEnv struct {
	vars []*Var
    outer Env
	func_ *Func
}

func (me fastEnv) SetFast(func_ *Func, index int, value interface{}) {
	if me.func_ == func_ {
		me.vars[index].Set(value)
	} else {
		me.outer.SetFast(func_, index, value)
	}
}

func (me fastEnv) Define(name string, value interface{}) {
}

func (me fastEnv) Find(name string) interface{} {
	panic("tried to find by name in fastenv")
}

func (me fastEnv) FindVar(name string) *Var {
	panic("tried by name in fastenv")
}

func NewFastEnv(outer Env, locals int) Env {
	return fastEnv{
		vars: make([]*Var, locals),
		outer: outer,
	}
}
