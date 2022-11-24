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
func (env *CompileEnv) GetCppRtti(module *ModuleRules) (result CppRttiType) {
	if result = env.CompileFlagsT.CppRtti; CPPRTTI_INHERIT != result {
		return result
	} else if result = module.CppRtti; CPPRTTI_INHERIT != result {
		return result
	} else {
		return env.GetConfig().CppRtti
	}
}
func (env *CompileEnv) GetCppStd(module *ModuleRules) (result CppStdType) {
	if result = env.CompileFlagsT.CppStd; CPPSTD_INHERIT != result {
		return result
	}
	if module != nil {
		if result = module.CppStd; CPPSTD_INHERIT != result {
			return result
		}
	}
	return env.GetCompiler().CppStd
}
func (env *CompileEnv) GetDebugType(module *ModuleRules) (result DebugType) {
	if result = env.CompileFlagsT.DebugSymbols; DEBUG_INHERIT != result {
		return result
	} else if result = module.Debug; DEBUG_INHERIT != result {
		return result
	} else {
		return env.GetConfig().Debug
	}
}
func (env *CompileEnv) GetExceptionType(module *ModuleRules) (result ExceptionType) {
	if result = env.CompileFlagsT.Exceptions; EXCEPTION_INHERIT != result {
		return result
	} else if result = module.Exceptions; EXCEPTION_INHERIT != result {
		return result
	} else {
		return env.GetConfig().Exceptions
	}
}
func (env *CompileEnv) GetPCHType(module *ModuleRules) (result PrecompiledHeaderType) {
	if result = env.PCH; PCH_INHERIT != result {
		return result
	} else if result = module.PCH; PCH_INHERIT != result {
		return result
	} else {
		return env.GetConfig().PCH
	}
}
func (env *CompileEnv) GetLinkType(module *ModuleRules) (result LinkType) {
	if result = env.CompileFlagsT.Link; LINK_INHERIT != result {
		return result
	} else if result = module.Link; LINK_INHERIT != result {
		return result
	} else {
		return env.GetConfig().Link
	}
}
func (env *CompileEnv) GetUnityType(module *ModuleRules) (result UnityType) {
	if result = env.Unity; UNITY_INHERIT != result {
		return result
	} else if result = module.Unity; UNITY_INHERIT != result {
		return result
	} else {
		return env.GetConfig().Unity
	}
}
func (env *CompileEnv) GetSanitizerType(module *ModuleRules) (result SanitizerType) {
	if result = env.CompileFlagsT.Sanitizer; SANITIZER_INHERIT != result {
		return result
	} else if result = module.Sanitizer; SANITIZER_INHERIT != result {
		return result
	} else {
		return env.GetConfig().Sanitizer
	}
}
func (env *CompileEnv) GetPayloadType(module *ModuleRules, link LinkType) (result PayloadType) {
	switch module.ModuleType {
	case MODULE_EXTERNAL:
		switch module.Link {
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
			utils.LogPanic("executable should have %s link, but found %s", LINK_STATIC, link)
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
func (env *CompileEnv) Compile(module *ModuleRules) *Unit {
	rootDir := module.ModuleDir

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
		Source:          module.Source,
		ModuleDir:       module.ModuleDir,
		GeneratedDir:    env.GeneratedDir().Folder(module.Path()...),
		IntermediateDir: env.IntermediateDir().Folder(module.Path()...),
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
	case PCH_MONOLITHIC, PCH_SHARED:
		if module.PrecompiledHeader == nil || module.PrecompiledSource == nil {
			if module.PrecompiledHeader != nil {
				utils.LogPanic("unit is using PCH_%s, but precompiled header is nil (source: %v)", unit.PCH, module.PrecompiledSource)
			}
			if module.PrecompiledSource != nil {
				utils.LogPanic("unit is using PCH_%s, but precompiled source is nil (header: %v)", unit.PCH, module.PrecompiledHeader)
			}
			unit.PCH = PCH_DISABLED
		} else {
			unit.PrecompiledHeader = *module.PrecompiledHeader
			unit.PrecompiledSource = unit.PrecompiledHeader
			utils.IfWindows(func() {
				// CPP is only used on Windows platform
				unit.PrecompiledSource = *module.PrecompiledSource
			})

			utils.Assert(func() bool { return module.PrecompiledHeader.Exists() })
			utils.Assert(func() bool { return module.PrecompiledSource.Exists() })
			unit.PrecompiledObject = env.GetPayloadOutput(unit.PrecompiledSource, PAYLOAD_PRECOMPILEDHEADER)
		}
	default:
		utils.UnexpectedValuePanic(unit.PCH, unit.PCH)
	}

	unit.Facet = NewFacet()
	unit.Compiler = env.Compiler
	unit.Facet.Append(
		env,
		module)

	unit.Decorate(
		env,
		module,
		env.GetPlatform(),
		env.GetConfig())

	return unit
}
func (env *CompileEnv) Link(bc utils.BuildContext, moduleGraph ModuleGraph) (utils.SetT[*Unit], error) {
	pbar := utils.LogProgress(0, len(moduleGraph.SortedKeys()), "%v/Link", env)
	defer pbar.Close()

	linked := utils.NewSet[*Unit]()

	wg := sync.WaitGroup{}
	wg.Add(len(moduleGraph.SortedKeys()))

	moduleGraph.EachNode(func(module Module, node *ModuleNode) {
		go func(node *ModuleNode) {
			defer pbar.Inc()
			defer wg.Done()

			node.Unit.Ordinal = node.Ordinal
			node.Unit.Defines.Append(
				"BUILD_TARGET_NAME="+node.Unit.Target.GetModuleAlias().String(),
				fmt.Sprintf("BUILD_TARGET_ORDINAL=%d", node.Unit.Ordinal))

			node.Range(func(dep Module, vis VisibilityType) {
				other := moduleGraph.NodeByModule(dep)
				moduleType := other.Rules.ModuleType

				switch other.Unit.Payload {
				case PAYLOAD_HEADERS:
					node.Unit.IncludeDependencies.Append(other.Unit.Target)
				case PAYLOAD_OBJECTLIST, PAYLOAD_PRECOMPILEDHEADER:
					node.Unit.CompileDependencies.Append(other.Unit.Target)
				case PAYLOAD_STATICLIB, PAYLOAD_SHAREDLIB:
					switch vis {
					case PUBLIC, PRIVATE:
						if moduleType != MODULE_EXTERNAL {
							node.Unit.LinkDependencies.AppendUniq(other.Unit.Target)
						} else {
							node.Unit.CompileDependencies.AppendUniq(other.Unit.Target)
						}
					case RUNTIME:
						if other.Unit.Payload == PAYLOAD_SHAREDLIB {
							node.Unit.RuntimeDependencies.AppendUniq(other.Unit.Target)
						} else {
							utils.LogPanic("%v <%v> is linking against %v <%v> with %v visibility, which is not allowed:\n%v",
								node.Unit.Payload, node.Unit, node.Unit.Payload, other, vis,
								node.Dependencies)
						}
					default:
						utils.UnexpectedValue(vis)
					}
				case PAYLOAD_EXECUTABLE:
					fallthrough // can't depend on an executable
				default:
					utils.UnexpectedValuePanic(node.Unit.Payload, other.Unit.Payload)
				}

			}, VIS_EVERYTHING)

			if node.Unit.Unity == UNITY_AUTOMATIC {
				node.Unit.Unity = node.Unity(env.SizePerUnity.Get())
			}

			for _, x := range node.Rules.Generateds {
				if err := x.GetGenerated().Generate(bc, env, node.Unit); err != nil {
					utils.LogPanicErr(err)
				}
			}

		}(node)
	})

	wg.Wait()
	pbar.Set(0)

	moduleGraph.EachNode(func(module Module, node *ModuleNode) {
		defer pbar.Inc()

		node.Unit.IncludeDependencies.Range(func(target TargetAlias) {
			other := moduleGraph.NodeByAlias(target.GetModuleAlias())
			utils.LogDebug("[%v] include dep -> %v", node.Unit.Target, target)
			node.Unit.Facet.Append(&other.Unit.TransitiveFacet)
		})

		node.Unit.CompileDependencies.Range(func(target TargetAlias) {
			other := moduleGraph.NodeByAlias(target.GetModuleAlias())
			utils.LogDebug("[%v] compile dep -> %v", node.Unit.Target, target)
			node.Unit.Facet.Append(&other.Unit.TransitiveFacet)
		})

		node.Unit.LinkDependencies.Range(func(target TargetAlias) {
			other := moduleGraph.NodeByAlias(target.GetModuleAlias())
			utils.LogDebug("[%v] link dep -> %v", node.Unit.Target, target)
			node.Unit.Facet.Append(&other.Unit.TransitiveFacet)
		})

		node.Unit.RuntimeDependencies.Range(func(target TargetAlias) {
			other := moduleGraph.NodeByAlias(target.GetModuleAlias())
			utils.LogDebug("[%v] runtime dep -> %v", node.Unit.Target, target)
			node.Unit.IncludePaths.Append(other.Unit.TransitiveFacet.IncludePaths...)
			node.Unit.ForceIncludes.Append(other.Unit.TransitiveFacet.ForceIncludes...)

		})

		node.Unit.Decorate(env, node.Unit.GetCompiler())
		node.Unit.Facet.PerformSubstitutions()

		linked.Append(node.Unit)
	})

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
