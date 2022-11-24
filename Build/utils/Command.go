package utils

import (
	"fmt"
	"strings"
	"sync"
	"time"
)

var AllCommands *SharedMapT[string, CommandFactory] = NewSharedMapT[string, CommandFactory]()

func NewCommandList() CommandList {
	return CommandList{}
}

type CommandFactory func() Command
type CommandFunc struct {
	Info  CommandInfo
	Event func(*CommandEnvT) error
}
type CommandList []CommandFunc

func (list *CommandList) Pop(barrier *sync.Mutex) CommandFunc {
	barrier.Lock()
	defer barrier.Unlock()
	if len(*list) > 0 {
		result := (*list)[0]
		*list = (*list)[1:]
		return result
	} else {
		return CommandFunc{}
	}
}
func (list *CommandList) Run(env *CommandEnvT) error {
	for {
		if run := list.Pop(&env.barrier); run.Event != nil {
			if err := run.Event(env); err != nil {
				LogError("%v: %v", run.Info.Name, err)
				return err
			}
		} else {
			break
		}
	}
	return nil
}

type CommandInfo struct {
	Name, Usage string
}
type commandArgs struct {
	Unparsed []string
}

func (x *commandArgs) Consume(n int) []string {
	if n >= 0 {
		result := x.Unparsed[:n]
		x.Unparsed = x.Unparsed[n+1:]
		return result
	} else {
		result := x.Unparsed
		x.Unparsed = []string{}
		return result
	}
}

type Command interface {
	Info() CommandInfo
	Init(env *CommandEnvT)
	Run(env *CommandEnvT) error
	Clean(env *CommandEnvT) error
}

type CommandFlagsT struct {
	Force       bool
	Purge       bool
	Quiet       bool
	Verbose     bool
	Trace       bool
	VeryVerbose bool
	Debug       bool
	Timestamp   bool
	Diagnostics bool
	LogFile     Filename
}

var CommandFlags = MakeServiceAccessor[ParsableFlags](newCommandFlags)

func newCommandFlags() *CommandFlagsT {
	return &CommandFlagsT{
		Force:       false,
		Quiet:       false,
		Verbose:     false,
		Trace:       false,
		VeryVerbose: false,
		Debug:       false,
		Diagnostics: false,
		Timestamp:   false,
	}
}
func (flags *CommandFlagsT) InitFlags(cfg *PersistentMap) {
	cfg.BoolVar(&flags.Force, "f", "force build even if up-to-date")
	cfg.BoolVar(&flags.Purge, "F", "force build and ignore cache")
	cfg.BoolVar(&flags.Quiet, "q", "disable all messages")
	cfg.BoolVar(&flags.Verbose, "v", "turn on verbose mode")
	cfg.BoolVar(&flags.Trace, "t", "print more informations about progress")
	cfg.BoolVar(&flags.VeryVerbose, "V", "turn on very verbose mode")
	cfg.BoolVar(&flags.Debug, "d", "turn on debug assertions and more log")
	cfg.BoolVar(&flags.Timestamp, "T", "turn on timestamp logging")
	cfg.BoolVar(&flags.Diagnostics, "X", "turn on diagnostics mode")
	cfg.Var(&flags.LogFile, "LogFile", "output log to specified file (default: stdout)")
}
func (flags *CommandFlagsT) ApplyVars(persistent *PersistentMap) {
	SetEnableDiagnostics(flags.Diagnostics)
	if flags.LogFile.Basename != "" {
		if outp, err := UFS.CreateWriter(flags.LogFile); err == nil {
			SetEnableInteractiveShell(false)
			SetLogOutput(outp)
		} else {
			LogPanicErr(err)
		}
	}
	if flags.Quiet {
		logger.Level = LOG_ERROR
		enableInteractiveShell = false
	}
	if flags.Verbose {
		logger.Level = LOG_VERBOSE
	}
	if flags.Trace {
		logger.Level = LOG_TRACE
	}
	if flags.VeryVerbose {
		logger.Level = LOG_VERYVERBOSE
	}
	if flags.Debug || EnableDiagnostics() {
		logger.Level = LOG_DEBUG
		enableAssertions = true
	} else {
		enableAssertions = false
	}
	SetLogTimestamp(flags.Timestamp)

	if flags.Purge {
		LogTrace("build will be forced due to '-F' command-line option")
		flags.Force = true
	}
	if flags.Force {
		LogTrace("build will be forced due to '-f' command-line option")
	}
}

type CommandEnvT struct {
	Flags      ServiceLocator[ParsableFlags]
	prefix     string
	buildGraph BuildGraph
	persistent *PersistentMap
	rootFile   Filename

	barrier   sync.Mutex
	immediate CommandList
	deferred  CommandList
	unparsed  *commandArgs

	configPath   Filename
	databasePath Filename

	lastPanic error
}

var CommandEnv *CommandEnvT

func InitCommandEnv(prefix string, rootFile Filename) *CommandEnvT {
	CommandEnv = &CommandEnvT{
		Flags:        NewServiceLocator[ParsableFlags](),
		prefix:       prefix,
		persistent:   NewPersistentMap(prefix),
		rootFile:     rootFile,
		barrier:      sync.Mutex{},
		immediate:    NewCommandList(),
		deferred:     NewCommandList(),
		configPath:   MAIN_MODULEPATH.Dirname.File(fmt.Sprint(".", prefix, "-config.json")),
		databasePath: MAIN_MODULEPATH.Dirname.File(fmt.Sprint(".", prefix, "-cache.db")),
		lastPanic:    nil,
	}
	CommandEnv.buildGraph = NewBuildGraph(CommandFlags.Create(CommandEnv.Flags))
	return CommandEnv
}
func (env *CommandEnvT) Prefix() string             { return env.prefix }
func (env *CommandEnvT) BuildGraph() BuildGraph     { return env.buildGraph }
func (env *CommandEnvT) Persistent() *PersistentMap { return env.persistent }
func (env *CommandEnvT) ConfigPath() Filename       { return env.configPath }
func (env *CommandEnvT) DatabasePath() Filename     { return env.databasePath }
func (env *CommandEnvT) RootFile() Filename         { return env.rootFile }
func (env *CommandEnvT) BuildTime() time.Time       { return UFS.MTime(UFS.Executable) }

