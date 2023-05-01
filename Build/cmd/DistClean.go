package cmd

import (
	. "build/compile"
	. "build/utils"
	"os"
)

var CommandDistClean = NewCommand(
	"Compilation",
	"distclean",
	"erase generated artifacts",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		args := GetCompletionArgs()
		if len(args.Inputs) == 0 {
			LogClaim("dist-clean all output folders and database")

			distCleanDir(UFS.Binaries)
			distCleanDir(UFS.Cache)
			distCleanDir(UFS.Generated)
			distCleanDir(UFS.Intermediate)
			distCleanDir(UFS.Projects)
			distCleanDir(UFS.Transient)

			// clean the database, not the config
			distCleanFile(CommandEnv.DatabasePath())

		} else {
			re := MakeGlobRegexp(Stringize(args.Inputs...)...)
			LogClaim("dist-clean all targets matching /%v/", re)

			buildGraph := CommandEnv.BuildGraph()
			return ForeachBuildTargets(func(bf BuildFactoryTyped[*BuildTargets]) error {
				buildTargets := bf.Build(buildGraph)
				if err := buildTargets.Failure(); err != nil {
					return err
				}

				for _, unitAlias := range buildTargets.Success().Aliases {
					if re.MatchString(unitAlias.String()) {
						if unit, err := GetBuildUnit(unitAlias); err == nil {
							LogInfo("dist-clean %q build unit", unit.String())

							distCleanDir(unit.GeneratedDir)
							distCleanDir(unit.IntermediateDir)

							unit.OutputFile.Dirname.MatchFiles(func(f Filename) error {
								distCleanFile(f)
								return nil
							}, MakeGlobRegexp(unit.OutputFile.ReplaceExt(".*").Basename))

						} else {
							return err
						}
					}
				}

				return nil
			})
		}

		return nil
	}))

func distCleanFile(f Filename) {
	if f.Exists() {
		LogVerbose("remove file '%v'", f)
		err := os.RemoveAll(f.String())
		if err != nil {
			LogWarning("distclean: %v", err)
		}
		f.Invalidate()
	}
}
func distCleanDir(d Directory) {
	if d.Exists() {
		LogVerbose("remove directory '%v'", d)
		err := os.RemoveAll(d.String())
		if err != nil {
			LogWarning("distclean: %v", err)
		}
		d.Invalidate()
	}
}
