package cmd

import (
	. "build/compile"
	. "build/utils"
	"io"
)

var CommandCompileDb = NewCommand(
	"Configure",
	"compiledb",
	"generate json compilation database",
	OptionCommandAllCompilationFlags(),
	OptionCommandRun(func(cc CommandContext) error {
		LogClaim("generation json compilation database in %q", UFS.Intermediate)

		bg := CommandEnv.BuildGraph()

		return ParallelRange(func(ea EnvironmentAlias) error {
			builder := CompilationDatabaseBuilder{
				EnvironmentAlias: ea,
				OutputFile:       UFS.Intermediate.Folder(ea.PlatformName).Folder(ea.ConfigName).AbsoluteFile("compile_commands.json"),
			}
			return builder.Build(bg)
		}, GetEnvironmentAliases()...)
	}),
)

/***************************************
 * Compilation Database
 ***************************************/

// https://clang.llvm.org/docs/JSONCompilationDatabase.html
type CompileCommand struct {
	Directory Directory `json:"directory"`
	File      Filename  `json:"file"`
	Output    Filename  `json:"output"`
	Arguments StringSet `json:"arguments"`
}

// func (x *CompileCommand) Serialize(ar Archive) {
// 	ar.Serializable(&x.Directory)
// 	ar.Serializable(&x.File)
// 	ar.Serializable(&x.Output)
// 	ar.Serializable(&x.Arguments)
// }

type CompilationDatabase = []CompileCommand

type CompilationDatabaseBuilder struct {
	EnvironmentAlias
	OutputFile Filename
	Database   CompilationDatabase
}

//	func (x CompilationDatabaseBuilder) Alias() BuildAlias {
//		return MakeBuildAlias("Compile", "Database", x.OutputFile.String())
//	}
//
//	func (x *CompilationDatabaseBuilder) Serialize(ar Archive) {
//		ar.Serializable(&x.EnvironmentAlias)
//		ar.Serializable(&x.OutputFile)
//		SerializeSlice(ar, &x.Database)
//	}
func (x *CompilationDatabaseBuilder) Build(bg BuildGraph) error {
	LogVerbose("generate compilation database for %v in %q...", x.EnvironmentAlias, x.OutputFile)

	x.Database = make([]CompileCommand, 0, len(x.Database))

	buildModules := GetBuildModules().Build(bg)
	if err := buildModules.Failure(); err != nil {
		return err
	}

	buildTargets := GetBuildTargets(x.EnvironmentAlias).Build(bg)
	if err := buildTargets.Failure(); err != nil {
		return err
	}

	err := ForeachTargetActions(x.EnvironmentAlias, func(bf BuildFactoryTyped[*TargetActions]) error {
		targetActions := bf.Build(bg)
		if err := targetActions.Failure(); err != nil {
			return err
		}

		actions, err := targetActions.Success().GetOutputActions()
		if err != nil {
			return err
		}

		expandedActions := make(ActionSet, 0, len(actions))
		err = actions.ExpandDependencies(&expandedActions)
		if err != nil {
			return err
		}

		for _, action := range expandedActions {
			rules := action.GetAction()
			x.Database = append(x.Database, CompileCommand{
				Directory: rules.WorkingDir,
				File:      rules.Inputs[0],
				Output:    rules.Outputs[0],
				Arguments: append([]string{rules.Executable.String()}, rules.Arguments...),
			})
		}

		return nil
	}, buildModules.Success().Modules...)
	if err != nil {
		return err
	}

	err = UFS.SafeCreate(x.OutputFile, func(w io.Writer) error {
		return JsonSerialize(x.Database, w, OptionJsonPrettyPrint(true))
	})
	if err != nil {
		return err
	}

	// bc.OutputFile(x.OutputFile)
	return nil
}
