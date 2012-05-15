package meme

type Env interface {
	Define(string, Node)
	Find(string) Node
	Set(string, Node)
}
