package utils

import (
	"fmt"
	"strings"
	"sync"
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

type Command interface {
	Info() CommandInfo
	Init(env *CommandEnvT)
	Run(env *CommandEnvT) error
	Clean(env *CommandEnvT) error
}

type CommandFlagsT struct {
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
			panic(err)
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
}

type CommandEnvT struct {
	Flags      ServiceLocator[ParsableFlags]
	buildGraph BuildGraph
	persistent *PersistentMap
	rootFile   Filename

	barrier   sync.Mutex
	immediate CommandList
	deferred  CommandList
	instanced []Command

	configPath   Filename
	databasePath Filename
}

var CommandEnv *CommandEnvT

func InitCommandEnv(prefix string, rootFile Filename) *CommandEnvT {
	CommandEnv = &CommandEnvT{
		Flags:        NewServiceLocator[ParsableFlags](),
		buildGraph:   NewBuildGraph(),
		persistent:   NewPersistentMap(prefix),
		rootFile:     rootFile,
		barrier:      sync.Mutex{},
		immediate:    NewCommandList(),
		deferred:     NewCommandList(),
		configPath:   MAIN_MODULEPATH.Dirname.File(fmt.Sprint(".", prefix, "-config.json")),
		databasePath: MAIN_MODULEPATH.Dirname.File(fmt.Sprint(".", prefix, "-cache.db")),
	}
	CommandFlags.Add(CommandEnv.Flags)
	return CommandEnv
}
func (env *CommandEnvT) BuildGraph() BuildGraph     { return env.buildGraph }
func (env *CommandEnvT) Persistent() *PersistentMap { return env.persistent }
func (env *CommandEnvT) RootFile() Filename         { return env.rootFile }

func (env *CommandEnvT) Init(args []string) {
	if len(args) == 0 {
		LogFatal("missing command: use --help to show usage")
	}
	if factory, ok := AllCommands.Get(args[0]); ok {
		env.barrier.Lock()
		defer env.barrier.Unlock()
		cmd := factory()
		env.instanced = append(env.instanced, cmd)
		cmd.Init(env)
		for _, flags := range (*env.Flags).Values() {
			switch flags.(type) {
			case Buildable:
				env.BuildGraph().Create(flags.(Buildable))
			}
		}
		env.persistent.Parse(args[1:], (*env.Flags).Values()...)
		LogClaim("running command <%v>", cmd.Info().Name)
		env.immediate = append(env.immediate, CommandFunc{cmd.Info(), cmd.Run})
		env.deferred = append(env.deferred, CommandFunc{cmd.Info(), cmd.Clean})
	} else {
		CommandFlags.Get(env.Flags).InitFlags(env.persistent)
		env.Persistent().Usage()
		LogFatal("invalid command '%v', available names: %v", args[0], strings.Join(AllCommands.Keys(), ", "))
	}
}
func (env *CommandEnvT) Run() error {
	LogTrace("running immediate tasks (%v elts)", len(env.immediate))
	if err := env.immediate.Run(env); err != nil {
		return err
	}
	LogTrace("running deferred tasks (%v elts)", len(env.immediate))
	if err := env.deferred.Run(env); err != nil {
		return err
	}
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

	if env.buildGraph.Dirty() {
		LogTrace("saving build graph to '%v'...", env.databasePath)
		UFS.Create(env.databasePath, env.buildGraph.Serialize)
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
					args.InitFlags(env.Persistent())
				}
				return args
			},
			run: func(env *CommandEnvT, args T) error {
				args.ApplyVars(env.Persistent())
				return run(env, args)
			},
			clean: nil,
		}
	})
}
