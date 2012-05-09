package meme

type Applicable interface {
    Apply(List, Env) interface{}
    Special() bool
}

func Truth(a interface{}) bool {
    _, ok := a.(falseType)
    return !ok
}

