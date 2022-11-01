package compile

import (
	"build/utils"
	"bytes"
	"fmt"
	"strings"
	"sync"
	"time"
)

type CompileEnv struct {
	Platform
	Configuration
	Compiler
	*CompileFlagsT
	Facet
}

func NewCompileEnv(
	platform Platform,
	config Configuration,
	compiler Compiler,
	compileFlags *CompileFlagsT) (result *CompileEnv) {
	result = &CompileEnv{
		Platform:      platform,
		Configuration: config,
		Compiler:      compiler,
		CompileFlagsT: compileFlags,
		Facet:         NewFacet(),
	}

	result.Facet.Defines.Append(
		"BUILD_ENVIRONMENT="+result.String(),
		"BUILD_PLATFORM="+result.GetPlatform().PlatformName,
		"BUILD_CONFIG="+result.GetConfig().ConfigName,
		"BUILD_COMPILER="+result.GetCompiler().CompilerName,
		"BUILD_FAMILY="+strings.Join(result.Family(), "-"),
		"BUILD_"+strings.Join(result.Family(), "_"))

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
func (env *CompileEnv) GetDigestable(o *bytes.Buffer) {
	env.Platform.GetDigestable(o)
	env.Configuration.GetDigestable(o)
	env.Compiler.GetDigestable(o)
	env.CompileFlagsT.GetDigestable(o)
	env.Facet.GetDigestable(o)
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
	}
	if module != nil {
		if result = module.GetModule().CppStd; CPPSTD_INHERIT != result {
			return result
		}
	}
	return env.GetCompiler().CppStd
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
func (env *CompileEnv) GetExceptionType(module Module) (result ExceptionType) {
	if result = env.CompileFlagsT.Exceptions; EXCEPTION_INHERIT != result {
		return result
	} else if result = module.GetModule().Exceptions; EXCEPTION_INHERIT != result {
		return result
	} else {
		return env.GetConfig().Exceptions
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
		switch module.GetModule().Link {
		case LINK_INHERIT:
			fallthrough
		case LINK_STATIC:
			return PAYLOAD_OBJECTLIST
		case LINK_DYNAMIC:
			return PAYLOAD_SHAREDLIB
		default:
			utils.UnexpectedValuePanic(module.ModuleType, link)
		}
	case MODULE_LIBRARY:
		switch link {
		case LINK_INHERIT:
			fallthrough
		case LINK_STATIC:
			return PAYLOAD_STATICLIB
		case LINK_DYNAMIC:
			return PAYLOAD_SHAREDLIB
		default:
			utils.UnexpectedValuePanic(module.ModuleType, link)
		}
	case MODULE_PROGRAM:
		switch link {
		case LINK_INHERIT:
			fallthrough
		case LINK_STATIC:
			return PAYLOAD_EXECUTABLE
		case LINK_DYNAMIC:
			utils.LogPanic("executable should have %s, but found %s", LINK_STATIC, link)
		default:
			utils.UnexpectedValuePanic(module.ModuleType, link)
		}
	case MODULE_HEADERS:
		return PAYLOAD_HEADERS
	default:
		utils.UnexpectedValuePanic(module.ModuleType, module.ModuleType)
	}
	return result
}
func (env *CompileEnv) GetBinariesOutput(modulePath string, payload PayloadType) utils.Filename {
	utils.AssertIn(payload, PAYLOAD_EXECUTABLE, PAYLOAD_SHAREDLIB)
	modulePath = strings.ReplaceAll(modulePath, "\\", "/")
	modulePath = strings.ReplaceAll(modulePath, "/", "-")
	return utils.UFS.Binaries.AbsoluteFile(modulePath).ReplaceExt(
		"-" + strings.Join(env.Family(), "-") + env.Extname(payload))
}
func (env *CompileEnv) GetIntermediateOutput(modulePath string, payload PayloadType) utils.Filename {
	utils.AssertIn(payload, PAYLOAD_OBJECTLIST, PAYLOAD_PRECOMPILEDHEADER, PAYLOAD_STATICLIB)
	modulePath = strings.ReplaceAll(modulePath, "\\", "/")
	modulePath = strings.ReplaceAll(modulePath, "/", "-")
	return env.IntermediateDir().AbsoluteFile(modulePath).
		ReplaceExt(env.Extname(payload))
}
func (env *CompileEnv) GetPayloadOutput(src utils.Filename, payload PayloadType) utils.Filename {
	rel := src.Relative(utils.UFS.Source)
	switch payload {
	case PAYLOAD_EXECUTABLE:
		fallthrough
	case PAYLOAD_SHAREDLIB:
		return env.GetBinariesOutput(rel, payload)
	case PAYLOAD_OBJECTLIST:
		fallthrough
	case PAYLOAD_PRECOMPILEDHEADER:
		fallthrough
	case PAYLOAD_STATICLIB:
		return env.GetIntermediateOutput(rel, payload)
	case PAYLOAD_HEADERS:
	default:
		utils.UnexpectedValue(payload)
	}
	return src
}

func (env *CompileEnv) EnvironmentAlias() EnvironmentAlias {
	return NewEnvironmentAlias(env.Platform, env.Configuration)
}
func (env *CompileEnv) ModuleAlias(module Module) TargetAlias {
	return NewTargetAlias(module, env.Platform, env.Configuration)
}
func (env *CompileEnv) Compile(module Module) *Unit {
	moduleRules := module.GetModule()
	rootDir := moduleRules.ModuleDir

	unit := &Unit{
		Target:          env.ModuleAlias(module),
		CppRtti:         env.GetCppRtti(module),
		CppStd:          env.GetCppStd(module),
		Debug:           env.GetDebugType(module),
		Exceptions:      env.GetExceptionType(module),
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

	utils.UFS.Mkdir(unit.OutputFile.Dirname)
	utils.UFS.Mkdir(unit.IntermediateDir)

	switch unit.PCH {
	case PCH_DISABLED:
		break
	case PCH_MONOLITHIC:
		if moduleRules.PrecompiledHeader == nil || moduleRules.PrecompiledSource == nil {
			unit.PCH = PCH_DISABLED
		} else {
			unit.PrecompiledHeader = *moduleRules.PrecompiledHeader
			unit.PrecompiledSource = unit.PrecompiledHeader
			utils.IfWindows(func() {
				// CPP is only used on Windows platform
				unit.PrecompiledSource = *moduleRules.PrecompiledSource
			})
			unit.PrecompiledObject = env.GetPayloadOutput(unit.PrecompiledSource, PAYLOAD_PRECOMPILEDHEADER)
		}
	default:
		utils.UnexpectedValuePanic(unit.PCH, unit.PCH)
	}

	unit.Facet = NewFacet()
	unit.Compiler = env.Compiler
	unit.Facet.Append(
		env,
		moduleRules)

	unit.Decorate(
		env,
		moduleRules,
		env.GetPlatform(),
		env.GetConfig())

	return unit
}
func (env *CompileEnv) Link(bc utils.BuildContext, moduleGraph *ModuleGraph, translated map[*ModuleRules]*Unit) (utils.SetT[*Unit], error) {
	pbar := utils.LogProgress(0, len(translated), "%v/Link", env)
	defer pbar.Close()

	linked := utils.NewSet[*Unit]()

	wg := sync.WaitGroup{}
	wg.Add(len(translated))

	for keyModule, valueUnit := range translated {
		go func(module *ModuleRules, unit *Unit) {
			defer pbar.Inc()
			defer wg.Done()
			moduleNode := moduleGraph.Get(module)

			unit.Ordinal = moduleNode.Ordinal
			unit.Defines.Append(
				"BUILD_TARGET_NAME="+unit.Target.GetModuleAlias().String(),
				fmt.Sprintf("BUILD_TARGET_ORDINAL=%d", unit.Ordinal))

			moduleNode.Range(func(dep Module, vis VisibilityType) {
				moduleType := dep.GetModule().ModuleType
				if other, ok := translated[dep.GetModule()]; ok {
					switch other.Payload {
					case PAYLOAD_HEADERS:
						unit.IncludeDependencies.Append(other.Target)
					case PAYLOAD_OBJECTLIST, PAYLOAD_PRECOMPILEDHEADER:
						unit.CompileDependencies.Append(other.Target)
					case PAYLOAD_STATICLIB, PAYLOAD_SHAREDLIB:
						switch vis {
						case PUBLIC, PRIVATE:
							if moduleType != MODULE_EXTERNAL {
								unit.LinkDependencies.AppendUniq(other.Target)
							} else {
								unit.CompileDependencies.AppendUniq(other.Target)
							}
						case RUNTIME:
							if other.Payload == PAYLOAD_SHAREDLIB {
								unit.RuntimeDependencies.AppendUniq(other.Target)
							} else {
								utils.LogPanic("%v <%v> is linking against %v <%v> with %v visibility, which is not allowed:\n%v",
									unit.Payload, unit, other.Payload, other, vis,
									moduleNode.Dependencies)
							}
						default:
							utils.UnexpectedValue(vis)
						}
					case PAYLOAD_EXECUTABLE:
						fallthrough // can't depend on an executable
					default:
						utils.UnexpectedValue(unit.Payload)
					}
				} else {
					utils.UnreachableCode()
				}
			}, VIS_EVERYTHING)

			if unit.Unity == UNITY_AUTOMATIC {
				unit.Unity = moduleNode.Unity(env.SizePerUnity.Get())
			}

			for _, x := range module.Generateds {
				if err := x.GetGenerated().Generate(bc, env, unit); err != nil {
					panic(err)
				}
			}
		}(keyModule, valueUnit)
	}

	wg.Wait()
	pbar.Set(0)

	for _, m := range moduleGraph.keys {
		defer pbar.Inc()
		unit := translated[m.GetModule()]

		unit.IncludeDependencies.Range(func(target TargetAlias) {
			dep := moduleGraph.Module(target.GetModuleAlias())
			if other, ok := translated[dep.GetModule()]; ok {
				utils.LogDebug("[%v] include dep -> %v", unit.Target, target)
				unit.Facet.Append(&other.TransitiveFacet)
			} else {
				utils.UnreachableCode()
			}
		})

		unit.CompileDependencies.Range(func(target TargetAlias) {
			dep := moduleGraph.Module(target.GetModuleAlias())
			if other, ok := translated[dep.GetModule()]; ok {
				utils.LogDebug("[%v] compile dep -> %v", unit.Target, target)
				unit.Facet.Append(&other.TransitiveFacet)
			} else {
				utils.UnreachableCode()
			}
		})

		unit.LinkDependencies.Range(func(target TargetAlias) {
			dep := moduleGraph.Module(target.GetModuleAlias())
			if other, ok := translated[dep.GetModule()]; ok {
				utils.LogDebug("[%v] link dep -> %v", unit.Target, target)
				unit.Facet.Append(&other.TransitiveFacet)
			} else {
				utils.UnreachableCode()
			}
		})

		unit.RuntimeDependencies.Range(func(target TargetAlias) {
			dep := moduleGraph.Module(target.GetModuleAlias())
			if other, ok := translated[dep.GetModule()]; ok {
				utils.LogDebug("[%v] runtime dep -> %v", unit.Target, target)
				unit.IncludePaths.Append(other.TransitiveFacet.IncludePaths...)
				unit.ForceIncludes.Append(other.TransitiveFacet.ForceIncludes...)
			} else {
				utils.UnreachableCode()
			}
		})

		unit.Decorate(env, unit.GetCompiler())
		unit.Facet.PerformSubstitutions()

		linked.Append(unit)
	}

	return linked, nil
}

type BuildEnvironmentsT struct {
	utils.SetT[*CompileEnv]
}

func (b *BuildEnvironmentsT) Alias() utils.BuildAlias {
	return utils.MakeBuildAlias("Build", "Environments")
}
func (b *BuildEnvironmentsT) Build(bc utils.BuildContext) (utils.BuildStamp, error) {
	b.Clear()

	compileFlags := CompileFlags.FindOrAdd(utils.CommandEnv.Flags)
	bc.DependsOn(compileFlags)

	allPlatforms := BuildPlatforms.Need(bc).Values
	allConfigs := BuildConfigs.Need(bc).Values

	count := len(allPlatforms) * len(allConfigs)

	pbar := utils.LogProgress(0, count, b.Alias().String())
	defer pbar.Close()

	digester := utils.MakeDigester()
	for _, platform := range allPlatforms {
		compiler := platform.GetCompiler(bc)

		for _, config := range allConfigs {
			env := NewCompileEnv(platform, config, compiler, compileFlags)

			utils.LogVerbose("build: new compile environment %v", env)
			pbar.Inc()

			b.Append(env)
			digester.Append(env)
		}
	}

	utils.LogTrace("prepared %d compilation environments from %d platforms and %d configs",
		b.Len(), len(allPlatforms), len(allConfigs))
	return utils.MakeTimedBuildStamp(time.Now(), digester.Finalize())
}
func (b *BuildEnvironmentsT) GetEnvironment(alias EnvironmentAlias) *CompileEnv {
	for _, env := range b.SetT {
		if env.EnvironmentAlias() == alias {
			return env
		}
	}
	utils.LogPanic("unknown compile environment <%v>", alias)
	return nil
}

var BuildEnvironments = utils.MakeBuildable(func(bi utils.BuildInit) *BuildEnvironmentsT {
	result := &BuildEnvironmentsT{}
	bi.DependsOn(CompileFlags.FindOrAdd(utils.CommandEnv.Flags))
	return result
})
