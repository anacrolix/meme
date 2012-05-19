package meme

type mapEnv interface {
	Find(string) *Var
	Define(string) *Var
}
