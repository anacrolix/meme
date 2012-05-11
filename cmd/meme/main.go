package main

import (
    "bufio"
    "meme"
    "log"
    "flag"
	"os"
)

func runReader(reader *bufio.Reader, env meme.Env) {
    lexer := meme.NewLexer(reader)
    for {
        data := meme.Parse(&lexer)
		if data == nil {
			break
		}
		code := data
        result := meme.Eval(code, env)
        if !meme.IsVoid(result) {
            log.Println(result.(meme.Printable))
        }
    }
}

func main() {
    flag.Parse()
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

