package build

import (
	"build/cmd"
	"build/compile"
	"build/hal"
	"build/utils"
	"strings"
	"time"
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

func printBuildGraphSummary(startedAt time.Time, g utils.BuildGraph) {
	totalDuration := time.Since(startedAt)
	utils.LogVerbose("Program took %.3f seconds to run", totalDuration.Seconds())

	stats := g.GetBuildStats()
	if stats.Count == 0 {
		return
	}

	utils.LogVerbose("Took %.3f seconds to build %d nodes using %d threads",
		stats.Duration.Exclusive.Seconds(), stats.Count, utils.GetGlobalWorkerPool().Arity())
	utils.LogVerbose("Most expansive nodes built:")

	for i, node := range g.GetMostExpansiveNodes(10, false) {
		stats := node.GetBuildStats()
		utils.LogVerbose("[%02d] - %5.2f%% -  %6.3f  %6.3f  --  %s",
			(i + 1),
			(100.0*stats.Duration.Exclusive.Seconds())/totalDuration.Seconds(),
			stats.Duration.Exclusive.Seconds(),
			stats.Duration.Inclusive.Seconds(),
			node.Alias())
	}
}

func LaunchCommand(prefix string, rootFile utils.Filename, args []string) {
	startedAt := time.Now()
	defer utils.PurgePinnedLogs()
	defer utils.StartProfiling()()
	defer utils.StartTrace()()

	env := utils.InitCommandEnv(prefix, rootFile, args)
	initInternals()

	env.Load()

	if err := env.Run(); err != nil {
		utils.LogError("command failed: %v", err)
	}

	if utils.IsLogLevelActive(utils.LOG_VERBOSE) {
		printBuildGraphSummary(startedAt, env.BuildGraph())
	}

	env.Save()
}
