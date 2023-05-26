package compile

import (
	//lint:ignore ST1001 ignore dot imports warning
	. "build/utils"
	"fmt"
	"strings"
)

var AllCompilers = NewStringSet()

/***************************************
 * Compiler Alias
 ***************************************/

type CompilerAlias struct {
	CompilerFamily  string
	CompilerName    string
	CompilerVariant string
}

func NewCompilerAlias(family, name, variant string) CompilerAlias {
	return CompilerAlias{
		CompilerFamily:  family,
		CompilerName:    name,
		CompilerVariant: variant,
	}
}
func (x CompilerAlias) Valid() bool {
	return len(x.CompilerName) > 0
}
func (x *CompilerAlias) Alias() BuildAlias {
	return MakeBuildAlias("Rules", "Compiler", x.String())
}
func (x *CompilerAlias) Serialize(ar Archive) {
	ar.String(&x.CompilerFamily)
	ar.String(&x.CompilerName)
	ar.String(&x.CompilerVariant)
}
func (x CompilerAlias) Compare(o CompilerAlias) int {
	if cmp0 := strings.Compare(x.CompilerFamily, o.CompilerFamily); cmp0 == 0 {
		if cmp1 := strings.Compare(x.CompilerName, o.CompilerName); cmp1 == 0 {
			return strings.Compare(x.CompilerVariant, o.CompilerVariant)
		} else {
			return cmp1
		}
	} else {
		return cmp0
	}
}
func (x *CompilerAlias) Set(in string) error {
	if _, err := fmt.Sscanf(in, "%s-%s-%s", &x.CompilerFamily, &x.CompilerName, &x.CompilerVariant); err == nil {
		return nil
	} else {
		return err
	}
}
func (x CompilerAlias) String() string {
	return fmt.Sprintf("%s-%s-%s", x.CompilerFamily, x.CompilerName, x.CompilerVariant)
}
func (x CompilerAlias) MarshalText() ([]byte, error) {
	return UnsafeBytesFromString(x.String()), nil
}
func (x *CompilerAlias) UnmarshalText(data []byte) error {
	return x.Set(UnsafeStringFromBytes(data))
}

/***************************************
 * Compiler interface
 ***************************************/

type Compiler interface {
	GetCompiler() *CompilerRules

	Extname(PayloadType) string

	AllowCaching(*Unit, PayloadType) CacheModeType
	Define(*Facet, ...string)
	CppRtti(*Facet, bool)
	CppStd(*Facet, CppStdType)
	DebugSymbols(u *Unit)
	Link(f *Facet, link LinkType)
	PrecompiledHeader(u *Unit)
	Sanitizer(*Facet, SanitizerType)

	ForceInclude(*Facet, ...Filename)
	IncludePath(*Facet, ...Directory)
	ExternIncludePath(*Facet, ...Directory)
	SystemIncludePath(*Facet, ...Directory)
	Library(*Facet, ...Filename)
	LibraryPath(*Facet, ...Directory)

	SourceDependencies(*ActionRules) Action

	FacetDecorator
	Buildable
	Serializable
}

/***************************************
 * Compiler Rules
 ***************************************/

type CompilerRules struct {
	CompilerAlias CompilerAlias

	CppStd   CppStdType
	Features CompilerFeatureFlags

	Executable   Filename
	Linker       Filename
	Librarian    Filename
	Preprocessor Filename

	Environment ProcessEnvironment
	ExtraFiles  FileSet

	Facet
}

func NewCompilerRules(alias CompilerAlias) CompilerRules {
	return CompilerRules{
		CompilerAlias: alias,
	}
}

func (rules *CompilerRules) Alias() BuildAlias {
	return rules.CompilerAlias.Alias()
}
func (rules *CompilerRules) String() string {
	return rules.CompilerAlias.String()
}

func (rules *CompilerRules) GetFacet() *Facet {
	return rules.Facet.GetFacet()
}
func (rules *CompilerRules) Serialize(ar Archive) {
	ar.Serializable(&rules.CompilerAlias)

	ar.Serializable(&rules.CppStd)
	ar.Serializable(&rules.Features)

	ar.Serializable(&rules.Executable)
	ar.Serializable(&rules.Linker)
	ar.Serializable(&rules.Librarian)
	ar.Serializable(&rules.Preprocessor)

	ar.Serializable(&rules.Environment)
	ar.Serializable(&rules.ExtraFiles)

	ar.Serializable(&rules.Facet)
}
func (rules *CompilerRules) Decorate(env *CompileEnv, unit *Unit) error {
	compiler, err := unit.GetBuildCompiler()
	if err != nil {
		return err
	}

	if err = compiler.Decorate(env, unit); err != nil {
		return err
	}

	compiler.CppStd(&unit.Facet, unit.CppStd)
	compiler.CppRtti(&unit.Facet, unit.CppRtti == CPPRTTI_ENABLED)

	compiler.DebugSymbols(unit)
	compiler.PrecompiledHeader(unit)

	compiler.Link(&unit.Facet, unit.Link)
	compiler.Sanitizer(&unit.Facet, unit.Sanitizer)

	compiler.Define(&unit.Facet, unit.Facet.Defines...)
	compiler.SystemIncludePath(&unit.Facet, unit.Facet.SystemIncludePaths...)
	compiler.ExternIncludePath(&unit.Facet, unit.Facet.ExternIncludePaths...)
	compiler.IncludePath(&unit.Facet, unit.Facet.IncludePaths...)
	compiler.ForceInclude(&unit.Facet, unit.Facet.ForceIncludes...)

	compiler.LibraryPath(&unit.Facet, unit.Facet.LibraryPaths...)
	compiler.Library(&unit.Facet, unit.Facet.Libraries...)

	return nil
}
