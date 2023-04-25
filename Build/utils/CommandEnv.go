package utils

import (
	"fmt"
	"os"
	"time"
)

/***************************************
 * Command Flags
 ***************************************/

type CommandFlags struct {
	Force       BoolVar
	Purge       BoolVar
	Quiet       BoolVar
	Verbose     BoolVar
	Trace       BoolVar
	VeryVerbose BoolVar
	Debug       BoolVar
	Timestamp   BoolVar
	Diagnostics BoolVar
	Jobs        IntVar
	Color       BoolVar
	Ide         BoolVar
	LogFile     Filename
	OutputDir   Directory
}

var GetCommandFlags = NewGlobalCommandParsableFlags("global command options", &CommandFlags{
	Force:       INHERITABLE_FALSE,
	Purge:       INHERITABLE_FALSE,
	Quiet:       INHERITABLE_FALSE,
	Verbose:     INHERITABLE_FALSE,
	Trace:       INHERITABLE_FALSE,
	VeryVerbose: INHERITABLE_FALSE,
	Debug:       INHERITABLE_FALSE,
	Diagnostics: INHERITABLE_FALSE,
	Jobs:        INHERIT_VALUE,
	Color:       INHERITABLE_INHERIT,
	Ide:         INHERITABLE_FALSE,
	Timestamp:   INHERITABLE_FALSE,
	OutputDir:   UFS.Output,
})

func (flags *CommandFlags) Flags(cfv CommandFlagsVisitor) {
	cfv.Variable("f", "force build even if up-to-date", &flags.Force)
	cfv.Variable("F", "force build and ignore cache", &flags.Purge)
	cfv.Variable("j", "override number for worker threads (default: numCpu-1)", &flags.Jobs)
	cfv.Variable("q", "disable all messages", &flags.Quiet)
	cfv.Variable("v", "turn on verbose mode", &flags.Verbose)
	cfv.Variable("t", "print more informations about progress", &flags.Trace)
	cfv.Variable("V", "turn on very verbose mode", &flags.VeryVerbose)
	cfv.Variable("d", "turn on debug assertions and more log", &flags.Debug)
	cfv.Variable("T", "turn on timestamp logging", &flags.Timestamp)
	cfv.Variable("X", "turn on diagnostics mode", &flags.Diagnostics)
	cfv.Variable("Color", "control ansi color output in log messages", &flags.Color)
	cfv.Variable("Ide", "set output to IDE mode (disable interactive shell)", &flags.Ide)
	cfv.Variable("LogFile", "output log to specified file (default: stdout)", &flags.LogFile)
	cfv.Variable("OutputDir", "override default output directory", &flags.OutputDir)
}
func (flags *CommandFlags) Apply() {
	SetEnableDiagnostics(flags.Diagnostics.Get())
	SetLogTimestamp(flags.Timestamp.Get())

	if flags.LogFile.Valid() {
		if outp, err := UFS.CreateWriter(flags.LogFile); err == nil {
			SetEnableInteractiveShell(false)
			SetLogOutput(outp)
		} else {
			LogPanicErr(err)
		}
	}

	if !flags.Jobs.IsInheritable() && flags.Jobs.Get() > 0 {
		GetGlobalWorkerPool().Resize(flags.Jobs.Get())
	}

	if flags.Ide.Get() {
		SetLogLevelMininum(LOG_VERBOSE)
		SetEnableInteractiveShell(false)
	}
	if flags.Verbose.Get() {
		SetLogLevel(LOG_VERBOSE)
	}
	if flags.Trace.Get() {
		SetLogLevel(LOG_TRACE)
	}
	if flags.VeryVerbose.Get() {
		SetLogLevel(LOG_VERYVERBOSE)
	}
	if flags.Quiet.Get() {
		SetLogLevel(LOG_ERROR)
		SetEnableInteractiveShell(false)
	}
	if flags.Debug.Get() || EnableDiagnostics() {
		SetLogLevel(LOG_DEBUG)
		enableAssertions = true
	} else {
		enableAssertions = false
	}
	if !flags.Color.IsInheritable() {
		SetEnableAnsiColor(flags.Color.Get())
	}

	if flags.OutputDir.Valid() {
		UFS.MountOutputDir(flags.OutputDir)
	}

	if flags.Purge.Get() {
		LogTrace("command: build will be forced due to '-F' command-line option")
		flags.Force.Enable()
	}
	if flags.Force.Get() {
		LogTrace("command: fbuild will be forced due to '-f' command-line option")
	}
}

