package compile

import (
	. "build/utils"
	"fmt"
	"strings"
)

/***************************************
 * Target Build Order
 ***************************************/

type TargetBuildOrder int32

func (x *TargetBuildOrder) Serialize(ar Archive) {
	ar.Int32((*int32)(x))
}

/***************************************
 * Target Alias
 ***************************************/

type TargetAlias struct {
	EnvironmentAlias
	ModuleAlias
}

type TargetAliases = SetT[TargetAlias]

func NewTargetAlias(module Module, platform Platform, config Configuration) TargetAlias {
	return TargetAlias{
		EnvironmentAlias: NewEnvironmentAlias(platform, config),
		ModuleAlias:      module.GetModule().ModuleAlias,
	}
}
func (x TargetAlias) Valid() bool {
	return x.EnvironmentAlias.Valid() && x.ModuleAlias.Valid()
}
func (x TargetAlias) Alias() BuildAlias {
	return MakeBuildAlias("Unit", x.String())
}
func (x *TargetAlias) Serialize(ar Archive) {
	ar.Serializable(&x.EnvironmentAlias)
	ar.Serializable(&x.ModuleAlias)
}
func (x TargetAlias) Compare(o TargetAlias) int {
	if cmp := x.ModuleAlias.Compare(o.ModuleAlias); cmp == 0 {
		return x.EnvironmentAlias.Compare(o.EnvironmentAlias)
	} else {
		return cmp
	}
}
func (x *TargetAlias) Set(in string) error {
	parts := strings.Split(in, "-")
	if len(parts) < 3 {
		return fmt.Errorf("invalid target alias: %q", in)
	}
	if err := x.ModuleAlias.Set(strings.Join(parts[:len(parts)-2], "-")); err != nil {
		return err
	}
	if err := x.PlatformAlias.Set(parts[len(parts)-2]); err != nil {
		return err
	}
	if err := x.ConfigurationAlias.Set(parts[len(parts)-1]); err != nil {
		return err
	}
	return nil
}
func (x TargetAlias) String() string {
	return fmt.Sprintf("%v-%v-%v", x.ModuleAlias, x.PlatformName, x.ConfigName)
}
func (x TargetAlias) MarshalText() ([]byte, error) {
	return UnsafeBytesFromString(x.String()), nil
}
func (x *TargetAlias) UnmarshalText(data []byte) error {
	return x.Set(UnsafeStringFromBytes(data))
}
func (x *TargetAlias) AutoComplete(in AutoComplete) {
	for _, a := range FindBuildAliases(CommandEnv.BuildGraph(), "Unit") {
		in.Add(strings.TrimPrefix(a.String(), "Unit://"))
	}
}

/***************************************
 * Unit Rules
 ***************************************/

type UnitDecorator interface {
	Decorate(env *CompileEnv, unit *Unit) error
	fmt.Stringer
}

type Units = SetT[*Unit]

type Unit struct {
	Target TargetAlias

	Ordinal     TargetBuildOrder
	Payload     PayloadType
	OutputFile  Filename
	SymbolsFile Filename
	ExportFile  Filename
	ExtraFiles  FileSet

	Source          ModuleSource
	ModuleDir       Directory
	GeneratedDir    Directory
	IntermediateDir Directory

	PrecompiledHeader Filename
	PrecompiledSource Filename
	PrecompiledObject Filename

	IncludeDependencies TargetAliases
	CompileDependencies TargetAliases
	LinkDependencies    TargetAliases
	RuntimeDependencies TargetAliases

	CompilerAlias     CompilerAlias
	PreprocessorAlias CompilerAlias

	Environment ProcessEnvironment

	TransitiveFacet Facet // append in case of public dependency
	GeneratedFiles  FileSet
	CustomUnits     CustomUnitList

	CppRules
	UnityRules
	Facet
}

func (unit *Unit) String() string {
	return unit.Alias().String()
}

func (unit *Unit) GetEnvironment() (*CompileEnv, error) {
	return FindGlobalBuildable[*CompileEnv](unit.Target.EnvironmentAlias)
}
func (unit *Unit) GetBuildCompiler() (Compiler, error) {
	return FindGlobalBuildable[Compiler](unit.CompilerAlias)
}
func (unit *Unit) GetBuildPreprocessor() (Compiler, error) {
	return FindGlobalBuildable[Compiler](unit.PreprocessorAlias)
}

func (unit *Unit) GetCompiler() *CompilerRules {
	if compiler, err := unit.GetBuildCompiler(); err == nil {
		return compiler.GetCompiler()
	} else {
		LogPanicErr(err)
		return nil
	}
}
func (unit *Unit) GetPreprocessor() *CompilerRules {
	if compiler, err := unit.GetBuildPreprocessor(); err == nil {
		return compiler.GetCompiler()
	} else {
		LogPanicErr(err)
		return nil
	}
}

