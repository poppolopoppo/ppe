package cmd

import (
	utils "build/utils"
	"fmt"
	"io"
	"math/rand"
	"reflect"
	"sort"
	"time"
)

var CommandCheckBuild = utils.NewCommand(
	"Debug",
	"check-build",
	"build graph aliases passed as input parameters",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		bg := utils.CommandEnv.BuildGraph()
		args := GetCompletionArgs()

		// look for every nodes passed as input parameters
		targets := utils.Map(func(it utils.StringVar) utils.BuildAlias {
			utils.LogVerbose("check-build: find build graph node named %q", it)
			node := bg.Find(utils.BuildAlias(it.Get()))
			if node == nil {
				utils.LogPanic("check-build: node could not be found %q", it)
			}
			return node.Alias()
		}, args.Inputs...)

		// build all nodes found
		result := bg.BuildMany(targets)
		return result.Join().Failure()
	}))

var CheckFingerprint = utils.NewCommand(
	"Debug",
	"check-fingerprint",
	"recompute nodes fingerprint and see if they match with the stamp stored in build graph",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		bg := utils.CommandEnv.BuildGraph()
		args := GetCompletionArgs()

		for _, it := range args.Inputs {
			a := utils.BuildAlias(it.Get())
			utils.LogVerbose("check-fingerprint: find build graph node named %q", a)

			// find the node associated with this alias
			node := bg.Find(a)
			if node == nil {
				utils.LogPanic("check-fingerprint: node could not be found %q", a)
			}

			// compute buildable fingerprint and check wether it matches save build stamp or not
			buildable := node.GetBuildable()
			checksum := utils.MakeBuildFingerprint(buildable)
			original := node.GetBuildStamp().Content

			// if save build stamp do not match, we will try to find which property is not stable by rebuilding it
			if original == checksum {
				utils.LogInfo("check-fingerprint: %q -> OK\n\told: %v\n\tnew: %v", a, original, checksum)
			} else {
				utils.LogWarning("check-fingerprint: %q -> KO\n\told: %v\n\tnew: %v", a, original, checksum)

				// duplicate original buildable, so we can make a diff after the build
				v := reflect.ValueOf(buildable).Elem()
				original := reflect.New(v.Type())
				original.Elem().Set(v)

				// build the node and check for errors
				_, future := bg.Build(node, utils.OptionBuildForce)
				result := future.Join()

				if err := result.Failure(); err != nil {
					return err
				}

				utils.LogInfo("check-fingerprint: %q ->\n\tbuild: %v", a, result.Success().BuildStamp)

				// finally make a diff between the original backup and the updated node after the build
				// -> the diff should issue an error on the property causing the desynchronization
				if err := utils.SerializableDiff(original.Interface().(utils.Serializable), result.Success().Buildable); err != nil {
					return err
				}
			}
		}

		return nil
	}))

var CheckSerialize = utils.NewCommand(
	"Debug",
	"check-serialize",
	"write and load every node, then check for differences",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		bg := utils.CommandEnv.BuildGraph()
		aliases := bg.Aliases()

		pbar := utils.LogProgress(0, len(aliases), "check-serialize")
		defer pbar.Close()

		ar := utils.NewArchiveDiff()
		defer ar.Close()

		type Stats struct {
			Num  int32
			Size int64
		}
		perClass := map[reflect.Type]Stats{}

		for _, a := range aliases {
			node := bg.Find(a)
			buildable := node.GetBuildable()

			bench := utils.LogBenchmark("%10s bytes   %T -> %q", utils.MakeStringer(func() string {
				return fmt.Sprint(ar.Len())
			}), buildable, a)

			// compare buildable with self: if an error happens then Serialize() has a bug somewhere
			// err should contain a stack of serialized objects, and hopefuly ease debugging
			if err := ar.Diff(buildable, buildable); err != nil {
				return err
			}

			bench.Close()
			pbar.Inc()

			stats := perClass[reflect.TypeOf(buildable)]
			stats.Num += 1
			stats.Size += int64(ar.Len())
			perClass[reflect.TypeOf(buildable)] = stats
		}

		classBySize := utils.Keys(perClass)
		sort.Slice(classBySize, func(i, j int) bool {
			return perClass[classBySize[i]].Size > perClass[classBySize[j]].Size
		})

		if len(classBySize) > 30 {
			classBySize = classBySize[:30]
		}

		for _, class := range classBySize {
			stats := perClass[class]
			utils.LogInfo("%6d elts  -  %10.3f KiB  -  %v", stats.Num, float32(stats.Size)/1024.0, class)
		}

		return nil
	}))

var DependencyChain = utils.NewCommand(
	"Debug",
	"dependency-chain",
	"find shortest dependency chain between 2 nodes",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		args := GetCompletionArgs()
		if len(args.Inputs) < 2 {
			return fmt.Errorf("dependency-chain: must pass at least 2 targets")
		}

		// print the dependency chain found
		return openCompletion(args, func(w io.Writer) error {

			for i := 1; i < len(args.Inputs); i += 1 {
				// build graph will use Dijkstra to find the shortest path between the 2 nodes
				buildGraph := utils.CommandEnv.BuildGraph()
				chain, err := buildGraph.GetDependencyChain(
					utils.BuildAlias(args.Inputs[0]),
					utils.BuildAlias(args.Inputs[i]))
				if err != nil {
					return err
				}

				indent := ""
				for i, link := range chain {
					utils.WithoutLog(func() {
						fmt.Fprintf(w, "%s[%d] %s: %s", indent, i, link.Type, link.Alias)
						if utils.IsLogLevelActive(utils.LOG_VERBOSE) {
							node := buildGraph.Find(link.Alias)
							fmt.Fprintf(w, " -> %v", node.GetBuildStamp())
						}
						fmt.Fprintln(w)
					})
					indent += "  "
				}
			}

			return nil
		})
	}))

var ProgressBar = utils.NewCommand(
	"Debug",
	"progress-bar",
	"print several progress bars for debugging",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		spinner := utils.LogSpinner("spinner")
		defer spinner.Close()

		for {
			n := int(10 + rand.Uint32()%50)
			pbar := utils.LogProgress(0, n, "progress")

			for i := 0; i < n; i += 1 {
				n2 := int(10 + rand.Uint32()%100)
				pbar2 := utils.LogProgress(0, n2, "progress2")

				spinner2 := utils.LogSpinner("spinner")

				for j := 0; j < n2; j += 1 {
					pbar2.Inc()
					spinner.Inc()
					spinner2.Inc()
					time.Sleep(time.Millisecond * 10)
				}

				pbar.Inc()
				time.Sleep(time.Millisecond * 50)
				pbar2.Close()
				spinner2.Close()
			}

			pbar.Close()
		}

		return nil
	}))

var ShowVersion = utils.NewCommand(
	"Debug",
	"version",
	"print build version",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		return openCompletion(GetCompletionArgs(), func(w io.Writer) (err error) {
			utils.WithoutLog(func() {
				_, err = fmt.Fprintln(w, utils.PROCESS_INFO)
			})
			return err
		})
	}))

var ShowSeed = utils.NewCommand(
	"Debug",
	"seed",
	"print build seed",
	OptionCommandCompletionArgs(),
	utils.OptionCommandRun(func(cc utils.CommandContext) error {
		return openCompletion(GetCompletionArgs(), func(w io.Writer) (err error) {
			utils.WithoutLog(func() {
				_, err = fmt.Printf("%v\n", utils.GetProcessSeed())
			})
			return err
		})
	}))
