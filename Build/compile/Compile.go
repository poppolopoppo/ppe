package compile

import (
	. "build/utils"
	"fmt"
)

var AllCompilationFlags []struct {
	Name, Description string
	CommandParsableFlags
}

func NewCompilationFlags[T any, P interface {
	*T
	CommandParsableFlags
}](name, description string, flags *T) func() P {
	parsable := P(flags)
	AllCompilationFlags = append(AllCompilationFlags, struct {
		Name        string
		Description string
		CommandParsableFlags
	}{
		Name:                 name,
		Description:          description,
		CommandParsableFlags: parsable,
	})
	return NewCommandParsableFlags[T, P](flags)
}

func OptionCommandAllCompilationFlags() CommandOptionFunc {
	return OptionCommandItem(func(ci CommandItem) {
		for _, it := range AllCompilationFlags {
			ci.Options(OptionCommandParsableFlags(it.Name, it.Description, it.CommandParsableFlags))
		}
	})
}

func InitCompile() {
	LogTrace("build/compile.Init()")

	// register type for serialization
	RegisterSerializable(&ActionRules{})
	RegisterSerializable(&BuildConfig{})
	RegisterSerializable(&BuildGenerated{})
	RegisterSerializable(&BuildModules{})
	RegisterSerializable(&BuildTargets{})
	RegisterSerializable(&CompileEnv{})
	RegisterSerializable(&CompilerAlias{})
	RegisterSerializable(&CompilerRules{})
	RegisterSerializable(&ConfigRules{})
	RegisterSerializable(&ConfigurationAlias{})
	RegisterSerializable(&CustomUnit{})
	RegisterSerializable(&EnvironmentAlias{})
	RegisterSerializable(&Facet{})
	RegisterSerializable(&GeneratorRules{})
	RegisterSerializable(&ModuleAlias{})
	RegisterSerializable(&ModuleRules{})
	RegisterSerializable(&NamespaceRules{})
	RegisterSerializable(&PlatformAlias{})
	RegisterSerializable(&PlatformRules{})
	RegisterSerializable(&TargetActions{})
	RegisterSerializable(&TargetAlias{})
	RegisterSerializable(&Unit{})
	RegisterSerializable(&UnityFile{})
	RegisterSerializable(&UnityRules{})

	AllConfigurations.Add("Debug", Configuration_Debug)
	AllConfigurations.Add("FastDebug", Configuration_FastDebug)
	AllConfigurations.Add("Devel", Configuration_Devel)
	AllConfigurations.Add("Test", Configuration_Test)
	AllConfigurations.Add("Shipping", Configuration_Shipping)
}

/***************************************
 * Compile Flags
 ***************************************/

type CompileFlags CppRules

var GetCompileFlags = NewCompilationFlags("compile_flags", "cross-platform compilation flags", &CompileFlags{
	CppRtti:         CPPRTTI_INHERIT,
	CppStd:          CPPSTD_INHERIT,
	DebugSymbols:    DEBUG_INHERIT,
	Exceptions:      EXCEPTION_INHERIT,
	Link:            LINK_INHERIT,
	PCH:             PCH_INHERIT,
	Sanitizer:       SANITIZER_NONE,
	Unity:           UNITY_INHERIT,
	SizePerUnity:    300 * 1024.0, // 300 KiB
	AdaptiveUnity:   INHERITABLE_TRUE,
	Benchmark:       INHERITABLE_FALSE,
	LTO:             INHERITABLE_INHERIT,
	Incremental:     INHERITABLE_INHERIT,
	RuntimeChecks:   INHERITABLE_INHERIT,
	CompilerVerbose: INHERITABLE_FALSE,
	LinkerVerbose:   INHERITABLE_FALSE,
})

func (flags *CompileFlags) Flags(cfv CommandFlagsVisitor) {
	cfv.Persistent("AdaptiveUnity", "enable/disable adaptive unity using source control", &flags.AdaptiveUnity)
	cfv.Persistent("Benchmark", "enable/disable compilation benchmarks", &flags.Benchmark)
	cfv.Persistent("CompilerVerbose", "enable/disable compiler verbose output", &flags.CompilerVerbose)
	cfv.Persistent("CppRtti", "override C++ rtti support ["+JoinString(",", CppRttiTypes()...)+"]", &flags.CppRtti)
	cfv.Persistent("CppStd", "override C++ standard ["+JoinString(",", CppStdTypes()...)+"]", &flags.CppStd)
	cfv.Persistent("DebugSymbols", "override debug symbols mode ["+JoinString(",", DebugTypes()...)+"]", &flags.DebugSymbols)
	cfv.Persistent("Deterministic", "enable/disable deterministic compilation output", &flags.Deterministic)
	cfv.Persistent("Exceptions", "override exceptions mode ["+JoinString(",", ExceptionTypes()...)+"]", &flags.Exceptions)
	cfv.Persistent("Incremental", "enable/disable incremental linker", &flags.Incremental)
	cfv.Persistent("Link", "override link type ["+JoinString(",", LinkTypes()...)+"]", &flags.Link)
	cfv.Persistent("LinkerVerbose", "enable/disable linker verbose output", &flags.LinkerVerbose)
	cfv.Persistent("LTO", "enable/disable link time optimization", &flags.LTO)
	cfv.Persistent("PCH", "override size limit for splitting unity files ["+JoinString(",", PrecompiledHeaderTypes()...)+"]", &flags.PCH)
	cfv.Persistent("RuntimeChecks", "enable/disable runtime security checks", &flags.RuntimeChecks)
	cfv.Persistent("Sanitizer", "override sanitizer mode ["+JoinString(",", SanitizerTypes()...)+"]", &flags.Sanitizer)
	cfv.Persistent("SizePerUnity", "size limit for splitting unity files", &flags.SizePerUnity)
	cfv.Persistent("Unity", "override unity build mode ["+JoinString(",", UnityTypes()...)+"]", &flags.Unity)
}

