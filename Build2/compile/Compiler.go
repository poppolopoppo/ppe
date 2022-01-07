package compile

import (
	utils "build/utils"
	"bytes"
	"errors"
)

type CompilerFactory func(ArchType) Compiler

var AllCompilers utils.SharedMapT[string, CompilerFactory]

type CompilerRules struct {
	CompilerName   string
	CompilerFamily string

	CppStd       CppStdType
	Executable   utils.Filename
	Linker       utils.Filename
	Librarian    utils.Filename
	Preprocessor utils.Filename
	ExtraFiles   utils.FileSet

	Facet
}

type Compiler interface {
	GetCompiler() *CompilerRules

	EnvPath() utils.DirSet
	WorkingDir() utils.Directory

	Extname(PayloadType) string

	Define(*Facet, ...string)
	CppRtti(*Facet, bool)
	CppStd(*Facet, CppStdType)
	DebugSymbols(*Facet, DebugType, utils.Filename)
	Link(*Facet, LinkType)
	PrecompiledHeader(*Facet, PrecompiledHeaderType, utils.Filename, utils.Filename, utils.Filename)
	Sanitizer(*Facet, SanitizerType)

	CompilationFlag(*Facet, ...string)
	ForceInclude(*Facet, ...utils.Filename)
	IncludePath(*Facet, ...utils.Directory)
	ExternIncludePath(*Facet, ...utils.Directory)
	SystemIncludePath(*Facet, ...utils.Directory)
	Library(*Facet, ...utils.Filename)
	LibraryPath(*Facet, ...utils.Directory)

	FacetDecorator
	utils.Buildable
	utils.Digestable
}

func (rules *CompilerRules) Alias() utils.BuildAlias {
	return utils.MakeBuildAlias("Compiler", rules.String())
}
func (rules *CompilerRules) String() string {
	return rules.CompilerName
}

func (rules *CompilerRules) GetFacet() *Facet {
	return rules.Facet.GetFacet()
}
func (rules *CompilerRules) GetDigestable(o *bytes.Buffer) {
	o.WriteString(rules.CompilerName)
	o.WriteString(rules.CompilerFamily)
	rules.CppStd.GetDigestable(o)
	rules.Executable.GetDigestable(o)
	rules.Linker.GetDigestable(o)
	rules.Librarian.GetDigestable(o)
	rules.Preprocessor.GetDigestable(o)
	rules.ExtraFiles.GetDigestable(o)
	rules.Facet.GetDigestable(o)
}
func (rules *CompilerRules) Decorate(env *CompileEnv, unit *Unit) {
	compiler := unit.Compiler
	compiler.Decorate(env, unit)
	compiler.CppStd(&unit.Facet, unit.CppStd)
	compiler.CppRtti(&unit.Facet, unit.CppRtti == CPPRTTI_ENABLED)
	compiler.DebugSymbols(&unit.Facet, unit.Debug, unit.OutputFile)
	compiler.Link(&unit.Facet, unit.Link)
	compiler.Sanitizer(&unit.Facet, unit.Sanitizer)

	compiler.PrecompiledHeader(&unit.Facet,
		unit.PCH,
		unit.PrecompiledHeader,
		unit.PrecompiledSource,
		unit.PrecompiledObject)

	compiler.Define(&unit.Facet, unit.Facet.Defines...)
	compiler.SystemIncludePath(&unit.Facet, unit.Facet.SystemIncludePaths...)
	compiler.ExternIncludePath(&unit.Facet, unit.Facet.ExternIncludePaths...)
	compiler.IncludePath(&unit.Facet, unit.Facet.IncludePaths...)
	compiler.ForceInclude(&unit.Facet, unit.Facet.ForceIncludes...)
	compiler.LibraryPath(&unit.Facet, unit.Facet.LibraryPaths...)
	compiler.Library(&unit.Facet, unit.Facet.Libraries...)
}

var getMemoizedCompiler = utils.MemoizeArg(func(arch ArchType) Compiler {
	if vname, ok := utils.CommandEnv.ReadData("Compiler"); ok {
		if factory, ok := AllCompilers.Get(vname); ok {
			if compiler := factory(arch); compiler != nil {
				return compiler
			}
		}
		panic(errors.New("compiler is not available"))
	} else {
		panic(errors.New("missing persistent variable <Compiler>"))
	}
})

func GetBuildCompiler(bc utils.BuildContext, arch ArchType) Compiler {
	compiler := getMemoizedCompiler(arch)
	return bc.DependsOn(compiler).(Compiler)
}
