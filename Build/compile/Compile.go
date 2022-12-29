package compile

import (
	. "build/utils"
	"bytes"
	"encoding/gob"
	"sort"
)

var AllCompilationFlags = NewServiceEvent[ParsableFlags]()

func InitCompile() {
	LogTrace("build/compile.Init()")

	// register type for serialization
	gob.Register(Facet{})
	gob.Register(ModuleAlias{})
	gob.Register(&Unit{})
	gob.Register(CustomUnit{})

	gob.Register(&CompilerRules{})
	gob.Register(&ConfigRules{})
	gob.Register(&PlatformRules{})
	gob.Register(&ModuleRules{})
	gob.Register(&GeneratedRules{})
	gob.Register(&NamespaceRules{})

	gob.Register(&CompileFlagsT{})

	gob.Register(&BuildConfigsT{})
	gob.Register(&BuildPlatformsT{})
	gob.Register(&BuildModulesT{})
	gob.Register(&BuildEnvironmentsT{})
	gob.Register(&EnvironmentTargetsT{})
	gob.Register(&BuildTargetsT{})

	AllCompilationFlags.Append(CompileFlags.Add)

	AllConfigurations.Add("Debug", Configuration_Debug)
	AllConfigurations.Add("FastDebug", Configuration_FastDebug)
	AllConfigurations.Add("Devel", Configuration_Devel)
	AllConfigurations.Add("Test", Configuration_Test)
	AllConfigurations.Add("Shipping", Configuration_Shipping)
}

type CompileFlagsT CppRules

var CompileFlags = MakeServiceAccessor[ParsableFlags](newCompileFlags)

func newCompileFlags() *CompileFlagsT {
	return CommandEnv.BuildGraph().Create(&CompileFlagsT{
		CppRtti:       CPPRTTI_INHERIT,
		CppStd:        CPPSTD_INHERIT,
		DebugSymbols:  DEBUG_INHERIT,
		Exceptions:    EXCEPTION_INHERIT,
		Link:          LINK_INHERIT,
		PCH:           PCH_INHERIT,
		Sanitizer:     SANITIZER_NONE,
		Unity:         UNITY_INHERIT,
		SizePerUnity:  300 * 1024.0, // 300 KiB
		AdaptiveUnity: INHERITABLE_TRUE,
		Benchmark:     INHERITABLE_FALSE,
		LTO:           INHERITABLE_TRUE,
		Incremental:   INHERITABLE_TRUE,
		RuntimeChecks: INHERITABLE_TRUE,
	}).GetBuildable().(*CompileFlagsT)
}
func (flags *CompileFlagsT) InitFlags(cfg *PersistentMap) {
	cfg.Persistent(&flags.CppRtti, "CppRtti", "override C++ rtti support ["+JoinString(",", CppRttiTypes()...)+"]")
	cfg.Persistent(&flags.CppStd, "CppStd", "override C++ standard ["+JoinString(",", CppStdTypes()...)+"]")
	cfg.Persistent(&flags.DebugSymbols, "DebugSymbols", "override debug symbols mode ["+JoinString(",", DebugTypes()...)+"]")
	cfg.Persistent(&flags.Exceptions, "Exceptions", "override exceptions mode ["+JoinString(",", ExceptionTypes()...)+"]")
	cfg.Persistent(&flags.Link, "Link", "override link type ["+JoinString(",", LinkTypes()...)+"]")
	cfg.Persistent(&flags.PCH, "PCH", "override size limit for splitting unity files ["+JoinString(",", PrecompiledHeaderTypes()...)+"]")
	cfg.Persistent(&flags.Sanitizer, "Sanitizer", "override sanitizer mode ["+JoinString(",", SanitizerTypes()...)+"]")
	cfg.Persistent(&flags.Unity, "Unity", "override unity build mode ["+JoinString(",", UnityTypes()...)+"]")
	cfg.Persistent(&flags.SizePerUnity, "SizePerUnity", "size limit for splitting unity files")
	cfg.Persistent(&flags.AdaptiveUnity, "AdaptiveUnity", "enable/disable adaptive unity using source control")
	cfg.Persistent(&flags.Benchmark, "Benchmark", "enable/disable compilation benchmarks")
	cfg.Persistent(&flags.LTO, "LTO", "enable/disable link time optimization")
	cfg.Persistent(&flags.Incremental, "Incremental", "enable/disable incremental linker")
	cfg.Persistent(&flags.RuntimeChecks, "RuntimeChecks", "enable/disable runtime security checks")
}
func (flags *CompileFlagsT) ApplyVars(cfg *PersistentMap) {
}

