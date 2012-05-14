package meme

type Var struct {
	val interface{}
}

func (me *Var) Eval(Env) interface{} {
	return me.val
}

func (me *Var) Set(val interface{}) {
	me.val = val
}
