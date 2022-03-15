package build

import (
	"build/cmd"
	"build/compile"
	"build/hal"
	"build/utils"
)

func initInternals() {
	hal.InitHAL()
	utils.InitUtils()
	compile.InitCompile()
	cmd.InitCmd()
}

func LaunchCommand(prefix string, rootFile utils.Filename, args []string) {
	defer utils.PurgePinnedLogs()
	defer utils.StartProfiling()()
	defer utils.StartTrace()()

	env := utils.InitCommandEnv(prefix, rootFile)
	initInternals()

	env.Load()
	defer env.Save()

	env.Init(args)
	env.Run()
}
