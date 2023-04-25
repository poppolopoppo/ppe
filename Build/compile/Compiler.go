package compile

import (
	utils "build/utils"
	"fmt"
	"strings"
)

var AllCompilers = utils.NewStringSet()

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
func (x CompilerAlias) Alias() utils.BuildAlias {
	return utils.MakeBuildAlias("Rules", "Compiler", x.String())
}
func (x *CompilerAlias) Serialize(ar utils.Archive) {
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
	return []byte(x.String()), nil
}
func (x *CompilerAlias) UnmarshalText(data []byte) error {
	return x.Set(string(data))
}

/***************************************
 * Compiler interface
 ***************************************/

type Compiler interface {
	GetCompiler() *CompilerRules

	Extname(PayloadType) string

	Define(*Facet, ...string)
	CppRtti(*Facet, bool)
	CppStd(*Facet, CppStdType)
	DebugSymbols(u *Unit)
	Link(f *Facet, link LinkType)
	PrecompiledHeader(u *Unit)
	Sanitizer(*Facet, SanitizerType)

	ForceInclude(*Facet, ...utils.Filename)
	IncludePath(*Facet, ...utils.Directory)
	ExternIncludePath(*Facet, ...utils.Directory)
	SystemIncludePath(*Facet, ...utils.Directory)
	Library(*Facet, ...utils.Filename)
	LibraryPath(*Facet, ...utils.Directory)

	SourceDependencies(*ActionRules) Action

	FacetDecorator
	utils.Buildable
	utils.Serializable
}

/***************************************
 * Compiler Rules
 ***************************************/

type CompilerRules struct {
	CompilerAlias CompilerAlias

	CppStd   CppStdType
	Features CompilerFeatureFlags

	Executable   utils.Filename
	Linker       utils.Filename
	Librarian    utils.Filename
	Preprocessor utils.Filename

	Environment utils.ProcessEnvironment
	ExtraFiles  utils.FileSet
	WorkingDir  utils.Directory

	Facet
}

func NewCompilerRules(alias CompilerAlias) CompilerRules {
	return CompilerRules{
		CompilerAlias: alias,
	}
}

func (rules *CompilerRules) Alias() utils.BuildAlias {
	return rules.CompilerAlias.Alias()
}
func (rules *CompilerRules) String() string {
	return rules.CompilerAlias.String()
}

func (rules *CompilerRules) GetFacet() *Facet {
	return rules.Facet.GetFacet()
}
func (rules *CompilerRules) Serialize(ar utils.Archive) {
	ar.Serializable(&rules.CompilerAlias)

	ar.Serializable(&rules.CppStd)
	ar.Serializable(&rules.Features)

	ar.Serializable(&rules.Executable)
	ar.Serializable(&rules.Linker)
	ar.Serializable(&rules.Librarian)
	ar.Serializable(&rules.Preprocessor)

	ar.Serializable(&rules.Environment)
	ar.Serializable(&rules.ExtraFiles)
	ar.Serializable(&rules.WorkingDir)

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
