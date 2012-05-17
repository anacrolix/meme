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

func (me Int) Less(other Node) (less bool, err error) {
	if otherInt, ok := other.(Int); ok {
		return me.Int64() < otherInt.Int64(), nil
	}
	err = TypeError
	return
}

func NewInt(val int64) Int {
	return Int{val: val}
}

func (me Int) Print(p *Printer) {
	p.Atom(me.val)
}
