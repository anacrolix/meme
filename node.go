package meme

type Analyzer interface {
	Analyze(List, mapEnv) Evalable
}

type Applicable interface {
	Node
	Apply(List, Env) Node
}

type Expandable interface {
	Expand(Env) Parseable
}

type Evalable interface {
	Node
	Eval(Env) Node
}

type Parseable interface {
	Evalable
}

type Comparable interface {
	Node
	Less(Node) (bool, error)
}

type Printable interface {
	Print(*Print)
}

type RewriteFunc func(Node) Node

type Rewritable interface {
	Rewrite(RewriteFunc) Node
}

type Node interface {
	Printable
}
