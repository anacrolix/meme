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
	if ret == nil {
		panic(nil)
	}
	return
}

func Analyze(a Parseable, env Env) (ret Evalable) {
	log.Println("analyzing", a)
	defer func() {
		log.Println("analyzed", a, "->", ret)
	}()
	var list List
	var ok bool
	if list, ok = a.(List); !ok {
		return a
	}
	proc := Analyze(list.Car().(Parseable), env)
	if sym, ok := proc.(Symbol); ok {
		if spec, ok := specials[sym.Value()]; ok {
			return spec(list.Cdr(), env)
		}
	}
	return NewPair(proc, list.Cdr().Map(func(a Node) Node {
		return Analyze(a.(Parseable), env)
	}))
}

func Apply(a Applicable, args List, env Env) (ret Node) {
	log.Println("applying", a, "to", args)
	defer func() {
		log.Println("applied", a, "to", args, "->", ret)
	}()
	ret = a.Apply(args, env)
	return
}
