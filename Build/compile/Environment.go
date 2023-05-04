package compile

import (
	"build/utils"
	"fmt"
	"strings"
)

/***************************************
 * Environment Alias
 ***************************************/

type EnvironmentAlias struct {
	PlatformAlias
	ConfigurationAlias
}

func NewEnvironmentAlias(platform Platform, config Configuration) EnvironmentAlias {
	return EnvironmentAlias{
		PlatformAlias:      platform.GetPlatform().PlatformAlias,
		ConfigurationAlias: config.GetConfig().ConfigurationAlias,
	}
}
func (x EnvironmentAlias) Valid() bool {
	return x.PlatformAlias.Valid() && x.ConfigurationAlias.Valid()
}
func (x EnvironmentAlias) Alias() utils.BuildAlias {
	return utils.MakeBuildAlias("Rules", "Environment", x.String())
}
func (x *EnvironmentAlias) Serialize(ar utils.Archive) {
	ar.Serializable(&x.PlatformAlias)
	ar.Serializable(&x.ConfigurationAlias)
}
func (x EnvironmentAlias) Compare(o EnvironmentAlias) int {
	if cmp := x.PlatformAlias.Compare(o.PlatformAlias); cmp == 0 {
		return x.ConfigurationAlias.Compare(o.ConfigurationAlias)
	} else {
		return cmp
	}
}
func (x *EnvironmentAlias) Set(in string) error {
	if _, err := fmt.Sscanf(in, "%s-%s", &x.PlatformName, &x.ConfigName); err == nil {
		if err := x.PlatformAlias.Set(x.PlatformName); err != nil {
			return err
		}
		if err := x.ConfigurationAlias.Set(x.ConfigName); err != nil {
			return err
		}
		return nil
	} else {
		return err
	}
}
func (x EnvironmentAlias) String() string {
	utils.Assert(func() bool { return x.Valid() })
	return fmt.Sprintf("%v-%v", x.PlatformName, x.ConfigName)
}
func (x EnvironmentAlias) MarshalText() ([]byte, error) {
	return utils.UnsafeBytesFromString(x.String()), nil
}
func (x *EnvironmentAlias) UnmarshalText(data []byte) error {
	return x.Set(utils.UnsafeStringFromBytes(data))
}
func (x *EnvironmentAlias) AutoComplete(in utils.AutoComplete) {
	ForeachEnvironmentAlias(func(ea EnvironmentAlias) error {
		in.Add(ea.String())
		return nil
	})
}

/***************************************
 * Compilation Environment
 ***************************************/

type CompileEnv struct {
	EnvironmentAlias EnvironmentAlias
	Facet

	CompilerAlias CompilerAlias
	CompileFlags  CompileFlags
}

func (env *CompileEnv) Family() []string {
	return []string{env.EnvironmentAlias.PlatformName, env.EnvironmentAlias.ConfigName}
}
func (env *CompileEnv) String() string {
	return strings.Join(append([]string{env.CompilerAlias.CompilerName}, env.Family()...), "_")
}
func (env *CompileEnv) Serialize(ar utils.Archive) {
	ar.Serializable(&env.EnvironmentAlias)
	ar.Serializable(&env.Facet)
	ar.Serializable(&env.CompilerAlias)
	utils.SerializeParsableFlags(ar, &env.CompileFlags)
}

func (env *CompileEnv) GetBuildPlatform() (Platform, error) {
	return utils.FindGlobalBuildable[Platform](env.EnvironmentAlias.PlatformAlias)
}
func (env *CompileEnv) GetBuildConfig() (*BuildConfig, error) {
	return utils.FindGlobalBuildable[*BuildConfig](env.EnvironmentAlias.ConfigurationAlias)
}
func (env *CompileEnv) GetBuildCompiler() (Compiler, error) {
	return utils.FindGlobalBuildable[Compiler](env.CompilerAlias)
}

func (env *CompileEnv) GetPlatform() *PlatformRules {
	if platform, err := env.GetBuildPlatform(); err == nil {
		return platform.GetPlatform()
	} else {
		utils.LogPanicErr(err)
		return nil
	}
}
func (env *CompileEnv) GetConfig() *ConfigRules {
	if config, err := env.GetBuildConfig(); err == nil {
		return config.GetConfig()
	} else {
		utils.LogPanicErr(err)
		return nil
	}
}
func (env *CompileEnv) GetCompiler() *CompilerRules {
	if compiler, err := env.GetBuildCompiler(); err == nil {
		return compiler.GetCompiler()
	} else {
		utils.LogPanicErr(err)
		return nil
	}
}

func (env *CompileEnv) GetFacet() *Facet { return &env.Facet }

