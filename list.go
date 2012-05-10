package meme

type MapFunc func(Node) Node

type List interface {
	Parseable
	Car() Node
	Cdr() List
	IsNull() bool
	Index(uint) Node
	Len() uint
	Map(MapFunc) List
}
