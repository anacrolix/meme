package meme

import (
	"errors"
	"flag"
	"fmt"
	"log"
)

var Trace bool

func init() {
	flag.BoolVar(&Trace, "trace", false, "enable tracing logs")
}

var TypeError = errors.New("type error")

func Truth(a Node) bool {
	_, ok := a.(falseType)
	return !ok
}

func Eval(a Evalable, env Env) (ret Node) {
	if Trace {
		log.Println("evaluating", a)
		defer func() {
			log.Println("evaluated", fmt.Sprintf("%#v", a), "->", ret)
		}()
	}
	ret = a.Eval(env)
	return
}

func Analyze(a Parseable, env Env) (ret Evalable) {
	if Trace {
		log.Println("analyzing", a)
		defer func() {
			log.Println("analyzed", a, "->", ret)
		}()
	}
	var list List
	var ok bool
	if list, ok = a.(List); !ok {
		return a
	}
	proc := Analyze(list.Car().(Parseable), env)
	if sym, ok := proc.(Symbol); ok {
		var_ := env.Find(sym.Value())
		if var_ != nil {
			if spec, ok := var_.Get().(Special); ok {
				return spec.Analyze(list.Cdr(), env)
			}
		}
	}
	return Cons(proc, list.Cdr().Map(func(a Node) Node {
		return Analyze(a.(Parseable), env)
	}))
}

func Apply(a Applicable, args List, env Env) (ret Node) {
	if Trace {
		log.Println("applying", a, "to", args)
		defer func() {
			log.Println("applied", a, "to", args, "->", ret)
		}()
	}
	ret = a.Apply(args, env)
	return
}

func evalList(a List, env Env) List {
	return a.Map(func(a Node) Node {
		return Eval(a.(Evalable), env)
	})
}
