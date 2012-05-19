package meme

type func_ struct {
	body   Evalable
	locals []string
	fixed  int
	rest   bool
	parent *func_
}

var _ Evalable = &func_{}
var _ Printable = &func_{}

func (me *func_) Eval(env Env) Node {
	fe, _ := env.(fastEnv)
	return closure{
		func_: me,
		env:   fe,
	}
}

func (me *func_) Print(p *Printer) {
	p.Atom("#(func")
	if me.fixed != 0 {
		p.ListStart()
		for i := 0; i < me.fixed; i++ {
			p.Atom(me.locals[i])
		}
		if me.rest {
			p.Atom(".")
			p.Atom(me.locals[me.fixed])
		}
		p.ListEnd()
	} else if me.rest {
		p.Atom(me.locals[0])
	} else {
		p.ListStart()
		p.ListEnd()
	}
	me.body.Print(p)
	p.ListEnd()
}

func rewriteBegins(node Node) Node {
	switch val := node.(type) {
	case begin:
		exps := make([]Evalable, 0, len(val))
		for _, e := range val {
			e1 := Rewrite(e, rewriteBegins).(Evalable)
			switch data := e1.(type) {
			case begin:
				exps = append(exps, data...)
			default:
				exps = append(exps, data)
			}
		}
		if len(exps) == 1 {
			return exps[0]
		}
		return begin(exps)
	}
	return nil
}

func (me func_) bindName(name string, env mapEnv) Evalable {
	func_ := me
	for ups := 0; ; ups++ {
		for index, s := range func_.locals {
			if s == name {
				return fastVar{
					func_: me,
					index:   index,
					ups:     ups,
				}
			}
		}
		if func_.parent == nil {
			break
		}
		func_ = *func_.parent
	}
	ret := env.Find(name)
	if ret == nil {
		panic("symbol not defined: " + name)
	}
	return ret
}

func newFunc(_lambda lambda, env mapEnv, outer *func_) *func_ {
	ret := &func_{
		fixed:  len(_lambda.fixed),
		rest:   _lambda.rest != nil,
		locals: append(_lambda.fixed),
		parent: outer,
	}
	if _lambda.rest != nil {
		ret.locals = append(ret.locals, *_lambda.rest)
	}
	ret.body = Rewrite(_lambda.body, func(node Node) Node {
		switch val := node.(type) {
		case define:
			ret.locals = append(ret.locals, val.name)
			return NewQuote(Void)
		}
		return nil
	}).(Evalable)
	ret.body = Rewrite(ret.body, rewriteBegins).(Evalable)
	ret.body = Rewrite(ret.body, func(node Node) Node {
		if sym, ok := node.(Symbol); ok {
			return ret.bindName(sym.Value(), env.(mapEnv)).(Evalable)
		}
		return nil
	}).(Evalable)
	ret.body = Rewrite(ret.body, func(node Node) Node {
		switch val := node.(type) {
		case lambda:
			return newFunc(val, env, ret)
		}
		return nil
	}).(Evalable)
	return ret
}
