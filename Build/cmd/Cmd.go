package cmd

import utils "build/utils"

func InitCmd() {
	utils.RegisterSerializable(&BffBuilder{})
	utils.RegisterSerializable(&CompilationDatabaseBuilder{})
	utils.RegisterSerializable(&VcxprojBuilder{})
	utils.RegisterSerializable(&VscodeBuilder{})
}
