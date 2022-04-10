package cmd

import (
	compile "build/compile"
	. "build/utils"
	"strings"
)

/***************************************
 * FBuild command
 ***************************************/

type fbuildCmdArgs struct {
	Any BoolVar
	FBuildArgs
}

func (flags *fbuildCmdArgs) InitFlags(cfg *PersistentMap) {
	cfg.Var(&flags.Any, "Any", "will build any unit matching the given args")
}
func (flags *fbuildCmdArgs) ApplyVars(cfg *PersistentMap) {
}

var FBuild = MakeCommand(
	"fbuild",
	"launch FASTBuild compilation process",
	func(cmd *CommandEnvT) *fbuildCmdArgs {
		GenerateSourceControlModifiedFiles()
		args := &fbuildCmdArgs{
			Any: false,
			FBuildArgs: FBuildArgs{
				Cache:         FBUILD_CACHE_DISABLED,
				Clean:         false,
				Dist:          false,
				BffInput:      BFFFILE_DEFAULT,
				NoUnity:       false,
				NoStopOnError: false,
				Report:        false,
				ShowCmds:      false,
				Threads:       0,
			},
		}
		cmd.Flags.Add("fbuild", args)
		return args
	},
	func(cmd *CommandEnvT, args *fbuildCmdArgs) (err error) {
		scm := GenerateSourceControlModifiedFiles()
		targets := cmd.ConsumeArgs(-1)
		if args.Any {
			if build, err := compile.BuildTargets.Build(cmd.BuildGraph()); err == nil {
				targetNames := Stringize(build.TranslatedUnits()...)
				targetGlobs := []string{}
				for i, input := range targets {
					targets[i] = strings.ToUpper(input)
				}
				for _, targetName := range targetNames {
					targetName = strings.ToUpper(targetName)
					for _, input := range targets {
						if strings.Contains(targetName, input) {
							targetGlobs = append(targetGlobs, targetName)
						}
					}
				}
				targets = targetGlobs
			} else {
				LogPanic("failed to load translated units: %v", err)
			}
		}
		fbuild := MakeFBuildExecutor(&args.FBuildArgs, targets...)
		scm.Join().Success()
		return fbuild.Run()
	},
)
