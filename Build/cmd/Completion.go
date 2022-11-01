package cmd

import (
	compile "build/compile"
	utils "build/utils"
	"fmt"
	"regexp"
	"sort"
)

type CompletionArgs struct{}

func (*CompletionArgs) InitFlags(cfg *utils.PersistentMap) {
}
func (*CompletionArgs) ApplyVars(cfg *utils.PersistentMap) {
}

func printCompletion(cmd *utils.CommandEnvT, in []string) {
	filterCompletion(cmd, func(s string) {
		fmt.Println(s)
	}, in...)
}
func mapCompletion[T any](cmd *utils.CommandEnvT, output func(string), values map[string]T) {
	filterCompletion(cmd, output, utils.Keys(values)...)
}
func filterCompletion(cmd *utils.CommandEnvT, output func(string), in ...string) {
	sort.Strings(in)
	args := cmd.ConsumeArgs(-1)
	if len(args) > 0 {
		for _, q := range args {
			glob := regexp.MustCompile(q)
			for _, x := range in {
				if glob.MatchString(x) {
					output(x)
				}
			}
		}
	} else {
		for _, x := range in {
			output(x)
		}
	}
}

var ListArtifacts = utils.MakeCommand(
	"list-artifacts",
	"list all known artifacts",
	nil,
	func(cmd *utils.CommandEnvT, _ *CompletionArgs) error {
		filterCompletion(cmd, func(s string) {
			a := utils.BuildAlias(s)
			node := cmd.BuildGraph().Node(a)
			fmt.Printf("%v --> %v (%T)\n", node.GetBuildStamp(), a, node.GetBuildable())
		}, utils.Stringize(cmd.BuildGraph().Aliases()...)...)
		return nil
	},
)

var ListCommands = utils.MakeCommand(
	"list-commands",
	"list all available commands",
	nil,
	func(cmd *utils.CommandEnvT, _ *CompletionArgs) error {
		printCompletion(cmd, utils.AllCommands.Keys())
		return nil
	},
)

var ListPlatforms = utils.MakeCommand(
	"list-platforms",
	"list all available platforms",
	nil,
	func(cmd *utils.CommandEnvT, _ *CompletionArgs) error {
		printCompletion(cmd, compile.AllPlatforms.Keys())
		return nil
	},
)

var ListConfigs = utils.MakeCommand(
	"list-configs",
	"list all available configurations",
	nil,
	func(cmd *utils.CommandEnvT, _ *CompletionArgs) error {
		printCompletion(cmd, compile.AllConfigurations.Keys())
		return nil
	},
)

var ListCompilers = utils.MakeCommand(
	"list-compilers",
	"list all available compilers",
	nil,
	func(cmd *utils.CommandEnvT, _ *CompletionArgs) error {
		printCompletion(cmd, compile.AllCompilers.Slice())
		return nil
	},
)

var ListModules = utils.MakeCommand(
	"list-modules",
	"list all available modules",
	nil,
	func(cmd *utils.CommandEnvT, _ *CompletionArgs) error {
		if build, err := compile.BuildModules.Build(cmd.BuildGraph()); err == nil {
			printCompletion(cmd, build.ModuleKeys())
		} else {
			utils.LogPanicErr(err)
		}
		return nil
	},
)

var ListNamespaces = utils.MakeCommand(
	"list-namespaces",
	"list all available namespaces",
	nil,
	func(cmd *utils.CommandEnvT, _ *CompletionArgs) error {
		if build, err := compile.BuildModules.Build(cmd.BuildGraph()); err == nil {
			printCompletion(cmd, build.NamespaceKeys())
		} else {
			utils.LogPanicErr(err)
		}
		return nil
	},
)

var ListEnvironments = utils.MakeCommand(
	"list-environments",
	"list all compilation environments",
	nil,
	func(cmd *utils.CommandEnvT, _ *CompletionArgs) error {
		if build, err := compile.BuildEnvironments.Build(cmd.BuildGraph()); err == nil {
			printCompletion(cmd, utils.Stringize(build.Slice()...))
		} else {
			utils.LogPanicErr(err)
		}
		return nil
	},
)

var ListTargets = utils.MakeCommand(
	"list-targets",
	"list all translated targets",
	nil,
	func(cmd *utils.CommandEnvT, _ *CompletionArgs) error {
		if build, err := compile.BuildTargets.Build(cmd.BuildGraph()); err == nil {
			printCompletion(cmd, utils.Stringize(build.TranslatedUnits()...))
		} else {
			utils.LogPanicErr(err)
		}
		return nil
	},
)

var ExportNode = utils.MakeCommand(
	"export-node",
	"export build node to json",
	nil,
	func(cmd *utils.CommandEnvT, _ *CompletionArgs) error {
		aliases := cmd.BuildGraph().Aliases()
		completion := make(map[string]utils.BuildAlias, len(aliases))
		for _, a := range aliases {
			completion[a.String()] = a
		}
		results := make(map[utils.BuildAlias]utils.BuildNode, 8)
		mapCompletion(cmd, func(s string) {
			alias := completion[s]
			results[alias] = cmd.BuildGraph().Node(alias)
		}, completion)
		fmt.Println(utils.PrettyPrint(results))
		return nil
	},
)

var ExportUnit = utils.MakeCommand(
	"export-unit",
	"export translated unit to json",
	nil,
	func(cmd *utils.CommandEnvT, _ *CompletionArgs) error {
		if build, err := compile.BuildTargets.Build(cmd.BuildGraph()); err == nil {
			targets := build.TranslatedUnits()
			exportedUnits := make(map[string]*compile.Unit, targets.Len())
			for _, unit := range targets.Slice() {
				exportedUnits[unit.String()] = unit
			}
			results := []*compile.Unit{}
			mapCompletion(cmd, func(s string) {
				results = append(results, exportedUnits[s])
			}, exportedUnits)
			fmt.Println(utils.PrettyPrint(results))
		} else {
			utils.LogPanicErr(err)
		}
		return nil
	},
)

var ListPersistentData = utils.MakeCommand(
	"list-persistent-data",
	"list all persistent data",
	nil,
	func(cmd *utils.CommandEnvT, _ *CompletionArgs) error {
		data := cmd.Persistent().Data
		mapCompletion(cmd, func(s string) {
			fmt.Printf("%v=%v\n", s, data[s])
		}, data)
		return nil
	},
)

var ListPersistentVars = utils.MakeCommand(
	"list-persistent-vars",
	"list all persistent variables",
	nil,
	func(cmd *utils.CommandEnvT, _ *CompletionArgs) error {
		vars := cmd.Persistent().Vars
		mapCompletion(cmd, func(s string) {
			fmt.Printf("%v=%v\n", s, vars[s])
		}, vars)
		return nil
	},
)

var ShowVersion = utils.MakeCommand(
	"version",
	"print build version",
	nil,
	func(_ *utils.CommandEnvT, _ *CompletionArgs) error {
		fmt.Println(utils.MAIN_SIGNATURE)
		return nil
	},
)

var ShowSeed = utils.MakeCommand(
	"seed",
	"print build seed",
	nil,
	func(_ *utils.CommandEnvT, _ *CompletionArgs) error {
		fmt.Printf("%X\n", utils.Seed.Slice())
		return nil
	},
)
