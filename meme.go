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

func Apply(a Applicable, args List, env Env) (ret Node) {
	log.Println("applying", a, "to", args)
	defer func() {
		log.Println("applied", a, "to", args, "->", ret)
	}()
	ret = a.Apply(args, env)
	return
}
