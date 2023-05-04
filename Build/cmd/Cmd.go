package cmd

import . "build/utils"

func InitCmd() {
	RegisterSerializable(&BffBuilder{})
	RegisterSerializable(&CompilationDatabaseBuilder{})
	RegisterSerializable(&VcxprojBuilder{})
	RegisterSerializable(&VscodeBuilder{})
}
