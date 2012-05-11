package meme

type Env interface {
	Define(string, interface{})
	Find(string) interface{}
}
