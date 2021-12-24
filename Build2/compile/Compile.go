package compile

import (
	"build/utils"
	"bytes"
	"encoding/gob"
	"fmt"
	"strings"
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
	gob.Register(&NamespaceRules{})

	gob.Register(&BuildConfigsT{})
	gob.Register(&BuildPlatformsT{})
	gob.Register(&BuildModulesT{})
	gob.Register(&BuildTranslatedUnitsT{})
	gob.Register(&CompileFlagsT{})

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
	return &CompileFlagsT{
		CppRtti:       CPPRTTI_INHERIT,
		CppStd:        CPPSTD_INHERIT,
		DebugSymbols:  DEBUG_INHERIT,
		Link:          LINK_INHERIT,
		PCH:           PCH_INHERIT,
		Sanitizer:     SANITIZER_NONE,
		Unity:         UNITY_INHERIT,
		SizePerUnity:  300 * 1024.0, // 300 KiB
		AdaptiveUnity: true,
		LTO:           true,
		Incremental:   true,
		RuntimeChecks: true,
	}
}
func (flags *CompileFlagsT) InitFlags(cfg *utils.PersistentMap) {
	cfg.Persistent(&flags.CppRtti, "CppRtti", "override C++ rtti support ["+utils.Join(",", CppRttiTypes()...)+"]")
	cfg.Persistent(&flags.CppStd, "CppStd", "override C++ standard ["+utils.Join(",", CppStdTypes()...)+"]")
	cfg.Persistent(&flags.DebugSymbols, "DebugSymbols", "override debug symbols mode ["+utils.Join(",", DebugTypes()...)+"]")
	cfg.Persistent(&flags.Link, "Link", "override link type ["+utils.Join(",", LinkTypes()...)+"]")
	cfg.Persistent(&flags.PCH, "PCH", "override size limit for splitting unity files ["+utils.Join(",", PrecompiledHeaderTypes()...)+"]")
	cfg.Persistent(&flags.Sanitizer, "Sanitizer", "override sanitizer mode ["+utils.Join(",", SanitizerTypes()...)+"]")
	cfg.Persistent(&flags.Unity, "Unity", "override unity build mode ["+utils.Join(",", UnityTypes()...)+"]")
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

type CompileEnv struct {
	Platform
	Configuration
	Compiler
	*ModuleGraph
	*CompileFlagsT
	Facet

	translatedUnits *utils.SharedMapT[Module, *Unit]
}

func NewCompileEnv(
	platform Platform,
	config Configuration,
	compiler Compiler,
	moduleGraph *ModuleGraph,
	compileFlags *CompileFlagsT) (result *CompileEnv) {
	result = &CompileEnv{
		Platform:        platform,
		Configuration:   config,
		Compiler:        compiler,
		ModuleGraph:     moduleGraph,
		CompileFlagsT:   compileFlags,
		Facet:           NewFacet(),
		translatedUnits: utils.NewSharedMapT[Module, *Unit](),
	}

	result.Facet.Defines.Append(
		"BUILD_ENVIRONMENT="+result.String(),
		"BUILD_PLATFORM="+result.GetPlatform().PlatformName,
		"BUILD_CONFIG="+result.GetConfig().ConfigName,
		"BUILD_COMPILER="+result.GetCompiler().CompilerName,
		"BUILD_FAMILY="+strings.Join(result.Family(), "-"),
		"BUILD_"+result.String())

	result.Facet.IncludePaths.Append(utils.UFS.Source)

	result.Facet.Append(
		result.GetPlatform(),
		result.GetConfig(),
		result.GetCompiler())

	utils.LogDebug("%s: %s", result, &result.Facet)

	return result
}

func (env *CompileEnv) Family() []string {
	return []string{env.GetPlatform().PlatformName, env.GetConfig().ConfigName}
}
func (env *CompileEnv) String() string {
	return strings.Join(append([]string{env.GetCompiler().CompilerName}, env.Family()...), "_")
}

func (env *CompileEnv) GetPlatform() *PlatformRules { return env.Platform.GetPlatform() }
func (env *CompileEnv) GetConfig() *ConfigRules     { return env.Configuration.GetConfig() }
func (env *CompileEnv) GetCompiler() *CompilerRules { return env.Compiler.GetCompiler() }
func (env *CompileEnv) GetFacet() *Facet            { return &env.Facet }

func (env *CompileEnv) Extname(payload PayloadType) string { return env.Compiler.Extname(payload) }
func (env *CompileEnv) GeneratedDir() utils.Directory {
	return utils.UFS.Generated.Folder(env.Family()...)
}
func (env *CompileEnv) IntermediateDir() utils.Directory {
	return utils.UFS.Intermediate.Folder(env.Family()...)
}
func (env *CompileEnv) GetCppRtti(module Module) (result CppRttiType) {
	if result = env.CompileFlagsT.CppRtti; CPPRTTI_INHERIT != result {
		return result
	} else if result = module.GetModule().CppRtti; CPPRTTI_INHERIT != result {
		return result
	} else {
		return env.GetConfig().CppRtti
	}
}
func (env *CompileEnv) GetCppStd(module Module) (result CppStdType) {
	if result = env.CompileFlagsT.CppStd; CPPSTD_INHERIT != result {
		return result
	} else if result = module.GetModule().CppStd; CPPSTD_INHERIT != result {
		return result
	} else {
		return env.GetCompiler().CppStd
	}
}
func (env *CompileEnv) GetDebugType(module Module) (result DebugType) {
	if result = env.CompileFlagsT.DebugSymbols; DEBUG_INHERIT != result {
		return result
	} else if result = module.GetModule().Debug; DEBUG_INHERIT != result {
		return result
	} else {
		return env.GetConfig().Debug
	}
}
func (env *CompileEnv) GetPCHType(module Module) (result PrecompiledHeaderType) {
	if result = env.PCH; PCH_INHERIT != result {
		return result
	} else if result = module.GetModule().PCH; PCH_INHERIT != result {
		return result
	} else {
		return env.GetConfig().PCH
	}
}
func (env *CompileEnv) GetLinkType(module Module) (result LinkType) {
	if result = env.CompileFlagsT.Link; LINK_INHERIT != result {
		return result
	} else if result = module.GetModule().Link; LINK_INHERIT != result {
		return result
	} else {
		return env.GetConfig().Link
	}
}
func (env *CompileEnv) GetUnityType(module Module) (result UnityType) {
	if result = env.Unity; UNITY_INHERIT != result {
		return result
	} else if result = module.GetModule().Unity; UNITY_INHERIT != result {
		return result
	} else {
		return env.GetConfig().Unity
	}
}
func (env *CompileEnv) GetSanitizerType(module Module) (result SanitizerType) {
	if result = env.CompileFlagsT.Sanitizer; SANITIZER_INHERIT != result {
		return result
	} else if result = module.GetModule().Sanitizer; SANITIZER_INHERIT != result {
		return result
	} else {
		return env.GetConfig().Sanitizer
	}
}
func (env *CompileEnv) GetPayloadType(module Module, link LinkType) (result PayloadType) {
	switch module.GetModule().ModuleType {
	case MODULE_EXTERNAL:
		fallthrough
	case MODULE_LIBRARY:
		switch link {
		case LINK_INHERIT:
			fallthrough
		case LINK_STATIC:
			return PAYLOAD_STATICLIB
		case LINK_DYNAMIC:
			return PAYLOAD_SHAREDLIB
		default:
			utils.UnexpectedValue(link)
		}
		break
	case MODULE_PROGRAM:
		switch link {
		case LINK_INHERIT:
			fallthrough
		case LINK_STATIC:
			return PAYLOAD_STATICLIB
		case LINK_DYNAMIC:
			utils.LogFatal("executable should have %s, but found %s", LINK_STATIC, link)
		default:
			utils.UnexpectedValue(link)
		}
		break
	case MODULE_HEADERS:
		return PAYLOAD_HEADERS
	default:
		utils.UnexpectedValue(module.GetModule().ModuleType)
	}
	return result
}
func (env *CompileEnv) GetPayloadOutput(src utils.Filename, payload PayloadType) utils.Filename {
	rel := src.Relative(utils.UFS.Source)
	switch payload {
	case PAYLOAD_EXECUTABLE:
		fallthrough
	case PAYLOAD_SHAREDLIB:
		rel = strings.ReplaceAll(rel, "\\", "/")
		rel = strings.ReplaceAll(rel, "/", "-")
		return utils.UFS.Binaries.AbsoluteFile(rel).ReplaceExt(
			"-" + strings.Join(env.Family(), "-") + env.Extname(payload))
	case PAYLOAD_OBJECTLIST:
		fallthrough
	case PAYLOAD_STATICLIB:
		fallthrough
	case PAYLOAD_PRECOMPILEDHEADER:
		rel = strings.ReplaceAll(rel, "\\", "/")
		rel = strings.ReplaceAll(rel, "/", "-")
		return env.IntermediateDir().AbsoluteFile(rel).ReplaceExt(
			env.Extname(payload))
	case PAYLOAD_HEADERS:
		break
	case PAYLOAD_DEBUG:
	default:
		utils.UnexpectedValue(payload)
	}
	return src
}

func (env *CompileEnv) ModuleAlias(module Module) TargetAlias {
	return TargetAlias{
		NamespaceName: module.GetNamespace().String(),
		ModuleName:    module.GetModule().ModuleName,
		PlatformName:  env.GetPlatform().PlatformName,
		ConfigName:    env.GetConfig().ConfigName,
	}
}
func (env *CompileEnv) Compile(module Module) {
	moduleRules := module.GetModule()
	rootDir := moduleRules.ModuleDir

	unit := &Unit{
		Target:          env.ModuleAlias(module),
		CppRtti:         env.GetCppRtti(module),
		CppStd:          env.GetCppStd(module),
		Debug:           env.GetDebugType(module),
		PCH:             env.GetPCHType(module),
		Link:            env.GetLinkType(module),
		Sanitizer:       env.GetSanitizerType(module),
		Unity:           env.GetUnityType(module),
		Source:          moduleRules.Source,
		ModuleDir:       moduleRules.ModuleDir,
		GeneratedDir:    env.GeneratedDir().Folder(moduleRules.Path()...),
		IntermediateDir: env.IntermediateDir().Folder(moduleRules.Path()...),
		Compiler:        env.Compiler,
	}
	unit.Payload = env.GetPayloadType(module, unit.Link)
	unit.OutputFile = env.GetPayloadOutput(utils.Filename{
		Dirname:  rootDir[:len(rootDir)-1],
		Basename: rootDir[len(rootDir)-1],
	}, unit.Payload)

	switch unit.PCH {
	case PCH_DISABLED:
		break
	case PCH_MONOLITHIC:
		if moduleRules.PrecompiledHeader == nil || moduleRules.PrecompiledSource == nil {
			unit.PCH = PCH_DISABLED
		} else {
			unit.PrecompiledHeader = *moduleRules.PrecompiledHeader
			unit.PrecompiledSource = *moduleRules.PrecompiledSource
			unit.PrecompiledObject = env.GetPayloadOutput(unit.PrecompiledSource, PAYLOAD_OBJECTLIST)
		}
	default:
		utils.UnexpectedValue(unit.PCH)
	}

	unit.Facet = NewFacet()
	unit.Compiler = env.Compiler
	unit.Defines.Append(
		"BUILD_TARGET_NAME="+moduleRules.String(),
		fmt.Sprintf("BUILD_TARGET_ORDINAL=%d", env.ModuleGraph.Get(module).Ordinal))
	unit.Facet.Append(
		moduleRules,
		env)

	unit.Decorate(
		env,
		moduleRules,
		env.GetPlatform(),
		env.GetConfig())

	env.translatedUnits.Add(module, unit)
}
func (env *CompileEnv) Link() (result []*Unit, err error) {
	env.translatedUnits.Range(func(m Module, unit *Unit) {
		moduleNode := env.ModuleGraph.Get(m)

		moduleNode.Range(func(dep Module, vis VisibilityType) {
			if other, ok := env.translatedUnits.Get(dep); ok {
				switch dep.GetModule().ModuleType {
				case MODULE_HEADERS, MODULE_EXTERNAL:
					unit.IncludeDependencies.AppendUniq(other.Target)
				case MODULE_LIBRARY:
					if vis != RUNTIME || other.Link != LINK_DYNAMIC {
						unit.StaticDependencies.AppendUniq(other.Target)
					} else {
						unit.DynamicDependencies.AppendUniq(other.Target)
					}
				default:
					utils.UnexpectedValue(dep.GetModule().ModuleType)
				}
			} else {
				utils.UnreachableCode()
			}
		}, VIS_EVERYTHING)

		unit.IncludeDependencies.Range(func(target TargetAlias) {
			dep := env.ModuleGraph.Module(target.ModuleAlias())
			if other, ok := env.translatedUnits.Get(dep); ok {
				unit.Facet.Append(&other.Transitive)
			} else {
				utils.UnreachableCode()
			}
		})

		unit.StaticDependencies.Range(func(target TargetAlias) {
			dep := env.ModuleGraph.Module(target.ModuleAlias())
			if other, ok := env.translatedUnits.Get(dep); ok {
				unit.IncludePaths.Append(other.Transitive.IncludePaths...)
				unit.ForceIncludes.Append(other.Transitive.ForceIncludes...)
			} else {
				utils.UnreachableCode()
			}
		})

		if unit.Unity == UNITY_AUTOMATIC {
			unit.Unity = moduleNode.Unity(env.SizePerUnity.Get())
		}

		unit.Decorate(env, unit.GetCompiler())
		unit.Facet.PerformSubstitutions()

		result = append(result, unit)

		// for _, gen := range m.GetModule().Generateds {
		// 	gen.Generate(env.Artifacts(), unit) // #TODO: refactor generated files
		// }
	})

	return result, nil
}

type BuildTranslatedUnitsT struct {
	Units []*Unit
}

func (b *BuildTranslatedUnitsT) Alias() utils.BuildAlias {
	return utils.MakeBuildAlias("Build", "TranslatedUnits")
}
func (b *BuildTranslatedUnitsT) Build(ctx utils.BuildContext) (utils.BuildStamp, error) {
	pbarUnits := utils.LogProgress(0, 0, "BuildTranslatedUnits")
	defer pbarUnits.Close()

	allPlatforms := BuildPlatforms.Need(ctx).Values
	allConfigs := BuildConfigs.Need(ctx).Values
	allModules := BuildModules.Need(ctx)

	moduleGraph := GetModuleGraph(allModules)

	count := len(allPlatforms) * len(allConfigs)
	compileEnvs := make(chan *CompileEnv, count)

	go func() {
		defer close(compileEnvs)

		compileFlags := CompileFlags.Get(utils.CommandEnv.Flags)

		pbarCompileEnvs := utils.LogProgress(0, count, "BuildEnvironments")
		defer pbarCompileEnvs.Close()

		for _, platform := range allPlatforms {
			for _, config := range allConfigs {
				compiler := GetBuildCompiler(ctx, platform.GetPlatform().Arch)
				env := NewCompileEnv(platform, config, compiler, moduleGraph, compileFlags)

				utils.LogVerbose("new compile env: %v", env)
				pbarCompileEnvs.Inc()

				compileEnvs <- env
			}
		}
	}()

	var pendingCompile []utils.Future[[]*Unit]
	for x := range compileEnvs {
		compileEnv := x
		pendingCompile = append(pendingCompile, utils.MakeFuture(func() ([]*Unit, error) {
			wg := sync.WaitGroup{}
			n := len(compileEnv.ModuleGraph.Keys())
			wg.Add(n)
			pbarUnits.Add(n)
			for _, module := range compileEnv.ModuleGraph.Keys() {
				go func(compileEnv *CompileEnv, module Module) {
					compileEnv.Compile(module)
					pbarUnits.Inc()
					wg.Done()
				}(compileEnv, module)
			}
			wg.Wait()
			return compileEnv.Link()
		}))
	}

	for _, x := range pendingCompile {
		units := x.Join().Success()
		b.Units = append(b.Units, units...)
		for _, u := range units {
			ctx.DependsOn(u.Compiler)
		}
	}

	o := bytes.Buffer{}
	digest := utils.MapDigest(len(b.Units), func(i int) []byte {
		o.Reset()
		b.Units[i].GetDigestable(&o)
		return o.Bytes()
	})

	return utils.MakeBuildStamp(digest)
}

var BuildTranslatedUnits = utils.MakeBuildable(func(bi utils.BuildInit) *BuildTranslatedUnitsT {
	result := &BuildTranslatedUnitsT{}
	bi.DependsOn(CompileFlags.Get(utils.CommandEnv.Flags))
	return result
})
