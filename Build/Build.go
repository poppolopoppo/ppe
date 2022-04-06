package build

import (
	"build/cmd"
	"build/compile"
	"build/hal"
	"build/utils"
	"strings"
)

func initInternals() {
	hal.InitHAL()
	utils.InitUtils()
	compile.InitCompile()
	cmd.InitCmd()
}

func splitArgsIFN(args []string, each func([]string) error) error {
	first := 0
	for last := 0; last < len(args); last += 1 {
		if strings.TrimSpace(args[last]) == `-and` {
			if first < last {
				if err := each(args[first:last]); err != nil {
					return err
				}
			}
			first = last + 1
		}
	}

	if first < len(args) {
		return each(args[first:])
	}

	return nil
}

func LaunchCommand(prefix string, rootFile utils.Filename, args []string) {
	defer utils.PurgePinnedLogs()
	defer utils.StartProfiling()()
	defer utils.StartTrace()()

	env := utils.InitCommandEnv(prefix, rootFile)
	initInternals()

	env.Load()
	defer env.Save()

	splitArgsIFN(args, func(subArgs []string) error {
		env.Init(subArgs)
		return nil
	})

	if err := env.Run(); err != nil {
		utils.LogError("command failed: %v", err)
	}
}
