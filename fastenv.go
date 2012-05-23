package meme

type fastEnv struct {
	vars  []*Var
	outer *fastEnv
}
