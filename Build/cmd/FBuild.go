package cmd

import (
	. "build/compile"
	. "build/utils"
	"strings"
)

/***************************************
 * FBuild command
 ***************************************/

type FBuildCommand struct {
	Targets []TargetAlias
	Any     BoolVar
	Args    FBuildArgs
}

var CommandFBuild = NewCommandable(
	"Compilation",
	"fbuild",
	"launch FASTBuild compilation process",
	&FBuildCommand{})

func (x *FBuildCommand) Flags(cfv CommandFlagsVisitor) {
	cfv.Variable("Any", "will build any unit matching the given args", &x.Any)
	x.Args.Flags(cfv)
}
func (x *FBuildCommand) Init(cc CommandContext) error {
	cc.Options(
		OptionCommandParsableFlags("CommandFBuild", "optional flags to pass to FASTBuild when compiling", x),
		OptionCommandAllCompilationFlags(),
		OptionCommandConsumeMany("TargetAlias", "build all targets specified as argument", &x.Targets),
	)
	return nil
}
func (x *FBuildCommand) Prepare(cc CommandContext) error {
	// prepare source control early on, without blocking
	BuildSourceControlModifiedFiles().Prepare(CommandEnv.BuildGraph())
	return nil
}
func (x *FBuildCommand) Run(cc CommandContext) error {
	if x.Any.Get() {
		targetGlobs := StringSet{}

		buildGraph := CommandEnv.BuildGraph()
		err := ForeachBuildTargets(func(bf BuildFactoryTyped[*BuildTargets]) error {
			if buildTargets := bf.Build(buildGraph); buildTargets.Failure() == nil {
				for _, target := range buildTargets.Success().Aliases {
					targetName := target.String()
					for _, input := range x.Targets {
						LogDebug(LogFBuild, "check target <%v> against input <%v>", targetName, input)
						if strings.Contains(strings.ToUpper(targetName), strings.ToUpper(input.String())) {
							targetGlobs = append(targetGlobs, targetName)
						}
					}
				}
			} else {
				return buildTargets.Failure()
			}
			return nil
		})
		if err != nil {
			return err
		}

		if len(targetGlobs) == 0 {
			LogFatal("fbuild: no target matching [ %v ]", strings.Join(targetGlobs, ", "))
		}
	}

	sourceControlModifiedFiles := BuildSourceControlModifiedFiles().Build(CommandEnv.BuildGraph())
	if err := sourceControlModifiedFiles.Failure(); err != nil {
		return err
	}

	fbuild := MakeFBuildExecutor(&x.Args, Stringize(x.Targets...)...)
	return fbuild.Run()
}
