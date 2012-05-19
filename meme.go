package meme

import (
	"errors"
	"flag"
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

func SprintNode(n Node) string {
	p := NewPrinter()
	n.Print(&p)
	return p.buf.String()
}

func Eval(a Evalable, env Env) (ret Node) {
	if Trace {
		log.Println("evaluating", SprintNode(a))
	}
	ret = a.Eval(env)
	if ret == nil {
		log.Println("error evaluating", SprintNode(a))
	} else if Trace {
		log.Println("evaluated", SprintNode(a), "->", SprintNode(ret))
	}
	return
}

func Rewrite(node Node, f RewriteFunc) (ret Node) {
	if Trace {
		log.Println("rewriting", SprintNode(node))
		defer func() {
			if ret != nil {
				log.Println("rewrote", SprintNode(node), "->", SprintNode(ret))
			}
		}()
	}
	ret = f(node)
	if ret != nil {
		return
	}
	if rw, ok := node.(Rewritable); ok {
		return rw.Rewrite(func(n Node) Node {
			return Rewrite(n, f)
		})
	}
	return node
}

func Analyze(a Parseable, env mapEnv) (ret Evalable) {
	if Trace {
		log.Println("analyzing", SprintNode(a))
		defer func() {
			log.Println("analyzed", SprintNode(a), "->", SprintNode(ret))
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
		log.Println("applying", SprintNode(a), "to", SprintNode(args))
	}
	ret = a.Apply(args, env)
	if ret == nil {
		log.Println("error applying", SprintNode(a), "to", SprintNode(args))
	} else if Trace {
		log.Println("applied", SprintNode(a), "to", SprintNode(args), "->", SprintNode(ret))
	}
	return
}
