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
	names  map[string]int
	parent *Closure
}

var _ Applicable = &Closure{}
var _ Evalable = &Closure{}

func (me *Closure) Eval(Env) Node {
	return me
}

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

func (me *Closure) fixName(name string, env Env) Evalable {
	for ups := 0; ; ups++ {
		if index, ok := me.names[name]; ok {
			return fastVar{
				closure: me,
				index:   index,
				ups: ups,
			}
		}
		me = me.parent
		if me == nil {
			break
		}
	}
	ret := env.Find(name)
	if ret == nil {
		panic("symbol not defined: " + name)
	}
	return ret
}

func NewClosure(func_ *Func, env Env, outer *Closure) *Closure {
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
	ret.parent = outer
	ret.body = Rewrite(ret.body, func(node Node) Node {
		if sym, ok := node.(Symbol); ok {
			return ret.fixName(sym.Value(), env)
		}
		return nil
	}).(Evalable)
	ret.body = Rewrite(ret.body, func(node Node) Node {
		switch val := node.(type) {
		case *Func:
			return NewClosure(val, env, ret)
		}
		return nil
	}).(Evalable)
	return ret
}
