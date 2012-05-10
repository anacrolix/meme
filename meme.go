package meme

import (
	"log"
)

func Truth(a Node) bool {
	_, ok := a.(falseType)
	return !ok
}

func Eval(a Evalable, env Env) (ret interface{}) {
	log.Println("evaluating", a)
	defer func() {
		log.Println("evaluated", a, "->", ret)
	}()
	ret = a.Eval(env)
	return
}

func Analyze(a Parseable, env Env) (ret Evalable) {
	log.Println("analyzing", a)
	defer func() {
		log.Println("analyzed", a, "->", ret)
	}()
	if list, ok := a.(List); ok {
		proc := Analyze(list.Car().(Parseable), env)
		if sym, ok := proc.(Symbol); ok {
			spec, ok := env.Find(sym.Value()).(Special)
			if ok {
				return Analyze(spec.Apply(list.Cdr(), env), env)
			}
		}
		return NewPair(proc, list.Cdr().Map(func(n Node)Node {
			return Analyze(n.(Parseable), env)
		}))
	}
	return a
}
