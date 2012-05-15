package meme

type Func struct {
	body  Evalable
	fixed []string
	rest  *string
}

func (me *Func) Eval(env Env) Node {
	return NewClosure(me, env)
}

/*
func (me *Func) Print(p *Printer) {
	p.SyntaxToken(ListStart)
	p.Atom("#(lambda)")
	if len(me.fixed) == 0 {
		if me.rest == nil {
			Nil.Print(p)
		} else {
			p.Atom(*me.rest)
		}
	} else {
		p.SyntaxToken(ListStart)
		for _, a := range me.fixed {
			p.Atom(a)
		}
		if me.rest != nil {
			p.Atom(".")
			p.Atom(*me.rest)
		}
	}
	me.body.Print(p)
}
*/

func (me *Func) Run(args List, outer Env) Node {
	env := NewMapEnv(outer)
	for _, name := range me.fixed {
		env.Define(name).Set(args.Car())
		args = args.Cdr()
	}
	if me.rest != nil {
		env.Define(*me.rest).Set(args)
	} else if !args.IsNull() {
		panic(nil)
	}
	return Eval(me.body, env)
}

func NewFunc(fixed []string, rest *string, body Evalable) *Func {
	return &Func{
		fixed: fixed,
		rest:  rest,
		body:  body,
	}
}
