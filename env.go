package meme

type Env interface {
    Define(string, interface{})
    Find(string) interface{}
	FindVar(string) *Var
	SetFast(*Func, int, interface{})
}

