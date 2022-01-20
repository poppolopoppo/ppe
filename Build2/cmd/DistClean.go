package cmd

import (
	. "build/compile"
	. "build/utils"
	"os"
)

type DistCleanArgs struct{}

func (*DistCleanArgs) InitFlags(cfg *PersistentMap) {
}
func (*DistCleanArgs) ApplyVars(cfg *PersistentMap) {
}

var DistClean = MakeCommand(
	"distclean",
	"erase generated artifacts",
	func(cmd *CommandEnvT) *DistCleanArgs {
		AllCompilationFlags.Needed(cmd.Flags)
		return &DistCleanArgs{}
	},
	func(cmd *CommandEnvT, _ *DistCleanArgs) error {
		args := cmd.ConsumeArgs(-1)
		if len(args) == 0 {
			distCleanDir(UFS.Binaries)
			distCleanDir(UFS.Cache)
			distCleanDir(UFS.Generated)
			distCleanDir(UFS.Intermediate)
			distCleanDir(UFS.Projects)
			distCleanDir(UFS.Transient)

			// clean the database, not the config
			distCleanFile(CommandEnv.DatabasePath())

		} else {
			translatedUnits, err := BuildTargets.Build(cmd.BuildGraph())
			if err != nil {
				panic(err)
			}

			re := MakeGlobRegexp(args...)
			LogClaim("matching %d translated units with /%v/", translatedUnits.Len(), re)
			for _, u := range translatedUnits.Slice() {
				if re.MatchString(u.Target.String()) {
					distCleanDir(u.GeneratedDir)
					distCleanDir(u.IntermediateDir)

					outputRe := MakeGlobRegexp(u.OutputFile.ReplaceExt(".*").Basename)
					u.OutputFile.Dirname.MatchFiles(func(f Filename) error {
						distCleanFile(f)
						return nil
					}, outputRe)
				}
			}
		}

		return nil
	},
)

func distCleanFile(f Filename) {
	LogInfo("remove file '%v'", f)
	err := os.RemoveAll(f.String())
	if err != nil {
		LogWarning("distclean: %v", err)
	}
}
func distCleanDir(d Directory) {
	LogInfo("remove directory '%v'", d)
	err := os.RemoveAll(d.String())
	if err != nil {
		LogWarning("distclean: %v", err)
	}
}
