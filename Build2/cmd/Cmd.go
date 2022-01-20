package cmd

import "encoding/gob"

func InitCmd() {
	// BFF
	gob.Register(&BffArgsT{})
	gob.Register(&BffBuilder{})
	// VCXPROJ
	gob.Register(&VcxprojArgsT{})
	gob.Register(&VcxProjBuilder{})
}
