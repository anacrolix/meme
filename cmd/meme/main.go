package main

import (
	"bufio"
	"flag"
	"fmt"
	"log"
	"bitbucket.org/anacrolix/meme"
	"os"
	"runtime/pprof"
)

func runReader(reader *bufio.Reader, env *meme.TopEnv) {
	lexer := meme.NewLexer(reader)
	for {
		data := meme.Parse(&lexer)
		if data == nil {
			break
		}
		code := meme.Analyze(data, env)
		result := meme.Eval(code, env)
		if !meme.IsVoid(result) {
			fmt.Fprintln(os.Stderr, meme.SprintNode(result))
		}
	}
}

var cpuprofile = flag.String("cpuprofile", "", "write cpu profile to file")
var memprofile = flag.String("memprofile", "", "write heap profile to file")

func main() {
	flag.Parse()
	if *cpuprofile != "" {
		f, err := os.Create(*cpuprofile)
		if err != nil {
			log.Fatal(err)
		}
		pprof.StartCPUProfile(f)
		defer pprof.StopCPUProfile()
	}
	if *memprofile != "" {
		f, err := os.Create(*memprofile)
		if err != nil {
			log.Fatal(err)
		}
		pprof.WriteHeapProfile(f)
		defer f.Close()
	}
	env := meme.NewTopEnv()
	for _, name := range flag.Args() {
		file, err := os.Open(name)
		if err != nil {
			log.Fatal(err)
			return
		}
		runReader(bufio.NewReader(file), env)
	}
}
