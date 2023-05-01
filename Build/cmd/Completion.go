package cmd

import (
	compile "build/compile"
	utils "build/utils"
	"fmt"
	"io"
	"os"
	"regexp"
	"sort"
)

type CompletionArgs struct {
	Inputs []utils.StringVar
	Output utils.Filename
}

var GetCompletionArgs = utils.NewCommandParsableFlags(&CompletionArgs{})

func OptionCommandCompletionArgs() utils.CommandOptionFunc {
	return utils.OptionCommandItem(func(ci utils.CommandItem) {
		ci.Options(
			utils.OptionCommandParsableAccessor("CompletionArgs", "control completion command output", GetCompletionArgs),
			utils.OptionCommandConsumeMany("Input", "multiple command input", &GetCompletionArgs().Inputs, utils.COMMANDARG_OPTIONAL))
	})
}

func (flags *CompletionArgs) Flags(cfv utils.CommandFlagsVisitor) {
	cfv.Variable("Output", "optional output file", &flags.Output)
}

func openCompletion(args *CompletionArgs, closure func(io.Writer) error) error {
	utils.LogVerbose("completion: input parameters = %v", args.Inputs)
	if args.Output.Basename != "" {
		utils.LogInfo("export completion results to %q...", args.Output)
		return utils.UFS.CreateBuffered(args.Output, closure)
	} else {
		return closure(os.Stdout)
	}
}
func printCompletion(args *CompletionArgs, in []string) error {
	return openCompletion(args, func(w io.Writer) error {
		filterCompletion(args, func(s string) {
			utils.WithoutLog(func() {
				fmt.Fprintln(w, s)
			})
		}, in...)
		return nil
	})
}
func mapCompletion[T any](completionArgs *CompletionArgs, output func(string), values map[string]T) {
	filterCompletion(completionArgs, output, utils.Keys(values)...)
}
func filterCompletion(completionArgs *CompletionArgs, output func(string), in ...string) {
	sort.Strings(in)
	args := completionArgs.Inputs

	if len(args) > 0 {
		pbar := utils.LogProgress(0, len(args), "filter-completion")
		defer pbar.Close()

		for _, q := range args {
			glob := regexp.MustCompile(regexp.QuoteMeta(q.Get()))
			for _, x := range in {
				if glob.MatchString(x) {
					output(x)
				}
			}
			pbar.Inc()
		}
	} else {
		pbar := utils.LogProgress(0, len(in), "filter-completion")
		defer pbar.Close()

		for _, x := range in {
			output(x)
			pbar.Inc()
		}
	}
}

var ListArtifacts = utils.NewCommand(
	"Metadata",
	"list-artifacts",
	"list all known artifacts",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		bg := utils.CommandEnv.BuildGraph()
		args := GetCompletionArgs()
		return openCompletion(args, func(w io.Writer) error {
			filterCompletion(args, func(s string) {
				a := utils.BuildAlias(s)
				node := bg.Find(a)

				utils.WithoutLog(func() {
					fmt.Fprintf(w, "%v --> %v (%T)\n", node.GetBuildStamp(), a, node.GetBuildable())
				})
			}, utils.Stringize(bg.Aliases()...)...)
			return nil
		})
	}))

var ListCommands = utils.NewCommand(
	"Metadata",
	"list-commands",
	"list all available commands",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		return printCompletion(GetCompletionArgs(), utils.Commands.Keys())
	}))

var ListPlatforms = utils.NewCommand(
	"Metadata",
	"list-platforms",
	"list all available platforms",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		return printCompletion(GetCompletionArgs(), compile.AllPlatforms.Keys())
	}))

var ListConfigs = utils.NewCommand(
	"Metadata",
	"list-configs",
	"list all available configurations",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		return printCompletion(GetCompletionArgs(), compile.AllConfigurations.Keys())
	}))

var ListCompilers = utils.NewCommand(
	"Metadata",
	"list-compilers",
	"list all available compilers",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		return printCompletion(GetCompletionArgs(), compile.AllCompilers.Slice())
	}))

var ListModules = utils.NewCommand(
	"Metadata",
	"list-modules",
	"list all available modules",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		bg := utils.CommandEnv.BuildGraph()
		if result := compile.GetBuildModules().Build(bg); result.Failure() == nil {
			return printCompletion(GetCompletionArgs(), utils.Stringize(result.Success().Modules...))
		} else {
			return result.Failure()
		}
	}))

var ListNamespaces = utils.NewCommand(
	"Metadata",
	"list-namespaces",
	"list all available namespaces",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		bg := utils.CommandEnv.BuildGraph()
		if result := compile.GetBuildModules().Build(bg); result.Failure() == nil {
			return printCompletion(GetCompletionArgs(), utils.Stringize(result.Success().Namespaces...))
		} else {
			result.Failure()
		}
		return nil
	}))

var ListEnvironments = utils.NewCommand(
	"Metadata",
	"list-environments",
	"list all compilation environments",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		return printCompletion(GetCompletionArgs(), utils.Stringize(compile.GetEnvironmentAliases()...))
	}))

var ListTargets = utils.NewCommand(
	"Metadata",
	"list-targets",
	"list all translated targets",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		bg := utils.CommandEnv.BuildGraph()
		return compile.ForeachEnvironmentAlias(func(ea compile.EnvironmentAlias) error {
			if result := compile.GetBuildTargets(ea).Build(bg); result.Failure() == nil {
				return printCompletion(GetCompletionArgs(), utils.Stringize(utils.Keys(result.Success().Targets)...))
			} else {
				return result.Failure()
			}
		})
	}))

var ListPersistentData = utils.NewCommand(
	"Metadata",
	"list-persistents",
	"list all persistent data",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		args := GetCompletionArgs()
		data := utils.CommandEnv.Persistent().PinData()
		return openCompletion(args, func(w io.Writer) error {
			mapCompletion(args, func(s string) {
				utils.WithoutLog(func() {
					fmt.Printf("%v=%v\n", s, data[s])
				})
			}, data)
			return nil
		})
	}))

var ListModifiedFiles = utils.NewCommand(
	"Metadata",
	"list-modified-files",
	"list modified files from source control",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		bg := utils.CommandEnv.BuildGraph()
		result := utils.BuildSourceControlModifiedFiles().Build(bg)
		return printCompletion(GetCompletionArgs(), utils.Stringize(result.Success().ModifiedFiles...))
	}))
