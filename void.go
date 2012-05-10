package meme

type voidType struct{}

var Void voidType

func IsVoid(a Node) bool {
	_, ok := a.(voidType)
	return ok
}
