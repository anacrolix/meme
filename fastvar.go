package meme

import "fmt"

type fastVar struct {
	closure *Closure
	index int
}

func (me fastVar) Eval(env Env) Node {
	fastEnv := env.(FastEnv)
	return fastEnv.vars[me.index].Get()
}

func (me fastVar) Print(p *Printer) {
	p.Atom("#(fastVar")
	p.Atom(fmt.Sprint(me.index))
	p.ListEnd()
}

