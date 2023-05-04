package cmd

import (
	"build/compile"
	. "build/utils"
	"fmt"
	"io"
	"math/rand"
	"os"
	"reflect"
	"sort"
	"time"
)

var CommandCheckBuild = NewCommand(
	"Debug",
	"check-build",
	"build graph aliases passed as input parameters",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		bg := CommandEnv.BuildGraph()
		args := GetCompletionArgs()

		// look for every nodes passed as input parameters
		targets := Map(func(it StringVar) BuildAlias {
			LogVerbose("check-build: find build graph node named %q", it)
			node := bg.Find(BuildAlias(it.Get()))
			if node == nil {
				LogPanic("check-build: node could not be found %q", it)
			}
			return node.Alias()
		}, args.Inputs...)

		// build all nodes found
		result := bg.BuildMany(targets)
		return result.Join().Failure()
	}))

var CommandCheckCache = NewCommand(
	"Debug",
	"check-cache",
	"inspect action cache content validity and clean invalid/outdated entries",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		cache := compile.GetActionCache()
		cachePath := cache.GetCachePath()

		tempPath := UFS.Transient.Folder("check-cache")
		UFS.Mkdir(tempPath)
		defer os.RemoveAll(tempPath.String())

		cacheGlob := MakeGlobRegexp("*" + compile.ActionCacheEntryExtname)
		expireTime := time.Now().AddDate(0, -1, 0) // remove all cached entries older than 1 month

		return cachePath.MatchFilesRec(func(f Filename) error {
			GetGlobalWorkerPool().Queue(func() {
				var entry compile.ActionCacheEntry

				removeCacheEntry := false
				defer func() {
					if !removeCacheEntry {
						return
					}
					LogWarning("check-cache: remove %q cache entry")
					for _, bulk := range entry.Bulks {
						UFS.Remove(bulk.Path)
					}

					UFS.Remove(f)
				}()

				LogDebug("check-cache: found cache entry %q", f)
				if err := entry.Load(f); err != nil {
					LogError("check-cache: %v", err)
					removeCacheEntry = true
				}

				LogVerbose("check-cache: read cache entry %q with key %s and %d bulks", f, entry.Key.GetFingerprint().ShortString(), len(entry.Bulks))
				for i := 0; i < len(entry.Bulks); {
					removeBulk := false
					bulk := &entry.Bulks[i]

					dst := tempPath.Folder(bulk.Path.ReplaceExt("").Basename)
					defer os.RemoveAll(dst.String())

					if artifacts, err := bulk.Inflate(dst); err == nil {
						for _, it := range artifacts {
							UFS.Remove(it)
						}
					} else {
						LogError("check-cache: %v", err)
						removeBulk = true
					}

					if !removeBulk { // expire cache entries
						removeBulk = removeBulk || UFS.MTime(bulk.Path).Before(expireTime)
					}

					if removeBulk {
						LogVerbose("check-cache: remove cache bulk %q", bulk)
						UFS.Remove(bulk.Path)

						if i+1 < len(entry.Bulks) {
							entry.Bulks[i] = entry.Bulks[len(entry.Bulks)-1]
						}
						entry.Bulks = entry.Bulks[:len(entry.Bulks)-1]
					} else {
						i += 1
					}
				}

				if len(entry.Bulks) == 0 {
					removeCacheEntry = true
				}
			})
			return nil
		}, cacheGlob)
	}))

var CheckFingerprint = NewCommand(
	"Debug",
	"check-fingerprint",
	"recompute nodes fingerprint and see if they match with the stamp stored in build graph",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		bg := CommandEnv.BuildGraph()
		args := GetCompletionArgs()

		for _, it := range args.Inputs {
			a := BuildAlias(it.Get())
			LogVerbose("check-fingerprint: find build graph node named %q", a)

			// find the node associated with this alias
			node := bg.Find(a)
			if node == nil {
				LogPanic("check-fingerprint: node could not be found %q", a)
			}

			// compute buildable fingerprint and check wether it matches save build stamp or not
			buildable := node.GetBuildable()
			checksum := MakeBuildFingerprint(buildable)
			original := node.GetBuildStamp().Content

			// if save build stamp do not match, we will try to find which property is not stable by rebuilding it
			if original == checksum {
				LogInfo("check-fingerprint: %q -> OK\n\told: %v\n\tnew: %v", a, original, checksum)
			} else {
				LogWarning("check-fingerprint: %q -> KO\n\told: %v\n\tnew: %v", a, original, checksum)

				// duplicate original buildable, so we can make a diff after the build
				v := reflect.ValueOf(buildable).Elem()
				original := reflect.New(v.Type())
				original.Elem().Set(v)

				// build the node and check for errors
				_, future := bg.Build(node, OptionBuildForce)
				result := future.Join()

				if err := result.Failure(); err != nil {
					return err
				}

				LogInfo("check-fingerprint: %q ->\n\tbuild: %v", a, result.Success().BuildStamp)

				// finally make a diff between the original backup and the updated node after the build
				// -> the diff should issue an error on the property causing the desynchronization
				if err := SerializableDiff(original.Interface().(Serializable), result.Success().Buildable); err != nil {
					return err
				}
			}
		}

		return nil
	}))

