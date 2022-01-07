package cmd

import "encoding/gob"

func InitCmd() {
	gob.Register(&BffArgsT{})
	gob.Register(&BffBuilder{})
}
