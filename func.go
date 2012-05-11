package meme

type Func struct {
	body  List
	fixed []string
	rest  *string
}

func (me *Func) Eval(env Env) interface{} {
	return NewClosure(me, env)
}

func (me *Func) Run(args List, outer Env) interface{} {
	env := NewMapEnv(outer)
	for _, name := range me.fixed {
		env.Define(name, args.Car())
		args = args.Cdr()
	}
	if me.rest != nil {
		env.Define(*me.rest, args)
	} else if !args.IsNull() {
		panic(nil)
	}
	return applyBegin(me.body, env)
}

func NewFunc(fixed []string, rest *string, body List) *Func {
	return &Func{
		fixed: fixed,
		rest:  rest,
		body:  body,
	}
}
