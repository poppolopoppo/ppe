package compile

import (
	"build/utils"
	"bytes"
	"encoding/gob"
	"sync"
)

var AllCompilationFlags = utils.NewServiceEvent[utils.ParsableFlags]()

func InitCompile() {
	utils.LogTrace("build/compile.Init()")

	// register type for serialization
	gob.Register(Facet{})
	gob.Register(&Unit{})

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
	gob.Register(&BuildTargetsT{})

	AllCompilationFlags.Append(CompileFlags.Add)

	AllConfigurations.Add("Debug", Configuration_Debug)
	AllConfigurations.Add("FastDebug", Configuration_FastDebug)
	AllConfigurations.Add("Devel", Configuration_Devel)
	AllConfigurations.Add("Test", Configuration_Test)
	AllConfigurations.Add("Shipping", Configuration_Shipping)
}

type CompileFlagsT struct {
	CppRtti       CppRttiType
	CppStd        CppStdType
	DebugSymbols  DebugType
	Exceptions    ExceptionType
	Link          LinkType
	PCH           PrecompiledHeaderType
	Sanitizer     SanitizerType
	Unity         UnityType
	SizePerUnity  utils.IntVar
	AdaptiveUnity utils.BoolVar
	Benchmark     utils.BoolVar
	LTO           utils.BoolVar
	Incremental   utils.BoolVar
	RuntimeChecks utils.BoolVar
}

var CompileFlags = utils.MakeServiceAccessor[utils.ParsableFlags](newCompileFlags)

func newCompileFlags() *CompileFlagsT {
	return utils.CommandEnv.BuildGraph().Create(&CompileFlagsT{
		CppRtti:       CPPRTTI_INHERIT,
		CppStd:        CPPSTD_INHERIT,
		DebugSymbols:  DEBUG_INHERIT,
		Exceptions:    EXCEPTION_INHERIT,
		Link:          LINK_INHERIT,
		PCH:           PCH_INHERIT,
		Sanitizer:     SANITIZER_NONE,
		Unity:         UNITY_INHERIT,
		SizePerUnity:  300 * 1024.0, // 300 KiB
		AdaptiveUnity: true,
		LTO:           true,
		Incremental:   true,
		RuntimeChecks: true,
	}).GetBuildable().(*CompileFlagsT)
}
func (flags *CompileFlagsT) InitFlags(cfg *utils.PersistentMap) {
	cfg.Persistent(&flags.CppRtti, "CppRtti", "override C++ rtti support ["+utils.JoinString(",", CppRttiTypes()...)+"]")
	cfg.Persistent(&flags.CppStd, "CppStd", "override C++ standard ["+utils.JoinString(",", CppStdTypes()...)+"]")
	cfg.Persistent(&flags.DebugSymbols, "DebugSymbols", "override debug symbols mode ["+utils.JoinString(",", DebugTypes()...)+"]")
	cfg.Persistent(&flags.Exceptions, "Exceptions", "override exceptions mode ["+utils.JoinString(",", ExceptionTypes()...)+"]")
	cfg.Persistent(&flags.Link, "Link", "override link type ["+utils.JoinString(",", LinkTypes()...)+"]")
	cfg.Persistent(&flags.PCH, "PCH", "override size limit for splitting unity files ["+utils.JoinString(",", PrecompiledHeaderTypes()...)+"]")
	cfg.Persistent(&flags.Sanitizer, "Sanitizer", "override sanitizer mode ["+utils.JoinString(",", SanitizerTypes()...)+"]")
	cfg.Persistent(&flags.Unity, "Unity", "override unity build mode ["+utils.JoinString(",", UnityTypes()...)+"]")
	cfg.Persistent(&flags.SizePerUnity, "SizePerUnity", "size limit for splitting unity files")
	cfg.Persistent(&flags.AdaptiveUnity, "AdaptiveUnity", "enable/disable adaptive unity using source control")
	cfg.Persistent(&flags.Benchmark, "Benchmark", "enable/disable compilation benchmarks")
	cfg.Persistent(&flags.LTO, "LTO", "enable/disable link time optimization")
	cfg.Persistent(&flags.Incremental, "Incremental", "enable/disable incremental linker")
	cfg.Persistent(&flags.RuntimeChecks, "RuntimeChecks", "enable/disable runtime security checks")
}
func (flags *CompileFlagsT) ApplyVars(cfg *utils.PersistentMap) {
}

func (flags *CompileFlagsT) Alias() utils.BuildAlias {
	return utils.MakeBuildAlias("Flags", "CompileFlags")
}
func (flags *CompileFlagsT) Build(utils.BuildContext) (utils.BuildStamp, error) {
	//flags.InitFlags(utils.CommandEnv.Persistent())
	return utils.MakeBuildStamp(flags)
}
func (flags *CompileFlagsT) GetDigestable(o *bytes.Buffer) {
	flags.CppRtti.GetDigestable(o)
	flags.CppStd.GetDigestable(o)
	flags.DebugSymbols.GetDigestable(o)
	flags.Link.GetDigestable(o)
	flags.PCH.GetDigestable(o)
	flags.Sanitizer.GetDigestable(o)
	flags.Unity.GetDigestable(o)
	flags.SizePerUnity.GetDigestable(o)
	flags.AdaptiveUnity.GetDigestable(o)
	flags.Benchmark.GetDigestable(o)
	flags.LTO.GetDigestable(o)
	flags.Incremental.GetDigestable(o)
	flags.RuntimeChecks.GetDigestable(o)
}

type BuildTargetsT struct {
	utils.SetT[*Unit]
}

func (b *BuildTargetsT) Alias() utils.BuildAlias {
	return utils.MakeBuildAlias("Build", "TranslatedUnits")
}
func (b *BuildTargetsT) Build(bc utils.BuildContext) (utils.BuildStamp, error) {
	pbar := utils.LogProgress(0, 0, b.Alias().String())
	defer pbar.Close()

	b.Clear()

	compileEnvs := BuildEnvironments.Need(bc)
	allModules := BuildModules.Need(bc)
	moduleGraph := GetModuleGraph(allModules)

	var pendingCompile []utils.Future[[]*Unit]
	for _, x := range compileEnvs.Slice() {
		compileEnv := x
		pbar.Add(len(moduleGraph.Keys()))
		pendingCompile = append(pendingCompile, utils.MakeFuture(func() ([]*Unit, error) {
			units := utils.NewSharedMapT[Module, *Unit]()
			wg := sync.WaitGroup{}
			wg.Add(len(moduleGraph.Keys()))
			for _, module := range moduleGraph.Keys() {
				go func(compileEnv *CompileEnv, module Module) {
					units.Add(module, compileEnv.Compile(module))
					wg.Done()
					pbar.Inc()
				}(compileEnv, module)
			}
			wg.Wait()
			return compileEnv.Link(bc, moduleGraph, units.Pin())
		}))
	}

	for _, x := range pendingCompile {
		units := x.Join().Success()
		b.Append(units...)
		for _, u := range units {
			bc.DependsOn(u.Compiler)
		}
	}

	o := bytes.Buffer{}
	digest := utils.MapDigest(b.Len(), func(i int) []byte {
		o.Reset()
		b.At(i).GetDigestable(&o)
		return o.Bytes()
	})

	utils.LogTrace("translated %d compilation units", b.Len())
	return utils.MakeBuildStamp(digest)
}

var BuildTargets = utils.MakeBuildable(func(bi utils.BuildInit) *BuildTargetsT {
	return &BuildTargetsT{}
})
