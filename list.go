package meme

type List interface {
    Evalable
	Printable
    Car() interface{}
    Cdr() List
    IsNull() bool
	Index(uint) interface{}
	Len() uint
	Map(func(interface{})interface{}) List
}