func (flags *CompileFlagsT) Alias() BuildAlias {
	return MakeBuildAlias("Flags", "CompileFlags")
}
func (flags *CompileFlagsT) Build(BuildContext) (BuildStamp, error) {
	return MakeBuildStamp(flags)
}
func (flags *CompileFlagsT) GetDigestable(o *bytes.Buffer) {
	(*CppRules)(flags).GetDigestable(o)
}

type EnvironmentTargetsT struct {
	Environment EnvironmentAlias
	SetT[*Unit]
}

func (e *EnvironmentTargetsT) Alias() BuildAlias {
	return MakeBuildAlias("Targets", e.Environment.Alias().String())
}
func (e *EnvironmentTargetsT) Build(bc BuildContext) (BuildStamp, error) {
	e.SetT.Clear()

	allModules := BuildModules.Need(bc)
	compileEnv := BuildEnvironments.Need(bc).GetEnvironment(e.Environment)

	moduleGraph := NewModuleGraph(compileEnv, allModules)
	moduleGraph.CompileUnits()

	if units, err := compileEnv.Link(bc, moduleGraph); err == nil {
		e.SetT = units
		return MakeBuildStamp(e)
	} else {
		return BuildStamp{}, err
	}
}
func (e *EnvironmentTargetsT) GetDigestable(o *bytes.Buffer) {
	e.Environment.GetDigestable(o)
	for _, unit := range e.SetT {
		unit.GetDigestable(o)
	}
}

func getEnvironmentTargets(environment EnvironmentAlias) *EnvironmentTargetsT {
	result := &EnvironmentTargetsT{
		Environment: environment,
	}
	return CommandEnv.BuildGraph().
		Create(result).
		GetBuildable().(*EnvironmentTargetsT)
}

type BuildTargetsT struct {
	Aliases SetT[EnvironmentAlias]
}

func (b *BuildTargetsT) Alias() BuildAlias {
	return MakeBuildAlias("Build", "TranslatedUnits")
}
func (b *BuildTargetsT) Build(bc BuildContext) (BuildStamp, error) {
	compileEnvs := BuildEnvironments.Need(bc)

	b.Aliases.Clear()
	targets := Map(func(compileEnv *CompileEnv) BuildAliasable {
		it := getEnvironmentTargets(compileEnv.EnvironmentAlias())
		b.Aliases.Append(it.Environment)
		return it
	}, compileEnvs.Slice()...)

	bc.DependsOn(targets...)

	sort.SliceStable(b.Aliases, func(i, j int) bool {
		return b.Aliases[i].Compare(b.Aliases[j]) < 0
	})

	return MakeBuildStamp(b)
}
func (b *BuildTargetsT) GetDigestable(o *bytes.Buffer) {
	for _, a := range b.TranslatedUnits().Slice() {
		a.GetDigestable(o)
	}
}
func (b *BuildTargetsT) TranslatedUnits() SetT[*Unit] {
	units := NewSet[*Unit]()
	for _, a := range b.Aliases {
		units.Append(getEnvironmentTargets(a).Slice()...)
	}
	return units
}

var BuildTargets = MakeBuildable(func(bi BuildInit) *BuildTargetsT {
	return &BuildTargetsT{}
})
