package build

import (
	"build/cmd"
	"build/compile"
	"build/hal"
	"build/utils"
	"time"
)

func initInternals() {
	hal.InitHAL()
	utils.InitUtils()
	compile.InitCompile()
	cmd.InitCmd()
}

func LaunchCommand(prefix string, rootFile utils.Filename, args []string) {
	startedAt := time.Now()

	defer utils.StartProfiling()()
	defer utils.StartTrace()()
	defer utils.PurgePinnedLogs()

	env := utils.InitCommandEnv(prefix, rootFile, args, startedAt)
	initInternals()

	env.Load()

	if err := env.Run(); err != nil {
		utils.LogError("command failed: %v", err)
	}

	env.Save()
}