var CheckSerialize = NewCommand(
	"Debug",
	"check-serialize",
	"write and load every node, then check for differences",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		bg := CommandEnv.BuildGraph()
		aliases := bg.Aliases()

		pbar := LogProgress(0, len(aliases), "check-serialize")
		defer pbar.Close()

		ar := NewArchiveDiff()
		defer ar.Close()

		type Stats struct {
			Num      int32
			Size     int64
			Duration time.Duration
		}
		perClass := map[reflect.Type]Stats{}

		for _, a := range aliases {
			node := bg.Find(a)
			buildable := node.GetBuildable()

			bench := LogBenchmark("%10s bytes   %T -> %q", MakeStringer(func() string {
				return fmt.Sprint(ar.Len())
			}), buildable, a)

			// compare buildable with self: if an error happens then Serialize() has a bug somewhere
			// err should contain a stack of serialized objects, and hopefuly ease debugging
			if err := ar.Diff(buildable, buildable); err != nil {
				return err
			}

			duration := bench.Close()
			pbar.Inc()

			stats := perClass[reflect.TypeOf(buildable)]
			stats.Num += 1
			stats.Size += int64(ar.Len())
			stats.Duration += duration
			perClass[reflect.TypeOf(buildable)] = stats
		}

		classBySize := Keys(perClass)
		sort.Slice(classBySize, func(i, j int) bool {
			return perClass[classBySize[i]].Size > perClass[classBySize[j]].Size
		})

		if len(classBySize) > 30 {
			classBySize = classBySize[:30]
		}

		for _, class := range classBySize {
			stats := perClass[class]
			LogInfo("%6d elts  -  %10.3f KiB  - %8.3f MiB/s  -  %v",
				stats.Num,
				float32(stats.Size)/1024.0,
				(float64(stats.Size)/(1024*1024.0))/(float64(stats.Duration.Seconds())),
				class)
		}

		return nil
	}))

var DependencyChain = NewCommand(
	"Debug",
	"dependency-chain",
	"find shortest dependency chain between 2 nodes",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		args := GetCompletionArgs()
		if len(args.Inputs) < 2 {
			return fmt.Errorf("dependency-chain: must pass at least 2 targets")
		}

		// print the dependency chain found
		return openCompletion(args, func(w io.Writer) error {

			for i := 1; i < len(args.Inputs); i += 1 {
				// build graph will use Dijkstra to find the shortest path between the 2 nodes
				buildGraph := CommandEnv.BuildGraph()
				chain, err := buildGraph.GetDependencyChain(
					BuildAlias(args.Inputs[0]),
					BuildAlias(args.Inputs[i]))
				if err != nil {
					return err
				}

				indent := ""
				for i, link := range chain {
					WithoutLog(func() {
						fmt.Fprintf(w, "%s[%d] %s: %s", indent, i, link.Type, link.Alias)
						if IsLogLevelActive(LOG_VERBOSE) {
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

var ProgressBar = NewCommand(
	"Debug",
	"progress-bar",
	"print several progress bars for debugging",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		spinner := LogSpinner("spinner")
		defer spinner.Close()

		for {
			n := int(10 + rand.Uint32()%50)
			pbar := LogProgress(0, n, "progress")

			for i := 0; i < n; i += 1 {
				n2 := int(10 + rand.Uint32()%100)
				pbar2 := LogProgress(0, n2, "progress2")

				spinner2 := LogSpinner("spinner")

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

var ShowVersion = NewCommand(
	"Debug",
	"version",
	"print build version",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		return openCompletion(GetCompletionArgs(), func(w io.Writer) (err error) {
			WithoutLog(func() {
				_, err = fmt.Fprintln(w, PROCESS_INFO)
			})
			return err
		})
	}))

var ShowSeed = NewCommand(
	"Debug",
	"seed",
	"print build seed",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		return openCompletion(GetCompletionArgs(), func(w io.Writer) (err error) {
			WithoutLog(func() {
				_, err = fmt.Printf("%v\n", GetProcessSeed())
			})
			return err
		})
	}))
