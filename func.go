package meme

type Func struct {
    body Evaler
	locals []string
	fixed int
	rest bool
	outer *Func
}

func (me Func) NumParams() (num int) {
	num = me.fixed
	if me.rest {
		num++
	}
	return
}

func (me Func) Eval(env Env) interface{} {
    return Closure{
        Env: env,
        Func: me,
    }
}

func (me Func) Apply([]Node, Env) Node {
    panic("can't apply unevaluated function")
}

func (me *Func) Run(args List, outer Env) interface{} {
	env := NewFastEnv(outer, len(me.locals))
	for i := 0; i < me.fixed; i++ {
		env.SetFast(me, i, args.Car())
		args = args.Cdr()
	}
	if me.rest {
		env.SetFast(me, me.fixed, args)
	} else if !args.IsNull() {
		panic("too many arguments given")
	}
	return me.body.Eval(env)
}

func (me Func) NewFastVar(name string) *FastVar {
	for i := 0; i < len(me.locals); i++ {
		if me.locals[i] == name {
			return &FastVar{
				index: i,
				func_: &me,
			}
		}
	}
	return me.outer.NewFastVar(name)
}
