package cmd

import (
	"bufio"
	. "build/hal"
	. "build/utils"
	"bytes"
	"io"
	"os"
	"os/exec"
	"strings"
	"time"
)

/***************************************
 * FBuild arguments
 ***************************************/

type FBuildCacheType int32

const (
	FBUILD_CACHE_DISABLED FBuildCacheType = iota
	FBUILD_CACHE_READ
	FBUILD_CACHE_WRITE
)

func FBuildCacheTypes() []FBuildCacheType {
	return []FBuildCacheType{
		FBUILD_CACHE_DISABLED,
		FBUILD_CACHE_READ,
		FBUILD_CACHE_WRITE,
	}
}
func (x FBuildCacheType) String() string {
	switch x {
	case FBUILD_CACHE_DISABLED:
		return "DISABLED"
	case FBUILD_CACHE_READ:
		return "READ"
	case FBUILD_CACHE_WRITE:
		return "WRITE"
	default:
		UnexpectedValue(x)
		return ""
	}
}
func (x *FBuildCacheType) Set(in string) error {
	switch strings.ToUpper(in) {
	case FBUILD_CACHE_DISABLED.String():
		*x = FBUILD_CACHE_DISABLED
	case FBUILD_CACHE_READ.String():
		*x = FBUILD_CACHE_READ
	case FBUILD_CACHE_WRITE.String():
		*x = FBUILD_CACHE_WRITE
	default:
		UnexpectedValue(in)
	}
	return nil
}
func (x FBuildCacheType) GetDigestable(o *bytes.Buffer) {
	o.WriteString(x.String())
}
func (x FBuildCacheType) MarshalJSON() ([]byte, error) {
	return MarshalJSON(x)
}
func (x *FBuildCacheType) UnmarshalJSON(data []byte) error {
	return UnmarshalJSON(x, data)
}

type FBuildArgs struct {
	Cache         FBuildCacheType
	Clean         BoolVar
	Dist          BoolVar
	BffFile       Filename
	NoStopOnError BoolVar
	NoUnity       BoolVar
	Report        BoolVar
	ShowCmds      BoolVar
	ShowCmdOutput BoolVar
	Threads       IntVar
}

func (flags *FBuildArgs) InitFlags(cfg *PersistentMap) {
	cfg.Var(&flags.Cache, "Cache", "set FASTBuild cache mode ["+Join(",", FBuildCacheTypes()...)+"]")
	cfg.Var(&flags.Clean, "Clean", "FASTBuild will rebuild all cached artifacts if enabled")
	cfg.Var(&flags.Dist, "Dist", "enable/disable FASTBuild to use distributed compilation")
	cfg.Persistent(&flags.BffFile, "BffFile", "source for generated FASTBuild config file (*.bff)")
	cfg.Var(&flags.NoStopOnError, "NoStopOnError", "FASTBuild will stop compiling on the first error if enabled")
	cfg.Var(&flags.NoUnity, "NoUnity", "enable/disable use of generated unity source files")
	cfg.Var(&flags.Report, "Report", "enable/disable FASTBuild compilation report generation")
	cfg.Var(&flags.ShowCmds, "ShowCmds", "display all command-lines executed by FASTBuild")
	cfg.Var(&flags.ShowCmdOutput, "ShowCmdOutput", "display full output of external processes regardless of outcome.")
	cfg.Persistent(&flags.Threads, "Threads", "set count of worker thread spawned by FASTBuild")
}
func (flags *FBuildArgs) ApplyVars(cfg *PersistentMap) {
}

/***************************************
 * FBuild Executor
 ***************************************/

type FBuildExecutor []string

