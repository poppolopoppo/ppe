package cmd

import (
	compile "build/compile"
	utils "build/utils"
	"fmt"
	"sort"
)

type CompletionArgs struct{}

func (*CompletionArgs) InitFlags(cfg *utils.PersistentMap) {
}
func (*CompletionArgs) ApplyVars(cfg *utils.PersistentMap) {
}

var ListArtifacts = utils.MakeCommand(
	"list-artifacts",
	"list all known artifacts",
	nil,
	func(env *utils.CommandEnvT, _ *CompletionArgs) error {
		aliases := env.BuildGraph().Aliases()
		sort.Slice(aliases, func(i, j int) bool {
			return string(aliases[i]) < string(aliases[j])
		})
		for _, a := range aliases {
			node := env.BuildGraph().Node(a)
			fmt.Printf("%v --> %v (%T)\n", node.GetBuildStamp(), a, node.GetBuildable())
		}
		return nil
	},
)

var ListCommands = utils.MakeCommand(
	"list-commands",
	"list all available commands",
	nil,
	func(_ *utils.CommandEnvT, _ *CompletionArgs) error {
		for _, name := range utils.AllCommands.Keys() {
			fmt.Println(name)
		}
		return nil
	},
)

var ListPlatforms = utils.MakeCommand(
	"list-platforms",
	"list all available platforms",
	nil,
	func(_ *utils.CommandEnvT, _ *CompletionArgs) error {
		for _, name := range sort.StringSlice(compile.AllPlatforms.Keys()) {
			fmt.Println(name)
		}
		return nil
	},
)

var ListConfigs = utils.MakeCommand(
	"list-configs",
	"list all available configurations",
	nil,
	func(_ *utils.CommandEnvT, _ *CompletionArgs) error {
		for _, name := range sort.StringSlice(compile.AllConfigurations.Keys()) {
			fmt.Println(name)
		}
		return nil
	},
)

var ListCompilers = utils.MakeCommand(
	"list-compilers",
	"list all available compilers",
	nil,
	func(_ *utils.CommandEnvT, _ *CompletionArgs) error {
		for _, name := range sort.StringSlice(compile.AllCompilers.Keys()) {
			fmt.Println(name)
		}
		return nil
	},
)

var ListModules = utils.MakeCommand(
	"list-modules",
	"list all available modules",
	nil,
	func(env *utils.CommandEnvT, _ *CompletionArgs) error {
		build := compile.BuildModules.Build(env.BuildGraph())
		for _, name := range build.ModuleKeys() {
			fmt.Println(name)
		}
		return nil
	},
)

var ListNamespaces = utils.MakeCommand(
	"list-namespaces",
	"list all available namespaces",
	nil,
	func(env *utils.CommandEnvT, _ *CompletionArgs) error {
		build := compile.BuildModules.Build(env.BuildGraph())
		for _, name := range build.NamespaceKeys() {
			fmt.Println(name)
		}
		return nil
	},
)

var ListPersistentData = utils.MakeCommand(
	"list-persistent-data",
	"list all persistent data",
	nil,
	func(env *utils.CommandEnvT, _ *CompletionArgs) error {
		for k, v := range env.Persistent().Data {
			fmt.Printf("%v=%v\n", k, v)
		}
		return nil
	},
)

var ListPersistentVars = utils.MakeCommand(
	"list-persistent-vars",
	"list all persistent variables",
	nil,
	func(env *utils.CommandEnvT, _ *CompletionArgs) error {
		for k, v := range env.Persistent().Vars {
			fmt.Printf("%v=%v\n", k, v)
		}
		return nil
	},
)
