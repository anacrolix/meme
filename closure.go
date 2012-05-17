package meme

import (
	"log"
)

type Closure struct {
	Env    Env
	body   Evalable
	locals []string
	fixed int
	rest bool
}

var _ Applicable = &Closure{}

func (me Closure) Print(p *Printer) {
	p.Atom("#(closure)")
}

func (me Closure) Apply(args List, outer Env) Node {
	env := NewFastEnv(outer)
	i := 0
	for ; i < me.fixed; i++ {
		env.define(me.locals[i]).Set(args.Car())
		args = args.Cdr()
	}
	if me.rest {
		env.define(me.locals[i]).Set(args)
		i++
	} else if !args.IsNull() {
		panic("too many args given")
	}
	for ; i < len(me.locals); i++ {
		env.define(me.locals[i])
	}
	return Eval(me.body, env)
}

func rewriteBegins(node Node) Node {
	switch val := node.(type) {
	case begin:
		exps := []Evalable{}
		for _, e := range val {
			e1 := rewriteBegins(e).(Evalable)
			if beg, ok := e1.(begin); ok {
				exps = append(exps, beg...)
			} else {
				exps = append(exps, e1)
			}
		}
		if len(exps) == 1 {
			return exps[0]
		} else {
			return begin(exps)
		}
	}
	return Rewrite(node, rewriteBegins)
}

func NewClosure(func_ *Func, env Env) Closure {
	log.Println("making closure from", SprintNode(func_))
	ret := Closure{
		Env:   env,
		fixed: len(func_.fixed),
		rest: func_.rest != nil,
		locals: append(func_.fixed),
	}
	if func_.rest != nil {
		ret.locals = append(ret.locals, *func_.rest)
	}
	var rwf RewriteFunc
	rwf = func(node Node) Node {
		switch val := node.(type) {
		case define:
			ret.locals = append(ret.locals, val.name)
			return NewQuote(Void)
		}
		return Rewrite(node, rwf)
	}
	ret.body = rwf(func_.body).(Evalable)
	log.Println("closure locals:", ret.locals)
	ret.body = rewriteBegins(ret.body).(Evalable)
	return ret
}
