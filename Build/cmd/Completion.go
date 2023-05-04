package cmd

import (
	compile "build/compile"
	. "build/utils"
	"fmt"
	"io"
	"os"
	"regexp"
	"sort"
)

type CompletionArgs struct {
	Inputs []StringVar
	Output Filename
}

var GetCompletionArgs = NewCommandParsableFlags(&CompletionArgs{})

func OptionCommandCompletionArgs() CommandOptionFunc {
	return OptionCommandItem(func(ci CommandItem) {
		ci.Options(
			OptionCommandParsableAccessor("CompletionArgs", "control completion command output", GetCompletionArgs),
			OptionCommandConsumeMany("Input", "multiple command input", &GetCompletionArgs().Inputs, COMMANDARG_OPTIONAL))
	})
}

func (flags *CompletionArgs) Flags(cfv CommandFlagsVisitor) {
	cfv.Variable("Output", "optional output file", &flags.Output)
}

func openCompletion(args *CompletionArgs, closure func(io.Writer) error) error {
	LogVerbose("completion: input parameters = %v", args.Inputs)
	if args.Output.Basename != "" {
		LogInfo("export completion results to %q...", args.Output)
		return UFS.CreateBuffered(args.Output, closure)
	} else {
		return closure(os.Stdout)
	}
}
func printCompletion(args *CompletionArgs, in []string) error {
	return openCompletion(args, func(w io.Writer) error {
		filterCompletion(args, func(s string) {
			WithoutLog(func() {
				fmt.Fprintln(w, s)
			})
		}, in...)
		return nil
	})
}
func mapCompletion[T any](completionArgs *CompletionArgs, output func(string), values map[string]T) {
	filterCompletion(completionArgs, output, Keys(values)...)
}
func filterCompletion(completionArgs *CompletionArgs, output func(string), in ...string) {
	sort.Strings(in)
	args := completionArgs.Inputs

	if len(args) > 0 {
		pbar := LogProgress(0, len(args), "filter-completion")
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
		pbar := LogProgress(0, len(in), "filter-completion")
		defer pbar.Close()

		for _, x := range in {
			output(x)
			pbar.Inc()
		}
	}
}

var ListArtifacts = NewCommand(
	"Metadata",
	"list-artifacts",
	"list all known artifacts",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		bg := CommandEnv.BuildGraph()
		args := GetCompletionArgs()
		return openCompletion(args, func(w io.Writer) error {
			filterCompletion(args, func(s string) {
				a := BuildAlias(s)
				node := bg.Find(a)

				WithoutLog(func() {
					fmt.Fprintf(w, "%v --> %v (%T)\n", node.GetBuildStamp(), a, node.GetBuildable())
				})
			}, Stringize(bg.Aliases()...)...)
			return nil
		})
	}))

var ListCommands = NewCommand(
	"Metadata",
	"list-commands",
	"list all available commands",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		return printCompletion(GetCompletionArgs(), Commands.Keys())
	}))

var ListPlatforms = NewCommand(
	"Metadata",
	"list-platforms",
	"list all available platforms",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		return printCompletion(GetCompletionArgs(), compile.AllPlatforms.Keys())
	}))

var ListConfigs = NewCommand(
	"Metadata",
	"list-configs",
	"list all available configurations",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		return printCompletion(GetCompletionArgs(), compile.AllConfigurations.Keys())
	}))

var ListCompilers = NewCommand(
	"Metadata",
	"list-compilers",
	"list all available compilers",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		return printCompletion(GetCompletionArgs(), compile.AllCompilers.Slice())
	}))

var ListModules = NewCommand(
	"Metadata",
	"list-modules",
	"list all available modules",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		bg := CommandEnv.BuildGraph()
		if result := compile.GetBuildModules().Build(bg); result.Failure() == nil {
			return printCompletion(GetCompletionArgs(), Stringize(result.Success().Modules...))
		} else {
			return result.Failure()
		}
	}))

var ListNamespaces = NewCommand(
	"Metadata",
	"list-namespaces",
	"list all available namespaces",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		bg := CommandEnv.BuildGraph()
		if result := compile.GetBuildModules().Build(bg); result.Failure() == nil {
			return printCompletion(GetCompletionArgs(), Stringize(result.Success().Namespaces...))
		} else {
			result.Failure()
		}
		return nil
	}))

var ListEnvironments = NewCommand(
	"Metadata",
	"list-environments",
	"list all compilation environments",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		return printCompletion(GetCompletionArgs(), Stringize(compile.GetEnvironmentAliases()...))
	}))

var ListTargets = NewCommand(
	"Metadata",
	"list-targets",
	"list all translated targets",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		bg := CommandEnv.BuildGraph()
		return compile.ForeachEnvironmentAlias(func(ea compile.EnvironmentAlias) error {
			if result := compile.GetBuildTargets(ea).Build(bg); result.Failure() == nil {
				return printCompletion(GetCompletionArgs(), Stringize(Keys(result.Success().Targets)...))
			} else {
				return result.Failure()
			}
		})
	}))

var ListPersistentData = NewCommand(
	"Metadata",
	"list-persistents",
	"list all persistent data",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		args := GetCompletionArgs()
		data := CommandEnv.Persistent().PinData()
		return openCompletion(args, func(w io.Writer) error {
			mapCompletion(args, func(s string) {
				WithoutLog(func() {
					fmt.Printf("%v=%v\n", s, data[s])
				})
			}, data)
			return nil
		})
	}))

var ListModifiedFiles = NewCommand(
	"Metadata",
	"list-modified-files",
	"list modified files from source control",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		bg := CommandEnv.BuildGraph()
		result := BuildSourceControlModifiedFiles().Build(bg)
		return printCompletion(GetCompletionArgs(), Stringize(result.Success().ModifiedFiles...))
	}))
