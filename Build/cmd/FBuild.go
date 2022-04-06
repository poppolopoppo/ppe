package cmd

import (
	. "build/utils"
)

/***************************************
 * FBuild command
 ***************************************/

var FBuild = MakeCommand(
	"fbuild",
	"launch FASTBuild compilation process",
	func(cmd *CommandEnvT) *FBuildArgs {
		GenerateSourceControlModifiedFiles()
		args := &FBuildArgs{
			Cache:         FBUILD_CACHE_DISABLED,
			Clean:         false,
			Dist:          false,
			BffInput:      BFFFILE_DEFAULT,
			NoUnity:       false,
			NoStopOnError: false,
			Report:        false,
			ShowCmds:      false,
			Threads:       0,
		}
		cmd.Flags.Add("fbuild", args)
		return args
	},
	func(cmd *CommandEnvT, args *FBuildArgs) (err error) {
		scm := GenerateSourceControlModifiedFiles()
		fbuild := MakeFBuildExecutor(args, cmd.ConsumeArgs(-1)...)
		scm.Join().Success()
		return fbuild.Run()
	},
)
