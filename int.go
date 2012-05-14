package meme

type Int struct {
	val int64
}

var _ Evalable = Int{}

func (me Int) Eval(Env) Node {
	return me
}

func (me Int) Int64() int64 {
	return me.val
}

func (me Int) Less(other Node) bool {
	otherInt, ok := other.(Int)
	if !ok {
		panic(other)
	}
	return me.Int64() < otherInt.Int64()
}

func NewInt(val int64) Int {
	return Int{val: val}
}

func (me Int) String() string {
	return printString(me)
}

func (me Int) Print(p *Printer) {
	p.Atom(me.val)
}
