package utils

import (
	"bufio"
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
	BffInput      Filename
	NoStopOnError BoolVar
	NoUnity       BoolVar
	Report        BoolVar
	ShowCmds      BoolVar
	ShowCmdOutput BoolVar
	Threads       IntVar
}

func (flags *FBuildArgs) InitFlags(cfg *PersistentMap) {
	cfg.Var(&flags.Cache, "Cache", "set FASTBuild cache mode ["+JoinString(",", FBuildCacheTypes()...)+"]")
	cfg.Var(&flags.Clean, "Clean", "FASTBuild will rebuild all cached artifacts if enabled")
	cfg.Var(&flags.Dist, "Dist", "enable/disable FASTBuild to use distributed compilation")
	cfg.Persistent(&flags.BffInput, "BffInput", "source for input FASTBuild config file (*.bff)")
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

var FBUILD_BIN Filename

type FBuildExecutor struct {
	Args    SetT[string]
	Capture bool
}

func MakeFBuildExecutor(flags *FBuildArgs, args ...string) (result FBuildExecutor) {
	result.Capture = true

	enableCache := false
	enableDist := false

	if flags != nil {
		result.Args.Append("-config", flags.BffInput.String())

		switch flags.Cache {
		case FBUILD_CACHE_READ:
			enableCache = true
			result.Args.Append("-cache", "read")
		case FBUILD_CACHE_WRITE:
			enableCache = true
			result.Args.Append("-cache", "write")
		case FBUILD_CACHE_DISABLED:
		}

		if flags.Clean.Get() {
			result.Args.Append("-clean")
		}
		if flags.Dist {
			enableDist = true
			result.Args.Append("-dist")
		}
		if flags.NoUnity.Get() {
			result.Args.Append("-nounity")
		}
		if flags.NoStopOnError.Get() {
			result.Args.Append("-nostoponerror")
		} else {
			result.Args.Append("-nosummaryonerror")
		}
		if flags.Report.Get() {
			result.Args.Append("-report")
		}
		if flags.Threads > 0 {
			result.Args.Append("-j" + flags.Threads.String())
		}
		if flags.ShowCmds {
			result.Args.Append("-showcmds")
		}
		if flags.ShowCmdOutput {
			result.Args.Append("-showcmdoutput")
		}
	}

	if IsLogLevelActive(LOG_DEBUG) {
		result.Args.Append("-j1", "-why")
	}
	if IsLogLevelActive(LOG_VERYVERBOSE) {
		if enableCache {
			result.Args.Append("-cacheverbose")
		}
		if enableDist {
			result.Args.Append("-distverbose")
		}
	}
	if IsLogLevelActive(LOG_TRACE) {
		result.Args.Append("-verbose")
	}
	if IsLogLevelActive(LOG_VERBOSE) {
		result.Args.Append("-summary")
	}
	if !IsLogLevelActive(LOG_INFO) {
		result.Args.Append("-quiet")
	}

	result.Args.Append("-noprogress")
	result.Args.Append(args...)
	return result
}
func (x *FBuildExecutor) Run() (err error) {
	LogVerbose("fbuild: running with '%v'", x)

	cmd := exec.Command(FBUILD_BIN.String(), x.Args.Slice()...)
	cmd.Dir = UFS.Root.String()
	cmd.Env = append(os.Environ(),
		"FASTBUILD_CACHE_PATH="+UFS.Cache.String(),
		"FASTBUILD_TEMP_PATH="+UFS.Transient.String())

	if x.Capture {
		pbar := LogSpinner("FBuild")
		defer pbar.Close()

		cmd.Stderr = cmd.Stdout

		var stdout io.ReadCloser
		if stdout, err = cmd.StdoutPipe(); err != nil {
			return err
		}
		defer stdout.Close()

		if err = cmd.Start(); err != nil {
			return err
		}

		scanner := bufio.NewScanner(stdout)
		// const MAX_LINESIZE int = 512 * 1024
		// scanner.Buffer(make([]byte, MAX_LINESIZE), MAX_LINESIZE)

		done := make(chan struct{})
		go func() {
			defer close(done)

			for scanner.Scan() {
				line := scanner.Text()
				line = strings.TrimSpace(line)
				if len(line) > 0 {
					LogForward(line)
				}
			}

			done <- struct{}{}
		}()

		for {
			select {
			case <-done:
				return cmd.Wait()
			default:
				pbar.Inc()
				time.Sleep(10 * time.Millisecond)
			}
		}

	} else {
		cmd.Stderr = nil
		cmd.Stdout = nil
		return cmd.Run()
	}
}
func (x *FBuildExecutor) String() string {
	return strings.Join(x.Args.Slice(), " ")
}
