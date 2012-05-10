package meme 

type FastVar struct {
	func_ *Func
	index int
}

func (me FastVar) Eval(env Env) interface{} {
	return env.FindFast(me.func_, me.index).Eval(nil)
}

