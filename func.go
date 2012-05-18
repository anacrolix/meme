package meme

type Func struct {
	body  Evalable
	fixed []string
	rest  *string
}

func (me *Func) Print(p *Printer) {
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
	} else if me.rest != nil {
		p.Atom(*me.rest)
	} else {
		p.ListStart()
		p.ListEnd()
	}
	me.body.Print(p)
	p.ListEnd()
}

func (me *Func) Eval(env Env) Node {
	return NewClosure(me, env, nil)
}

func NewFunc(fixed []string, rest *string, body Evalable) *Func {
	return &Func{
		fixed: fixed,
		rest:  rest,
		body:  body,
	}
}
