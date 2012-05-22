package meme

import (
//"log"
)

type closure struct {
	func_ *func_
	env   fastEnv
}

var _ Applicable = closure{}
var _ Printable = closure{}

func (me closure) Print(p *Print) {
	p.Atom("#(closure")
	me.func_.Print(p)
	p.ListEnd()
}

func (me closure) Apply(args List, _ Env) Node {
	locals := me.func_.locals
	fixed := me.func_.fixed
	env := fastEnv{
		vars:  make([]*Var, len(locals)),
		outer: &me.env,
	}
	i := 0
	for ; i < fixed; i++ {
		env.vars[i] = &Var{
			locals[i],
			args.Car(),
		}
		args = args.Cdr()
	}
	if me.func_.rest {
		env.vars[i] = &Var{locals[i], args}
		i++
	} else if !args.IsNull() {
		panic("too many args given")
	}
	for ; i < len(locals); i++ {
		env.vars[i] = &Var{
			name: locals[i],
		}
	}
	return Eval(me.func_.body, env)
}
