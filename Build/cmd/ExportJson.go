package cmd

import (
	compile "build/compile"
	. "build/utils"
	"fmt"
	"io"
)

func completeJsonExport[INPUT any, T any](cmd *CommandEnvT, args *CompletionArgs, factory func(INPUT) (T, error), inputs ...INPUT) error {
	matching := []T{}

	completion := make(map[string]T, len(inputs))
	for _, a := range inputs {
		it, err := factory(a)
		if err != nil {
			return err
		}

		completion[MakeString(a)] = it
	}

	mapCompletion(args, func(s string) {
		matching = append(matching, completion[s])
	}, completion)

	return openCompletion(args, func(w io.Writer) (err error) {
		WithoutLog(func() {
			_, err = fmt.Fprintln(w, PrettyPrint(matching))
		})
		return err
	})
}

var ExportConfig = NewCommand(
	"Export",
	"export-config",
	"export configuration to json",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		return completeJsonExport(CommandEnv, GetCompletionArgs(), func(name string) (compile.Configuration, error) {
			alias := compile.NewConfigurationAlias(name)

			result := compile.GetBuildConfig(alias).Build(CommandEnv.BuildGraph())

			if err := result.Failure(); err == nil {
				return result.Success(), nil
			} else {
				return nil, err
			}
		}, compile.AllConfigurations.Keys()...)
	}))

var ExportPlatform = NewCommand(
	"Export",
	"export-platform",
	"export platform to json",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		return completeJsonExport(CommandEnv, GetCompletionArgs(), func(name string) (compile.Platform, error) {
			alias := compile.NewPlatformAlias(name)
			result := compile.GetBuildPlatform(alias).Build(CommandEnv.BuildGraph())

			if err := result.Failure(); err == nil {
				return result.Success(), nil
			} else {
				return nil, err
			}
		}, compile.AllPlatforms.Keys()...)
	}))

var ExportModule = NewCommand(
	"Export",
	"export-module",
	"export parsed module to json",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		buildModules := compile.GetBuildModules().Build(CommandEnv.BuildGraph())
		if err := buildModules.Failure(); err != nil {
			return err
		}

		return completeJsonExport(CommandEnv, GetCompletionArgs(), func(ma compile.ModuleAlias) (compile.Module, error) {
			return compile.GetBuildModule(ma)
		}, buildModules.Success().Modules...)
	}))

var ExportNamespace = NewCommand(
	"Export",
	"export-namespace",
	"export parsed namespace to json",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		buildModules := compile.GetBuildModules().Build(CommandEnv.BuildGraph())
		if err := buildModules.Failure(); err != nil {
			return err
		}

		return completeJsonExport(CommandEnv, GetCompletionArgs(), func(na compile.NamespaceAlias) (compile.Namespace, error) {
			return compile.GetBuildNamespace(na)
		}, buildModules.Success().Namespaces...)
	}))

var ExportNode = NewCommand(
	"Export",
	"export-node",
	"export build node to json",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		args := GetCompletionArgs()

		aliases := CommandEnv.BuildGraph().Aliases()
		completion := make(map[string]BuildAlias, len(aliases))
		for _, a := range aliases {
			completion[a.String()] = a
		}

		results := make(map[BuildAlias]BuildNode, 8)
		mapCompletion(args, func(s string) {
			alias := completion[s]
			results[alias] = CommandEnv.BuildGraph().Find(alias)
		}, completion)

		return openCompletion(args, func(w io.Writer) (err error) {
			WithoutLog(func() {
				_, err = fmt.Fprintln(w, PrettyPrint(results))
			})
			return err
		})
	}))

var ExportUnit = NewCommand(
	"Export",
	"export-unit",
	"export translated unit to json",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		completion := make(map[string]*compile.Unit)
		compile.ForeachBuildTargets(func(bf BuildFactoryTyped[*compile.BuildTargets]) error {
			buildTargets := bf.Build(CommandEnv.BuildGraph())
			if err := buildTargets.Failure(); err != nil {
				return err
			}

			units, err := buildTargets.Success().GetTranslatedUnits()
			if err != nil {
				return err
			}

			for _, u := range units {
				completion[u.String()] = u
			}
			return nil
		})

		args := GetCompletionArgs()

		matching := compile.Units{}
		mapCompletion(args, func(s string) {
			matching.Append(completion[s])
		}, completion)

		return openCompletion(args, func(w io.Writer) (err error) {
			WithoutLog(func() {
				_, err = fmt.Fprintln(w, PrettyPrint(matching))
			})
			return err
		})
	}))
