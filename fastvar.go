package meme

type fastVar struct {
	func_      func_
	ups, index int
}

func (me fastVar) GetVar(fe fastEnv) *Var {
	for ups := me.ups; ups != 0; ups-- {
		fe = *fe.outer
	}
	return fe.vars[me.index]
}

func (me fastVar) Eval(env Env) Node {
	return me.GetVar(env.(fastEnv)).Get()
}

func (me fastVar) Print(p *Print) {
	func_ := me.func_
	for ups := me.ups; ups != 0; ups-- {
		func_ = *func_.parent
	}
	p.Atom(func_.locals[me.index])
}
