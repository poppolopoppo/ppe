package utils

import (
	"strings"
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
func (x *FBuildCacheType) Set(in string) (err error) {
	switch strings.ToUpper(in) {
	case FBUILD_CACHE_DISABLED.String():
		*x = FBUILD_CACHE_DISABLED
	case FBUILD_CACHE_READ.String():
		*x = FBUILD_CACHE_READ
	case FBUILD_CACHE_WRITE.String():
		*x = FBUILD_CACHE_WRITE
	default:
		err = MakeUnexpectedValueError(x, in)
	}
	return err
}
func (x *FBuildCacheType) Serialize(ar Archive) {
	ar.Int32((*int32)(x))
}
func (x FBuildCacheType) MarshalText() ([]byte, error) {
	return []byte(x.String()), nil
}
func (x *FBuildCacheType) UnmarshalText(data []byte) error {
	return x.Set(string(data))
}

/***************************************
 * FBuild Args
 ***************************************/

type FBuildArgs struct {
	BffInput      Filename
	Cache         FBuildCacheType
	Clean         BoolVar
	Dist          BoolVar
	NoStopOnError BoolVar
	NoUnity       BoolVar
	Report        BoolVar
	ShowCmds      BoolVar
	ShowCmdOutput BoolVar
	Threads       IntVar
}

var BFF_DEFAULT_BASENAME = `fbuild.bff`

var GetFBuildArgs = NewCommandParsableFlags(&FBuildArgs{
	Cache:         FBUILD_CACHE_DISABLED,
	Clean:         INHERITABLE_FALSE,
	Dist:          INHERITABLE_FALSE,
	NoUnity:       INHERITABLE_FALSE,
	NoStopOnError: INHERITABLE_TRUE,
	Report:        INHERITABLE_FALSE,
	ShowCmds:      INHERITABLE_FALSE,
	Threads:       0,
})

func (flags *FBuildArgs) Flags(cfv CommandFlagsVisitor) {
	cfv.Persistent("Cache", "set FASTBuild cache mode ["+JoinString(",", FBuildCacheTypes()...)+"]", &flags.Cache)
	cfv.Variable("Clean", "FASTBuild will rebuild all cached artifacts if enabled", &flags.Clean)
	cfv.Persistent("Dist", "enable/disable FASTBuild to use distributed compilation", &flags.Dist)
	cfv.Persistent("BffInput", "source for input FASTBuild config file (*.bff)", &flags.BffInput)
	cfv.Persistent("NoStopOnError", "FASTBuild will stop compiling on the first error if enabled", &flags.NoStopOnError)
	cfv.Variable("NoUnity", "enable/disable use of generated unity source files", &flags.NoUnity)
	cfv.Variable("Report", "enable/disable FASTBuild compilation report generation", &flags.Report)
	cfv.Variable("ShowCmds", "display all command-lines executed by FASTBuild", &flags.ShowCmds)
	cfv.Variable("ShowCmdOutput", "display full output of external processes regardless of outcome.", &flags.ShowCmdOutput)
	cfv.Persistent("Threads", "set count of worker thread spawned by FASTBuild", &flags.Threads)
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
		if flags.Dist.Get() {
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
		if flags.ShowCmds.Get() {
			result.Args.Append("-showcmds")
		}
		if flags.ShowCmdOutput.Get() {
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
	LogVerbose("fbuild: running with '%v' (capture=%v)", x, x.Capture)

	return RunProcess(FBUILD_BIN, x.Args.Slice(),
		OptionProcessExport("FASTBUILD_CACHE_PATH", UFS.Cache.String()),
		OptionProcessExport("FASTBUILD_TEMP_PATH", UFS.Transient.String()),
		OptionProcessWorkingDir(UFS.Root),
		OptionProcessCaptureOutputIf(x.Capture))
}
func (x *FBuildExecutor) String() string {
	return strings.Join(x.Args.Slice(), " ")
}