func MakeFBuildExecutor(
	flags *FBuildArgs,
	args ...string) (result FBuildExecutor) {

	enableCache := false
	enableDist := false

	if flags != nil {
		result = append(result, "-config", flags.BffFile.String())

		switch flags.Cache {
		case FBUILD_CACHE_READ:
			enableCache = true
			result = append(result, "-cache", "read")
		case FBUILD_CACHE_WRITE:
			enableCache = true
			result = append(result, "-cache", "write")
		case FBUILD_CACHE_DISABLED:
		}

		if flags.Clean.Get() {
			result = append(result, "-clean")
		}
		if flags.Dist {
			enableDist = true
			result = append(result, "-dist")
		}
		if flags.NoUnity.Get() {
			result = append(result, "-nounity")
		}
		if flags.NoStopOnError.Get() {
			result = append(result, "-nostoponerror")
		} else {
			result = append(result, "-nosummaryonerror")
		}
		if flags.Report.Get() {
			result = append(result, "-report")
		}
		if flags.Threads > 0 {
			result = append(result, "-j"+flags.Threads.String())
		}
		if flags.ShowCmds {
			result = append(result, "-showcmds")
		}
		if flags.ShowCmdOutput {
			result = append(result, "-showcmdoutput")
		}
	}

	if IsLogLevelActive(LOG_DEBUG) {
		result = append(result, "-j1", "-why")
	}
	if IsLogLevelActive(LOG_VERYVERBOSE) {
		if enableCache {
			result = append(result, "-cacheverbose")
		}
		if enableDist {
			result = append(result, "-distverbose")
		}
	}
	if IsLogLevelActive(LOG_TRACE) {
		result = append(result, "-verbose")
	}
	if IsLogLevelActive(LOG_VERBOSE) {
		result = append(result, "-summary")
	}
	if !IsLogLevelActive(LOG_INFO) {
		result = append(result, "-quiet")
	}

	result = append(result, "-noprogress")

	result = append(result, args...)
	return result
}
func (x *FBuildExecutor) Run() (err error) {
	LogVerbose("runing FASTBuild with: %v", x)

	pbar := LogSpinner("FBuild")
	defer pbar.Close()

	cmd := exec.Command(FBUILD_BIN.String(), (*x)...)
	cmd.Dir = UFS.Root.String()
	cmd.Env = os.Environ()
	cmd.Env = append(cmd.Env, "FASTBUILD_CACHE_PATH="+UFS.Cache.String())
	cmd.Env = append(cmd.Env, "FASTBUILD_TEMP_PATH="+UFS.Transient.String())

	var stdout io.ReadCloser
	var stderr io.ReadCloser

	if stdout, err = cmd.StdoutPipe(); err != nil {
		return err
	}
	if stderr, err = cmd.StderrPipe(); err != nil {
		return err
	}

	reader := func(src io.ReadCloser) <-chan string {
		result := make(chan string)
		go func() {
			defer close(result)

			const MAX_LINESIZE int = 512 * 1024
			scanner := bufio.NewScanner(src)
			scanner.Buffer(make([]byte, MAX_LINESIZE), MAX_LINESIZE)

			for scanner.Scan() {
				line := scanner.Text()
				line = strings.TrimSpace(line)
				if len(line) > 0 {
					result <- line
				}
			}
		}()
		return result
	}

	stdoutReader := reader(stdout)
	stderrReader := reader(stderr)

	if err = cmd.Start(); err != nil {
		return err
	}

	processExit := make(chan error)
	go func() {
		defer close(processExit)
		processExit <- cmd.Wait()
	}()

	for {
		select {
		case line := <-stdoutReader:
			if line = strings.TrimSpace(line); len(line) > 0 {
				LogForward(line)
			}
		case line := <-stderrReader:
			if line = strings.TrimSpace(line); len(line) > 0 {
				LogError("FBuild: %v", line)
			}
		case err = <-processExit:
			for {
				if line, ok := <-stdoutReader; ok {
					if line = strings.TrimSpace(line); len(line) > 0 {
						LogForward(line)
					}
				} else if line, ok := <-stderrReader; ok {
					if line = strings.TrimSpace(line); len(line) > 0 {
						LogError("FBuild: %v", line)
					}
				} else {
					break
				}
			}
			stdout.Close()
			stderr.Close()
			return err
		default:
			time.Sleep(30 * time.Millisecond)
		}
		pbar.Inc()
	}
}
func (x *FBuildExecutor) String() string {
	return strings.Join(*x, " ")
}

/***************************************
 * FBuild command
 ***************************************/

var FBuild = MakeCommand(
	"fbuild",
	"launch FASTBuild compilation process",
	func(cmd *CommandEnvT) *FBuildArgs {
		SourceControlModifiedFiles.Prepare(cmd.BuildGraph())
		args := &FBuildArgs{
			Cache:         FBUILD_CACHE_DISABLED,
			Clean:         false,
			Dist:          false,
			BffFile:       BFFFILE_DEFAULT,
			NoUnity:       false,
			NoStopOnError: false,
			Report:        false,
			ShowCmds:      false,
			Threads:       0,
		}
		cmd.Flags.Add("fbuild", args)
		return args
	},
	func(cmd *CommandEnvT, args *FBuildArgs) (err error) {
		_, scm := SourceControlModifiedFiles.Prepare(cmd.BuildGraph())
		fbuild := MakeFBuildExecutor(args, cmd.ConsumeArgs(-1)...)
		scm.Join().Success()
		return fbuild.Run()
	},
)
