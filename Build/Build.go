package build

import (
	"build/cmd"
	"build/compile"
	"build/hal"
	"build/utils"
	"os"
	"time"
)

func WithBuild(prefix string, scope func(*utils.CommandEnvT) error) error {
	startedAt := time.Now()

	defer utils.StartTrace()()
	defer utils.PurgePinnedLogs()

	env := utils.InitCommandEnv(prefix, os.Args[1:], startedAt)

	defer utils.StartProfiling()()

	hal.InitHAL()
	utils.InitUtils()
	compile.InitCompile()
	cmd.InitCmd()

	err := scope(env)
	if err != nil {
		utils.LogError(utils.LogCommand, "%v", err)
	}
	return err
}

/***************************************
 * Launch Command (program entry point)
 ***************************************/

func LaunchCommand(prefix string, rootFile utils.Filename) error {
	return WithBuild(prefix, func(env *utils.CommandEnvT) error {
		env.SetRootFile(rootFile)

		env.LoadConfig()
		env.LoadBuildGraph()

		err := env.Run()
		if err != nil {
			utils.LogError(utils.LogCommand, "%v", err)
		}

		env.SaveConfig()
		env.SaveBuildGraph()
		return err
	})
}