func (env *CompileEnv) GeneratedDir() utils.Directory {
	return utils.UFS.Generated.Folder(env.Family()...)
}
func (env *CompileEnv) IntermediateDir() utils.Directory {
	return utils.UFS.Intermediate.Folder(env.Family()...)
}
func (env *CompileEnv) GetCpp(module *ModuleRules) CppRules {
	result := CppRules{}
	result.Inherit((*CppRules)(&env.CompileFlags))

	if module != nil {
		result.Inherit(&module.CppRules)
	}
	if config := env.GetConfig(); config != nil {
		result.Inherit(&env.GetConfig().CppRules)
	}
	if compiler := env.GetCompiler(); compiler != nil {
		utils.Inherit(&result.CppStd, compiler.CppStd)
	}
	return result
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

func (env *CompileEnv) ModuleAlias(module Module) TargetAlias {
	return TargetAlias{
		EnvironmentAlias: env.EnvironmentAlias,
		ModuleAlias:      module.GetModule().ModuleAlias,
	}
}
func (env CompileEnv) Compile(compiler Compiler, module Module) (*Unit, error) {
	moduleRules := module.GetModule()
	relativePath := moduleRules.RelativePath()

	unit := &Unit{
		Target: TargetAlias{
			EnvironmentAlias: env.EnvironmentAlias,
			ModuleAlias:      moduleRules.ModuleAlias,
		},
		Source:          moduleRules.Source,
		ModuleDir:       moduleRules.ModuleDir,
		GeneratedDir:    env.GeneratedDir().AbsoluteFolder(relativePath),
		IntermediateDir: env.IntermediateDir().AbsoluteFolder(relativePath),
		CompilerAlias:   env.CompilerAlias,
		CppRules:        env.GetCpp(moduleRules),
		Environment:     compiler.GetCompiler().Environment,
	}
	unit.Payload = env.GetPayloadType(moduleRules, unit.Link)
	unit.OutputFile = unit.GetPayloadOutput(compiler,
		unit.ModuleDir.Parent().File(unit.Target.ModuleAlias.ModuleName),
		unit.Payload)

	switch unit.Payload {
	case PAYLOAD_SHAREDLIB:
		// when linking against a shared lib we must provide the export .lib/.a, not the produced .dll/.so
		unit.ExportFile = unit.OutputFile.ReplaceExt(compiler.Extname(PAYLOAD_STATICLIB))
	default:
		if unit.Payload.HasOutput() {
			unit.ExportFile = unit.OutputFile
		}
	}

	switch unit.PCH {
	case PCH_DISABLED:
		break
	case PCH_MONOLITHIC, PCH_SHARED:
		if moduleRules.PrecompiledHeader == nil || moduleRules.PrecompiledSource == nil {
			if moduleRules.PrecompiledHeader != nil {
				utils.LogPanic("unit is using PCH_%s, but precompiled header is nil (source: %v)", unit.PCH, moduleRules.PrecompiledSource)
			}
			if moduleRules.PrecompiledSource != nil {
				utils.LogPanic("unit is using PCH_%s, but precompiled source is nil (header: %v)", unit.PCH, moduleRules.PrecompiledHeader)
			}
			unit.PCH = PCH_DISABLED
		} else {
			unit.PrecompiledHeader = *moduleRules.PrecompiledHeader
			unit.PrecompiledSource = unit.PrecompiledHeader

			utils.IfWindows(func() {
				// CPP is only used on Windows platform
				unit.PrecompiledSource = *moduleRules.PrecompiledSource
			})

			utils.Assert(func() bool { return moduleRules.PrecompiledHeader.Exists() })
			utils.Assert(func() bool { return moduleRules.PrecompiledSource.Exists() })
			unit.PrecompiledObject = unit.GetPayloadOutput(compiler, unit.PrecompiledSource, PAYLOAD_PRECOMPILEDHEADER)
		}
	default:
		utils.UnexpectedValuePanic(unit.PCH, unit.PCH)
	}

	unit.Facet = NewFacet()
	unit.Facet.Append(&env, moduleRules)

	return unit, unit.Decorate(&env, moduleRules, env.GetPlatform(), env.GetConfig())
}
func (env *CompileEnv) Link(moduleGraph ModuleGraph) (utils.SetT[*Unit], error) {
	pbar := utils.LogProgress(0, len(moduleGraph.SortedModules()), "%v/Link", env)
	defer pbar.Close()

	err := utils.ParallelRange(func(module Module) error {
		defer pbar.Inc()

		node := moduleGraph.NodeByModule(module)

		node.Unit.Ordinal = node.Ordinal
		node.Unit.Defines.Append(
			"BUILD_TARGET_NAME="+node.Unit.Target.ModuleAlias.String(),
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
					if moduleType == MODULE_LIBRARY {
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

		return nil
	}, moduleGraph.SortedModules()...)
	if err != nil {
		return utils.SetT[*Unit]{}, err
	}

	pbar.Reset()

	linked := utils.NewSet[*Unit]()

	err = moduleGraph.EachNode(func(module Module, node *ModuleNode) error {
		defer pbar.Inc()

		node.Unit.IncludeDependencies.Range(func(target TargetAlias) {
			other := moduleGraph.NodeByAlias(target.ModuleAlias)
			utils.LogDebug("[%v] include dep -> %v", node.Unit.Target, target)
			node.Unit.Facet.Append(&other.Unit.TransitiveFacet)
		})

		node.Unit.CompileDependencies.Range(func(target TargetAlias) {
			other := moduleGraph.NodeByAlias(target.ModuleAlias)
			utils.LogDebug("[%v] compile dep -> %v", node.Unit.Target, target)
			node.Unit.Facet.Append(&other.Unit.TransitiveFacet)
		})

		node.Unit.LinkDependencies.Range(func(target TargetAlias) {
			other := moduleGraph.NodeByAlias(target.ModuleAlias)
			utils.LogDebug("[%v] link dep -> %v", node.Unit.Target, target)
			node.Unit.Facet.Append(&other.Unit.TransitiveFacet)
		})

		node.Unit.RuntimeDependencies.Range(func(target TargetAlias) {
			other := moduleGraph.NodeByAlias(target.ModuleAlias)
			utils.LogDebug("[%v] runtime dep -> %v", node.Unit.Target, target)
			node.Unit.IncludePaths.Append(other.Unit.TransitiveFacet.IncludePaths...)
			node.Unit.ForceIncludes.Append(other.Unit.TransitiveFacet.ForceIncludes...)
		})

		if err := node.Unit.Decorate(env, node.Unit.GetCompiler()); err != nil {
			return err
		}
		node.Unit.Facet.PerformSubstitutions()

		linked.Append(node.Unit)
		return nil
	})

	return linked, err
}

/***************************************
 * Compilation Environment Factory
 ***************************************/

func (env *CompileEnv) Alias() utils.BuildAlias {
	return env.EnvironmentAlias.Alias()
}
func (env *CompileEnv) Build(bc utils.BuildContext) error {
	if compile, err := utils.GetBuildableFlags(GetCompileFlags()).Need(bc); err == nil {
		env.CompileFlags = compile.Flags
	} else {
		return err
	}

	env.CompilerAlias = CompilerAlias{}

	if platform, err := env.GetBuildPlatform(); err == nil {
		if compiler, err := platform.GetCompiler().Need(bc); err == nil {
			env.CompilerAlias = compiler.GetCompiler().CompilerAlias
		} else {
			return err
		}
	} else {
		return err
	}

	env.Facet = NewFacet()
	env.Facet.Defines.Append(
		"BUILD_ENVIRONMENT="+env.String(),
		"BUILD_PLATFORM="+env.EnvironmentAlias.PlatformName,
		"BUILD_CONFIG="+env.EnvironmentAlias.ConfigName,
		"BUILD_COMPILER="+env.CompilerAlias.String(),
		"BUILD_FAMILY="+strings.Join(env.Family(), "-"),
		"BUILD_"+strings.Join(env.Family(), "_"))

	env.Facet.IncludePaths.Append(utils.UFS.Source)

	env.Facet.Append(
		env.GetPlatform(),
		env.GetConfig(),
		env.GetCompiler())

	return nil
}

func GetCompileEnvironment(env EnvironmentAlias) utils.BuildFactoryTyped[*CompileEnv] {
	return func(bi utils.BuildInitializer) (*CompileEnv, error) {
		// register dependency to Configuration/Platform
		// Compiler is a dynamic dependency, since it depends on CompileFlags

		config, err := GetBuildConfig(env.ConfigurationAlias).Need(bi)
		if err != nil {
			return nil, err
		}

		platform, err := GetBuildPlatform(env.PlatformAlias).Need(bi)
		if err != nil {
			return nil, err
		}

		return &CompileEnv{
			EnvironmentAlias: NewEnvironmentAlias(platform, config),
			Facet:            NewFacet(),
		}, nil
	}
}

func ForeachEnvironmentAlias(each func(EnvironmentAlias) error) error {
	for _, platformName := range AllPlatforms.Keys() {
		for _, configName := range AllConfigurations.Keys() {
			ea := EnvironmentAlias{
				PlatformAlias:      NewPlatformAlias(platformName),
				ConfigurationAlias: NewConfigurationAlias(configName),
			}
			if err := each(ea); err != nil {
				return err
			}
		}
	}
	return nil
}

func GetEnvironmentAliases() (result []EnvironmentAlias) {
	result = make([]EnvironmentAlias, 0, AllPlatforms.Len()*AllConfigurations.Len())
	for _, platformName := range AllPlatforms.Keys() {
		for _, configName := range AllConfigurations.Keys() {
			result = append(result, EnvironmentAlias{
				PlatformAlias:      NewPlatformAlias(platformName),
				ConfigurationAlias: NewConfigurationAlias(configName),
			})
		}
	}
	return result
}

func ForeachCompileEnvironment(each func(utils.BuildFactoryTyped[*CompileEnv]) error) error {
	return ForeachEnvironmentAlias(func(ea EnvironmentAlias) error {
		return each(GetCompileEnvironment(ea))
	})
}
