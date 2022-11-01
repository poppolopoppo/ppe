package cmd

import (
	. "build/compile"
	. "build/utils"
	"bytes"
	"fmt"
	"io"
	"os"
	"strings"
)

type VscodeArgsT struct{}

func (flags *VscodeArgsT) InitFlags(cfg *PersistentMap) {
}
func (flags *VscodeArgsT) ApplyVars(cfg *PersistentMap) {
}
func (flags *VscodeArgsT) Alias() BuildAlias {
	return MakeBuildAlias("Flags", "VscodeArgs")
}
func (flags *VscodeArgsT) Build(BuildContext) (BuildStamp, error) {
	return MakeBuildStamp(flags)
}
func (flags *VscodeArgsT) GetDigestable(o *bytes.Buffer) {
}

var VscodeArgs = MakeServiceAccessor[ParsableFlags](newVscodeArgs)

func newVscodeArgs() *VscodeArgsT {
	return CommandEnv.BuildGraph().Create(&VscodeArgsT{}).GetBuildable().(*VscodeArgsT)
}

var Vscode = MakeCommand(
	"vscode",
	"generate workspace for Visual Studio Code",
	func(cmd *CommandEnvT) *VscodeArgsT {
		AllCompilationFlags.Needed(cmd.Flags)
		return VscodeArgs.FindOrAdd(cmd.Flags)
	},
	func(cmd *CommandEnvT, args *VscodeArgsT) error {
		outputDir := UFS.Root.Folder(".vscode")
		LogClaim("generating VSCode workspace in '%v'", outputDir)

		bg := cmd.BuildGraph()
		builder := bg.Create(&VscodeBuilder{
			OutputDir: outputDir,
		}, args.Alias())

		_, result := bg.ForceBuild(builder.GetBuildable())
		return result.Join().Failure()
	},
)

/***************************************
 * Visual Studio Code workspace generation
 ***************************************/

type VscodeBuilder struct {
	OutputDir Directory
}

func (vsc *VscodeBuilder) Alias() BuildAlias {
	return MakeBuildAlias("Vscode", vsc.OutputDir.String())
}
func (vsc *VscodeBuilder) Build(bc BuildContext) (BuildStamp, error) {
	LogVerbose("build vscode configuration in '%v'...", vsc.OutputDir)

	/*args := */
	VscodeArgs.Need(CommandEnv.Flags)
	environments := BuildEnvironments.Need(bc)
	modules := BuildModules.Need(bc)
	targets := BuildTargets.Need(bc)
	platform := BuildPlatforms.Need(bc).Current()
	compiler := platform.GetCompiler(bc)

	CommandEnv.ConsumeArgs(-1) // eat args so FBuildExecutor won't use them

	c_cpp_properties_json := vsc.OutputDir.File("c_cpp_properties.json")
	LogTrace("generating vscode c/c++ properties in '%v'", c_cpp_properties_json)
	if err := vsc.c_cpp_properties(environments, targets, c_cpp_properties_json); err != nil {
		return BuildStamp{}, err
	}
	bc.OutputFile(c_cpp_properties_json)

	tasks_json := vsc.OutputDir.File("tasks.json")
	LogTrace("generating vscode build tasks in '%v'", tasks_json)
	if err := vsc.tasks(modules, tasks_json); err != nil {
		return BuildStamp{}, err
	}
	bc.OutputFile(tasks_json)

	launch_json := vsc.OutputDir.File("launch.json")
	LogTrace("generating vscode launch configuratiosn in '%v'", launch_json)
	if err := vsc.launch_configs(modules, compiler, launch_json); err != nil {
		return BuildStamp{}, err
	}
	bc.OutputFile(launch_json)

	return vsc.OutputDir.Build(bc)
}

