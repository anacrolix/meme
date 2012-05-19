package meme

type lambda struct {
	body  Evalable
	fixed []string
	rest  *string
}

func (me lambda) Print(p *Printer) {
	p.Atom("#(lambda")
	if len(me.fixed) != 0 {
		p.ListStart()
		for _, s := range me.fixed {
			p.Atom(s)
		}
		if me.rest != nil {
			p.Atom(".")
			p.Atom(*me.rest)
		}
		p.ListEnd()
	} else if me.rest != nil {
		p.Atom(*me.rest)
	} else {
		p.ListStart()
		p.ListEnd()
	}
	me.body.Print(p)
	p.ListEnd()
}

func (me lambda) Eval(env Env) Node {
	return newFunc(me, env.(mapEnv), nil).Eval(env)
}
