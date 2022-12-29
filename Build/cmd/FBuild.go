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
	flags.FBuildArgs.InitFlags(cfg)
	cfg.Var(&flags.Any, "Any", "will build any unit matching the given args")
}
func (flags *fbuildCmdArgs) ApplyVars(cfg *PersistentMap) {
	flags.FBuildArgs.ApplyVars(cfg)
}

var FBuild = MakeCommand(
	"fbuild",
	"launch FASTBuild compilation process",
	func(cmd *CommandEnvT) *fbuildCmdArgs {
		GenerateSourceControlModifiedFiles()
		args := &fbuildCmdArgs{
			Any: INHERITABLE_FALSE,
			FBuildArgs: FBuildArgs{
				Cache:         FBUILD_CACHE_DISABLED,
				Clean:         INHERITABLE_FALSE,
				Dist:          INHERITABLE_FALSE,
				BffInput:      BFFFILE_DEFAULT,
				NoUnity:       INHERITABLE_FALSE,
				NoStopOnError: INHERITABLE_TRUE,
				Report:        INHERITABLE_FALSE,
				ShowCmds:      INHERITABLE_FALSE,
				Threads:       0,
			},
		}
		cmd.Flags.Add("fbuild", args)
		return args
	},
	func(cmd *CommandEnvT, args *fbuildCmdArgs) (err error) {
		scm := GenerateSourceControlModifiedFiles()
		targetInputs := cmd.ConsumeArgs(-1)
		if args.Any.Get() {
			if build, err := compile.BuildTargets.Build(cmd.BuildGraph()); err == nil {
				targetNames := Stringize(build.TranslatedUnits()...)
				targetGlobs := []string{}

				for _, targetName := range targetNames {
					for _, input := range targetInputs {
						LogDebug("fbuild: check target <%v> against input <%v>", targetName, input)
						if strings.Contains(strings.ToUpper(targetName), strings.ToUpper(input)) {
							targetGlobs = append(targetGlobs, targetName)
						}
					}
				}

				if len(targetGlobs) == 0 {
					LogFatal("fbuild: no target matching [ %v ]", strings.Join(targetInputs, ", "))
				}

				targetInputs = targetGlobs
			} else {
				LogPanic("fbuild: failed to load translated units: %v", err)
			}
		}
		fbuild := MakeFBuildExecutor(&args.FBuildArgs, targetInputs...)
		scm.Join().Success()
		return fbuild.Run()
	},
)
