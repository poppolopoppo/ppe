package cmd

import (
	compile "build/compile"
	utils "build/utils"
	"fmt"
	"io"
)

func completeJsonExport[INPUT any, T any](cmd *utils.CommandEnvT, args *CompletionArgs, factory func(INPUT) (T, error), inputs ...INPUT) error {
	matching := []T{}

	completion := make(map[string]T, len(inputs))
	for _, a := range inputs {
		it, err := factory(a)
		if err != nil {
			return err
		}

		completion[utils.MakeString(a)] = it
	}

	mapCompletion(args, func(s string) {
		matching = append(matching, completion[s])
	}, completion)

	return openCompletion(args, func(w io.Writer) (err error) {
		utils.WithoutLog(func() {
			_, err = fmt.Fprintln(w, utils.PrettyPrint(matching))
		})
		return err
	})
}

var ExportConfig = utils.NewCommand(
	"Export",
	"export-config",
	"export configuration to json",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		return completeJsonExport(utils.CommandEnv, GetCompletionArgs(), func(name string) (compile.Configuration, error) {
			alias := compile.NewConfigurationAlias(name)

			result := compile.GetBuildConfig(alias).Build(utils.CommandEnv.BuildGraph())

			if err := result.Failure(); err == nil {
				return result.Success(), nil
			} else {
				return nil, err
			}
		}, compile.AllConfigurations.Keys()...)
	}))

var ExportPlatform = utils.NewCommand(
	"Export",
	"export-platform",
	"export platform to json",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		return completeJsonExport(utils.CommandEnv, GetCompletionArgs(), func(name string) (compile.Platform, error) {
			alias := compile.NewPlatformAlias(name)
			result := compile.GetBuildPlatform(alias).Build(utils.CommandEnv.BuildGraph())

			if err := result.Failure(); err == nil {
				return result.Success(), nil
			} else {
				return nil, err
			}
		}, compile.AllPlatforms.Keys()...)
	}))

var ExportModule = utils.NewCommand(
	"Export",
	"export-module",
	"export parsed module to json",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		buildModules := compile.GetBuildModules().Build(utils.CommandEnv.BuildGraph())
		if err := buildModules.Failure(); err != nil {
			return err
		}

		return completeJsonExport(utils.CommandEnv, GetCompletionArgs(), func(ma compile.ModuleAlias) (compile.Module, error) {
			return compile.GetBuildModule(ma)
		}, buildModules.Success().Modules...)
	}))

var ExportNamespace = utils.NewCommand(
	"Export",
	"export-namespace",
	"export parsed namespace to json",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		buildModules := compile.GetBuildModules().Build(utils.CommandEnv.BuildGraph())
		if err := buildModules.Failure(); err != nil {
			return err
		}

		return completeJsonExport(utils.CommandEnv, GetCompletionArgs(), func(na compile.NamespaceAlias) (compile.Namespace, error) {
			return compile.GetBuildNamespace(na)
		}, buildModules.Success().Namespaces...)
	}))

var ExportNode = utils.NewCommand(
	"Export",
	"export-node",
	"export build node to json",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		args := GetCompletionArgs()

		aliases := utils.CommandEnv.BuildGraph().Aliases()
		completion := make(map[string]utils.BuildAlias, len(aliases))
		for _, a := range aliases {
			completion[a.String()] = a
		}

		results := make(map[utils.BuildAlias]utils.BuildNode, 8)
		mapCompletion(args, func(s string) {
			alias := completion[s]
			results[alias] = utils.CommandEnv.BuildGraph().Find(alias)
		}, completion)

		return openCompletion(args, func(w io.Writer) (err error) {
			utils.WithoutLog(func() {
				_, err = fmt.Fprintln(w, utils.PrettyPrint(results))
			})
			return err
		})
	}))

var ExportUnit = utils.NewCommand(
	"Export",
	"export-unit",
	"export translated unit to json",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		completion := make(map[string]*compile.Unit)
		compile.ForeachBuildTargets(func(bf utils.BuildFactoryTyped[*compile.BuildTargets]) error {
			buildTargets := bf.Build(utils.CommandEnv.BuildGraph())
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
			utils.WithoutLog(func() {
				_, err = fmt.Fprintln(w, utils.PrettyPrint(matching))
			})
			return err
		})
	}))