func (unit *Unit) GetFacet() *Facet {
	return &unit.Facet
}
func (unit *Unit) DebugString() string {
	return PrettyPrint(unit)
}
func (unit *Unit) Decorate(env *CompileEnv, decorator ...UnitDecorator) error {
	LogVeryVerbose("unit %v: decorate with [%v]", unit.Target, MakeStringer(func() string {
		return Join(",", decorator...).String()
	}))
	for _, x := range decorator {
		if err := x.Decorate(env, unit); err != nil {
			return err
		}
	}
	return nil
}

func (unit *Unit) GetBinariesOutput(compiler Compiler, src Filename, payload PayloadType) Filename {
	AssertIn(payload, PAYLOAD_EXECUTABLE, PAYLOAD_SHAREDLIB)
	modulePath := src.Relative(UFS.Source)
	modulePath = SanitizePath(modulePath, '-')
	return UFS.Binaries.AbsoluteFile(modulePath).Normalize().ReplaceExt(
		fmt.Sprintf("-%s%s", unit.Target.EnvironmentAlias, compiler.Extname(payload)))
}
func (unit *Unit) GetGeneratedOutput(compiler Compiler, src Filename, payload PayloadType) Filename {
	modulePath := src.Relative(unit.ModuleDir)
	return unit.GeneratedDir.AbsoluteFile(modulePath).Normalize().ReplaceExt(compiler.Extname(payload))
}
func (unit *Unit) GetIntermediateOutput(compiler Compiler, src Filename, payload PayloadType) Filename {
	AssertIn(payload, PAYLOAD_OBJECTLIST, PAYLOAD_PRECOMPILEDHEADER, PAYLOAD_STATICLIB)
	var modulePath string
	if src.Dirname.IsIn(unit.GeneratedDir) {
		modulePath = src.Relative(unit.GeneratedDir)
	} else {
		modulePath = src.Relative(unit.ModuleDir)
	}
	return unit.IntermediateDir.AbsoluteFile(modulePath).Normalize().ReplaceExt(compiler.Extname(payload))
}
func (unit *Unit) GetPayloadOutput(compiler Compiler, src Filename, payload PayloadType) (result Filename) {
	switch payload {
	case PAYLOAD_EXECUTABLE, PAYLOAD_SHAREDLIB:
		result = unit.GetBinariesOutput(compiler, src, payload)
	case PAYLOAD_OBJECTLIST, PAYLOAD_PRECOMPILEDHEADER, PAYLOAD_STATICLIB:
		result = unit.GetIntermediateOutput(compiler, src, payload)
	case PAYLOAD_HEADERS:
		result = src
	default:
		UnexpectedValue(payload)
	}
	return
}

func (unit *Unit) Serialize(ar Archive) {
	ar.Serializable(&unit.Target)

	ar.Serializable(&unit.Ordinal)
	ar.Serializable(&unit.Payload)
	ar.Serializable(&unit.OutputFile)
	ar.Serializable(&unit.SymbolsFile)
	ar.Serializable(&unit.ExportFile)
	ar.Serializable(&unit.ExtraFiles)

	ar.Serializable(&unit.Source)
	ar.Serializable(&unit.ModuleDir)
	ar.Serializable(&unit.GeneratedDir)
	ar.Serializable(&unit.IntermediateDir)

	ar.Serializable(&unit.PrecompiledHeader)
	ar.Serializable(&unit.PrecompiledSource)
	ar.Serializable(&unit.PrecompiledObject)

	SerializeSlice(ar, unit.IncludeDependencies.Ref())
	SerializeSlice(ar, unit.CompileDependencies.Ref())
	SerializeSlice(ar, unit.LinkDependencies.Ref())
	SerializeSlice(ar, unit.RuntimeDependencies.Ref())

	ar.Serializable(&unit.CompilerAlias)
	ar.Serializable(&unit.PreprocessorAlias)

	ar.Serializable(&unit.Environment)

	ar.Serializable(&unit.TransitiveFacet)
	ar.Serializable(&unit.GeneratedFiles)
	ar.Serializable(&unit.CustomUnits)

	ar.Serializable(&unit.CppRules)
	ar.Serializable(&unit.UnityRules)
	ar.Serializable(&unit.Facet)
}

/***************************************
 * Unit Factory
 ***************************************/

func (unit Unit) Alias() BuildAlias {
	return unit.Target.Alias()
}
func (unit *Unit) Build(bc BuildContext) error {
	if err := CreateDirectory(bc, unit.OutputFile.Dirname); err != nil {
		return err
	}
	if err := CreateDirectory(bc, unit.IntermediateDir); err != nil {
		return err
	}

	if unit.Payload.HasOutput() {
		if err := unit.UnityRules.Generate(unit, bc); err != nil {
			return err
		}
	}

	return nil
}

func GetBuildUnit(alias TargetAlias) (*Unit, error) {
	return FindBuildable[*Unit](CommandEnv.BuildGraph(), alias)
}
