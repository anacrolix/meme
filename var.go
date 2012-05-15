package meme

type Var struct {
	val interface{}
}

func (me *Var) Eval(Env) Node {
	if me.val == nil {
		panic("unset var")
	}
	return me.val
}

func (me *Var) Set(val interface{}) {
	me.val = val
}