/***************************************
 * Command Env
 ***************************************/

type CommandEnvT struct {
	prefix     string
	buildGraph BuildGraph
	persistent *persistentData
	rootFile   Filename

	configPath   Filename
	databasePath Filename

	commandEvents CommandEvents
	commandLines  []CommandLine

	lastPanic error
}

var CommandEnv *CommandEnvT

func InitCommandEnv(prefix string, rootFile Filename, args []string) *CommandEnvT {
	CommandEnv = &CommandEnvT{
		prefix:     prefix,
		persistent: NewPersistentMap(prefix),
		rootFile:   rootFile,
		lastPanic:  nil,
	}

	CommandEnv.commandLines = NewCommandLine(CommandEnv.persistent, args)

	// parse global flags early-on
	for _, cl := range CommandEnv.commandLines {
		LogPanicIfFailed(GlobalParsableFlags.Parse(cl))
	}

	// apply global command flags early-on
	GetCommandFlags().Apply()

	// use UFS.Output only after having parsed -OutputDir= flags
	CommandEnv.configPath = UFS.Output.File(fmt.Sprint(".", prefix, "-config.json"))
	CommandEnv.databasePath = UFS.Output.File(fmt.Sprint(".", prefix, "-cache.db"))

	// finally create the build graph (empty)
	CommandEnv.buildGraph = NewBuildGraph(GetCommandFlags())
	return CommandEnv
}
func (env *CommandEnvT) Prefix() string             { return env.prefix }
func (env *CommandEnvT) BuildGraph() BuildGraph     { return env.buildGraph }
func (env *CommandEnvT) Persistent() PersistentData { return env.persistent }
func (env *CommandEnvT) ConfigPath() Filename       { return env.configPath }
func (env *CommandEnvT) DatabasePath() Filename     { return env.databasePath }
func (env *CommandEnvT) RootFile() Filename         { return env.rootFile }
func (env *CommandEnvT) BuildTime() time.Time       { return PROCESS_INFO.Timestamp }

// don't save the db when panic occured
func (env *CommandEnvT) OnPanic(err error) bool {
	if env.lastPanic == nil {

		env.lastPanic = err
		env.commandEvents.OnPanic.Invoke(err)
		return true
	}
	return false // a fatal error was already reported
}

func (env *CommandEnvT) Run() (result error) {
	env.buildGraph.PostLoad()

	// prepare specified commands
	for _, cl := range env.commandLines {
		if err := env.commandEvents.Parse(cl); err != nil {
			LogError("command: %s", err)
			PrintCommandHelp(os.Stderr, false)
			return nil
		}
	}

	if !env.commandEvents.Bound() {
		LogWarning("command: missing argument, use `help` to learn about command usage")
		return nil
	}

	defer func() {
		// detect futures never joined
		if err := env.buildGraph.Join(); err != nil {
			result = err
		}
	}()

	if err := env.commandEvents.Run(); err != nil {
		result = err
	}

	return
}

func (env *CommandEnvT) Load() {
	benchmark := LogBenchmark("load from disk")
	defer benchmark.Close()

	LogTrace("command: loading config from '%v'...", env.configPath)
	UFS.OpenBuffered(env.configPath, env.persistent.Deserialize)

	LogTrace("command: loading build graph from '%v'...", env.databasePath)
	if UFS.OpenBuffered(env.databasePath, env.buildGraph.Load) != nil {
		env.buildGraph.(*buildGraph).makeDirty()
	}
}
func (env *CommandEnvT) Save() {
	benchmark := LogBenchmark("save to disk")
	defer benchmark.Close()

	LogTrace("command: saving config to '%v'...", env.configPath)
	UFS.SafeCreate(env.configPath, env.persistent.Serialize)

	if env.lastPanic != nil {
		LogTrace("command: won't save build graph since a panic occured")
	} else if env.buildGraph.Dirty() {
		LogTrace("command: saving build graph to '%v'...", env.databasePath)
		UFS.SafeCreate(env.databasePath, env.buildGraph.Save)
	} else {
		LogTrace("command: skipped saving unmodified build graph")
	}
}
