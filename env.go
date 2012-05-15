package meme

type Env interface {
	Define(string) *Var
	Find(string) *Var
}