// don't save the db when panic occured
func (env *CommandEnvT) OnPanic(err error) bool {
	if env.lastPanic == nil {
		env.lastPanic = err
		return true
	}
	return false // a fatal error was already reported
}

func (env *CommandEnvT) Init(args []string) {
	if len(args) == 0 {
		LogFatal("missing command: use --help to show usage")
	}

	env.barrier.Lock()
	defer env.barrier.Unlock()

	if factory, ok := AllCommands.Get(args[0]); ok {
		cmd := factory()
		cmd.Init(env)

		LogVerbose("running command <%v>", cmd.Info().Name)

		freeArgs := &commandArgs{
			Unparsed: env.persistent.Parse(args[1:], env.Flags.Values()...),
		}

		env.deferred = append(env.deferred, CommandFunc{cmd.Info(), cmd.Clean})
		env.immediate = append(env.immediate, CommandFunc{cmd.Info(), func(cet *CommandEnvT) error {
			cet.unparsed = freeArgs
			err := cmd.Run(cet)
			cet.unparsed = nil

			if err != nil && len(freeArgs.Unparsed) > 0 {
				LogWarning("%v: unused arguments %v", args[0], strings.Join(freeArgs.Unparsed, ", "))
			}

			return err
		}})

	} else {
		env.persistent.resetFlagSet()

		CommandFlags.FindOrAdd(env.Flags).InitFlags(env.persistent)

		env.Persistent().Usage()

		LogFatal("invalid command '%v', available names: %v", args[0], strings.Join(AllCommands.Keys(), ", "))
	}
}
func (env *CommandEnvT) ConsumeArgs(n int) []string {
	return env.unparsed.Consume(n)
}
func (env *CommandEnvT) Run() error {
	env.buildGraph.PostLoad()

	for _, flags := range env.Flags.Values() {
		switch value := flags.(type) {
		case Buildable:
			env.BuildGraph().ForceBuild(value)
		}
	}

	LogTrace("running immediate tasks (%v elts)", len(env.immediate))
	if err := env.immediate.Run(env); err != nil {
		return err
	}
	LogTrace("running deferred tasks (%v elts)", len(env.immediate))
	if err := env.deferred.Run(env); err != nil {
		return err
	}

	env.buildGraph.Join()
	return nil
}

func (env *CommandEnvT) Load() {
	benchmark := LogBenchmark("load from disk")
	defer benchmark.Close()

	LogTrace("loading config from '%v'...", env.configPath)
	UFS.Open(env.configPath, env.persistent.Deserialize)
	LogTrace("loading build graph from '%v'...", env.databasePath)
	UFS.Open(env.databasePath, env.buildGraph.Deserialize)
}
func (env *CommandEnvT) Save() {
	benchmark := LogBenchmark("save to disk")
	defer benchmark.Close()

	LogTrace("saving config to '%v'...", env.configPath)
	UFS.Create(env.configPath, env.persistent.Serialize)

	if env.lastPanic != nil {
		LogTrace("won't save build graph since a panic occured")
	} else if env.buildGraph.Dirty() {
		LogTrace("saving build graph to '%v'...", env.databasePath)
		UFS.Create(env.databasePath, env.buildGraph.Serialize)
	} else {
		LogTrace("skipped saving unmodified build graph")
	}
}
func (env *CommandEnvT) ReadData(name string) (string, bool) {
	if x, ok := env.persistent.Data[name]; ok {
		return x, true
	} else {
		return "", false
	}
}
func (env *CommandEnvT) ReadVar(name string) (PersistentVar, bool) {
	if x, ok := env.persistent.Vars[name]; ok {
		return x, true
	} else {
		return nil, false
	}
}

type GenericCommand[T ParsableFlags] struct {
	args  T
	info  CommandInfo
	init  func(env *CommandEnvT) T
	run   func(env *CommandEnvT, args T) error
	clean func(env *CommandEnvT, args T) error
}

func (cmd *GenericCommand[T]) Args() T           { return cmd.args }
func (cmd *GenericCommand[T]) Info() CommandInfo { return cmd.info }
func (cmd *GenericCommand[T]) Init(env *CommandEnvT) {
	if cmd.init != nil {
		cmd.args = cmd.init(env)
	}
}
func (cmd *GenericCommand[T]) Run(env *CommandEnvT) error {
	return cmd.run(env, cmd.args)
}
func (cmd *GenericCommand[T]) Clean(env *CommandEnvT) error {
	if cmd.clean != nil {
		return cmd.clean(env, cmd.args)
	} else {
		return nil
	}
}

func MakeCommand[T ParsableFlags](
	name, usage string,
	init func(env *CommandEnvT) T,
	run func(env *CommandEnvT, args T) error,
) CommandFactory {
	if run == nil {
		UnexpectedValue(run)
	}
	return AllCommands.Add(name, func() Command {
		return &GenericCommand[T]{
			info: CommandInfo{
				Name:  name,
				Usage: usage,
			},
			init: func(env *CommandEnvT) (args T) {
				if init != nil {
					args = init(env)
				}
				return args
			},
			run: func(env *CommandEnvT, args T) error {
				return run(env, args)
			},
			clean: nil,
		}
	})
}
