package cmd

import "encoding/gob"

func InitCmd() {
	gob.Register(&BffArgs{})
	gob.Register(&BffBuilder{})
}