/***************************************
 * Build Targets
 ***************************************/

type BuildTargets struct {
	EnvironmentAlias
	Targets map[TargetAlias]TargetBuildOrder
	Aliases []TargetAlias
}

func (x *BuildTargets) Alias() BuildAlias {
	return MakeBuildAlias("Targets", x.EnvironmentAlias.String())
}
func (x *BuildTargets) Build(bc BuildContext) error {
	x.Aliases = []TargetAlias{}
	x.Targets = map[TargetAlias]TargetBuildOrder{}

	buildModules, err := GetBuildModules().Need(bc)
	if err != nil {
		return err
	}

	x.Aliases = make([]TargetAlias, 0, len(buildModules.Modules))
	x.Targets = make(map[TargetAlias]TargetBuildOrder, len(buildModules.Modules))

	compileEnv, err := GetCompileEnvironment(x.EnvironmentAlias).Need(bc)
	if err != nil {
		return err
	}

	compiler, err := compileEnv.GetBuildCompiler()
	if err != nil {
		return err
	}

	moduleGraph, err := MakeModuleGraph(compileEnv, buildModules)
	if err != nil {
		return err
	}

	err = ParallelRange(func(module Module) error {
		node := moduleGraph.NodeByModule(module)
		unit, err := compileEnv.Compile(compiler, node.Rules) // node.rules may be different from module.GetModule()
		if err != nil {
			return err
		}

		node.Unit = unit

		for _, it := range node.Rules.Generators {
			generated := it.GetGenerator().CreateGenerated(bc, node.Rules, node.Unit)
			node.Unit.GeneratedFiles.Append(generated.OutputFile)
		}

		return nil
	}, moduleGraph.SortedModules()...)
	if err != nil {
		return err
	}

	units, err := compileEnv.Link(moduleGraph)
	if err != nil {
		return err
	}

	for _, unit := range units {
		x.Targets[unit.Target] = TargetBuildOrder(len(x.Aliases))
		x.Aliases = append(x.Aliases, unit.Target)

		bc.OutputNode(MakeBuildFactory(func(bi BuildInitializer) (*Unit, error) {
			if err := bi.NeedBuildable(unit.Target.ModuleAlias, compileEnv.EnvironmentAlias); err != nil {
				return nil, err
			}
			for _, outputFile := range unit.GeneratedFiles {
				if err := bi.NeedBuildable(MakeGeneratedAlias(outputFile)); err != nil {
					return nil, err
				}
			}
			return unit, nil
		}))
	}

	return nil
}
func (x *BuildTargets) Serialize(ar Archive) {
	ar.Serializable(&x.EnvironmentAlias)
	SerializeMap(ar, &x.Targets)
	SerializeSlice(ar, &x.Aliases)
}
func (x *BuildTargets) GetUnitAlias(target TargetAlias) (TargetAlias, error) {
	if index, ok := x.Targets[target]; ok {
		return x.Aliases[index], nil
	} else {
		return TargetAlias{}, fmt.Errorf("targets: unknown %q target alias", target)
	}
}
func (x *BuildTargets) GetTranslatedUnits() (Units, error) {
	result := make(Units, len(x.Aliases))
	for i, alias := range x.Aliases {
		if unit, err := GetBuildUnit(alias); err == nil {
			result[i] = unit
		} else {
			return Units{}, err
		}
	}
	return result, nil
}

func GetBuildTargets(environmentAlias EnvironmentAlias) BuildFactoryTyped[*BuildTargets] {
	return func(bi BuildInitializer) (*BuildTargets, error) {
		return &BuildTargets{EnvironmentAlias: environmentAlias}, nil
	}
}

func ForeachBuildTargets(each func(BuildFactoryTyped[*BuildTargets]) error) error {
	return ForeachEnvironmentAlias(func(ea EnvironmentAlias) error {
		return each(GetBuildTargets(ea))
	})
}

func BuildTranslatedUnits(bc BuildContext) (result Units, err error) {
	err = ForeachBuildTargets(func(bf BuildFactoryTyped[*BuildTargets]) error {
		if buildTargets, err := bf.Need(bc); err == nil {
			if units, err := buildTargets.GetTranslatedUnits(); err == nil {
				result.Append(units...)
				return bc.DependsOn(MakeBuildAliases(result.Slice()...)...)
			} else {
				return err
			}
		} else {
			return err
		}
	})
	return
}
