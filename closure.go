package meme

import (
	"log"
)

type Closure struct {
	Env    Env
	body   Evalable
	locals []string
	fixed  int
	rest   bool
	names map[string]int
}

var _ Applicable = &Closure{}

func (me Closure) Print(p *Printer) {
	p.Atom("#(closure)")
}

func (me Closure) Apply(args List, outer Env) Node {
	env := NewFastEnv(outer, len(me.locals), &me)
	i := 0
	for ; i < me.fixed; i++ {
		env.vars[i] = &Var{args.Car()}
		args = args.Cdr()
	}
	if me.rest {
		env.vars[i] = &Var{args}
		i++
	} else if !args.IsNull() {
		panic("too many args given")
	}
	for ; i < len(me.locals); i++ {
		env.vars[i] = &Var{}
	}
	return Eval(me.body, env)
}

func rewriteBegins(node Node) Node {
	switch val := node.(type) {
	case begin:
		exps := make([]Evalable, 0, len(val))
		for _, e := range val {
			e1 := Rewrite(e, rewriteBegins).(Evalable)
			if beg, ok := e1.(begin); ok {
				exps = append(exps, beg...)
			} else {
				exps = append(exps, e1)
			}
		}
		if len(exps) == 1 {
			return exps[0]
		}
		return begin(exps)
	}
	return nil
}

func NewClosure(func_ *Func, env Env) *Closure {
	log.Println("making closure from", SprintNode(func_))
	ret := &Closure{
		Env:    env,
		fixed:  len(func_.fixed),
		rest:   func_.rest != nil,
		locals: append(func_.fixed),
	}
	if func_.rest != nil {
		ret.locals = append(ret.locals, *func_.rest)
	}
	ret.body = Rewrite(func_.body, func(node Node) Node {
		switch val := node.(type) {
		case define:
			ret.locals = append(ret.locals, val.name)
			return NewQuote(Void)
		}
		return nil
	}).(Evalable)
	log.Println("closure locals:", ret.locals)
	ret.body = Rewrite(ret.body, rewriteBegins).(Evalable)
	ret.names = make(map[string]int, len(ret.locals))
	for i, name := range ret.locals {
		ret.names[name] = i
	}
	ret.body = Rewrite(ret.body, func(node Node) Node {
		if sym, ok := node.(Symbol); ok {
			if i, ok := ret.names[sym.Value()]; ok {
				return fastVar{
					closure: ret,
					index: i,
				}
			}
			ret := env.Find(sym.Value())
			if ret == nil {
				panic("symbol not defined: " + sym.Value())
			}
			return ret
		}
		return nil
	}).(Evalable)
	return ret
}
