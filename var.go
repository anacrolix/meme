package meme

type Var struct {
	val Node
}

func (me *Var) Get() Node {
	if me.val == nil {
		panic("unset var")
	}
	return me.val
}

func (me *Var) Set(val Node) {
	me.val = val
}
