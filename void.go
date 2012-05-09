package meme

type voidType struct{}
var Void voidType

func IsVoid(a interface{}) bool {
    _, ok := a.(voidType)
    return ok
}