func (vsc *VscodeBuilder) c_cpp_properties(environments *BuildEnvironmentsT, targets *BuildTargetsT, outputFile Filename) error {
	configurations := []JsonMap{}

	for _, env := range environments.Slice() {
		environmentAlias := env.EnvironmentAlias()

		var intelliSenseMode string
		switch env.GetPlatform().Os {
		case "Linux":
			intelliSenseMode = fmt.Sprintf("linux-%s-x64", env.Compiler.FriendlyName())
		case "Windows":
			intelliSenseMode = fmt.Sprintf("windows-%s-x64", env.Compiler.FriendlyName())
		default:
			UnexpectedValue(env.GetPlatform().Os)
		}

		compiledb := env.IntermediateDir().File("compile_commands.json")
		if err := vsc.make_compiledb(environmentAlias, compiledb); err != nil {
			return err
		}

		translatedUnits := targets.TranslatedUnits()
		translatedUnits = translatedUnits.RemoveUnless(func(u *Unit) bool {
			return u.Target.EnvironmentAlias == environmentAlias
		})

		includePaths := DirSet{}
		for _, u := range translatedUnits {
			includePaths.AppendUniq(u.IncludePaths...)
		}

		configurations = append(configurations, JsonMap{
			"name":             env.EnvironmentAlias().Alias().String(),
			"compilerPath":     env.GetCompiler().Executable.String(),
			"compileCommands":  compiledb.String(),
			"cStandard":        "c11",
			"cppStandard":      strings.ToLower(env.GetCppStd(nil).String()),
			"defines":          env.Defines,
			"includePath":      includePaths,
			"intelliSenseMode": intelliSenseMode,
			"browse": JsonMap{
				"path":                          includePaths,
				"limitSymbolsToIncludedHeaders": true,
				"databaseFilename":              env.IntermediateDir().File("vscode-vc.db"),
			},
		})
	}

	return UFS.Create(outputFile, func(w io.Writer) error {
		return JsonSerialize(JsonMap{
			"version":        4,
			"configurations": configurations,
		}, w)
	})
}
func (vsc *VscodeBuilder) tasks(modules *BuildModulesT, outputFile Filename) error {
	selfExecutable, err := os.Executable()
	if err != nil {
		LogPanicErr(err)
	}

	var problemMatcher string
	switch CurrentHost().Id {
	case HOST_LINUX, HOST_DARWIN:
		problemMatcher = "$gcc"
	case HOST_WINDOWS:
		problemMatcher = "$msCompile"
	default:
		return MakeUnexpectedValueError(problemMatcher, CurrentHost().Id)
	}

	tasks := Map(func(moduleName string) JsonMap {
		alias := modules.Modules[moduleName].ModuleAlias()
		return JsonMap{
			"label":   alias.String(),
			"command": selfExecutable,
			"args":    []string{"fbuild", "-v", alias.String() + "-${command:cpptools.activeConfigName}"},
			"options": JsonMap{
				"cwd": UFS.Root,
			},
			"group": JsonMap{
				"kind":      "build",
				"isDefault": true,
			},
			"presentation": JsonMap{
				"clear":  true,
				"echo":   true,
				"reveal": "always",
				"focus":  false,
				"panel":  "dedicated",
			},
			"problemMatcher": problemMatcher,
		}
	}, modules.ModuleKeys()...)

	return UFS.Create(outputFile, func(w io.Writer) error {
		return JsonSerialize(JsonMap{
			"version": "2.0.0",
			"tasks":   tasks,
		}, w)
	})
}
func (vsc *VscodeBuilder) launch_configs(modules *BuildModulesT, compiler Compiler, outputFile Filename) error {
	var debuggerType string
	switch CurrentHost().Id {
	case HOST_LINUX, HOST_DARWIN:
		debuggerType = "cppdbg"
	case HOST_WINDOWS:
		debuggerType = "cppvsdbg"
	default:
		UnexpectedValue(CurrentHost().Id)
	}

	executableNames := RemoveUnless(func(moduleName string) bool {
		rules := modules.Modules[moduleName].GetModule(nil)
		return rules.ModuleType == MODULE_PROGRAM
	}, modules.ModuleKeys()...)

	configurations := Map(func(moduleName string) JsonMap {
		executable := strings.ReplaceAll(moduleName, "/", "-")
		cfg := JsonMap{
			"name":        moduleName,
			"type":        debuggerType,
			"request":     "launch",
			"program":     UFS.Binaries.File(executable + "-${command:cpptools.activeConfigName}" + compiler.Extname(PAYLOAD_EXECUTABLE)),
			"args":        []string{},
			"stopAtEntry": false,
			"cwd":         UFS.Binaries,
		}
		if envPath := compiler.EnvPath(); len(envPath) > 0 {
			cfg["environment"] = []JsonMap{
				{"name": "PATH", "value": JoinString(";", envPath...) + ";%PATH%"},
				{"name": "ASAN_OPTIONS", "value": "windows_hook_rtl_allocators=true"},
			}
		}
		return cfg
	}, executableNames...)

	return UFS.Create(outputFile, func(w io.Writer) error {
		return JsonSerialize(JsonMap{
			"version":        "0.2.0",
			"configurations": configurations,
		}, w)
	})
}
func (vsc *VscodeBuilder) make_compiledb(env EnvironmentAlias, output Filename) error {
	LogTrace("generating compile commands '%v' for <%v> environemnt...", output, env)

	fbuildArgs := FBuildArgs{
		BffInput: BFFFILE_DEFAULT,
	}

	fbuildExec := MakeFBuildExecutor(&fbuildArgs, "-compdb", "-nounity", env.Alias().String())
	fbuildExec.Capture = false

	if err := fbuildExec.Run(); err != nil {
		return err
	}

	compiledb := UFS.Root.File("compile_commands.json")
	if _, err := compiledb.Info(); err != nil {
		LogError("can't find generated compilation commands in '%v'", compiledb)
		return err
	}

	return UFS.Rename(compiledb, output)
}
