package windows

import (
	. "build/compile"
	. "build/utils"
	"bytes"
	"fmt"
)

type ResCompiler struct {
	CompilerRules
}

func (res *ResCompiler) GetCompiler() *CompilerRules { return &res.CompilerRules }
func (res *ResCompiler) EnvPath() DirSet             { return NewDirSet(res.WorkingDir()) }
func (res *ResCompiler) WorkingDir() Directory       { return res.CompilerRules.Executable.Dirname }
func (res *ResCompiler) Extname(PayloadType) string  { return ".res" }

func (res *ResCompiler) CppRtti(*Facet, bool)                     {}
func (res *ResCompiler) CppStd(*Facet, CppStdType)                {}
func (res *ResCompiler) DebugSymbols(*Facet, DebugType, Filename) {}
func (res *ResCompiler) Define(facet *Facet, def ...string) {
	for _, x := range def {
		facet.AddCompilationFlag(fmt.Sprintf("/d%s", x))
	}
}
func (res *ResCompiler) Link(*Facet, LinkType) {}
func (res *ResCompiler) PrecompiledHeader(*Facet, PrecompiledHeaderType, Filename, Filename, Filename) {
}
func (res *ResCompiler) Sanitizer(*Facet, SanitizerType) {}

func (res *ResCompiler) CompilationFlag(*Facet, ...string) {}
func (res *ResCompiler) ForceInclude(*Facet, ...Filename)  {}
func (res *ResCompiler) IncludePath(facet *Facet, dirs ...Directory) {
	for _, x := range dirs {
		facet.AddCompilationFlag(fmt.Sprintf("/i\"%v\"", x))
	}
}
func (res *ResCompiler) ExternIncludePath(facet *Facet, dirs ...Directory) {
	res.IncludePath(facet, dirs...)
}
func (res *ResCompiler) SystemIncludePath(facet *Facet, dirs ...Directory) {
	res.IncludePath(facet, dirs...)
}
func (res *ResCompiler) Library(*Facet, ...Filename)      {}
func (res *ResCompiler) LibraryPath(*Facet, ...Directory) {}
func (res *ResCompiler) Decorate(*CompileEnv, *Unit)      {}

func (res *ResCompiler) Alias() BuildAlias {
	return MakeBuildAlias("HAL", "WindowsResourcCompiler")
}
func (res *ResCompiler) Build(bc BuildContext) (BuildStamp, error) {
	windowsSDK := GetWindowsSDK(bc)
	bc.DependsOn(windowsSDK)
	res.CompilerRules.CompilerName = "RC_" + windowsSDK.Version
	res.CompilerRules.CompilerFamily = "custom"
	res.CompilerRules.Executable = windowsSDK.ResourceCompiler
	bc.NeedFile(res.CompilerRules.Executable)
	return MakeBuildStamp(res.CompilerRules)
}
func (res *ResCompiler) GetDigestable(o *bytes.Buffer) {
	res.CompilerRules.GetDigestable(o)
}

var WindowsResourceCompiler = MakeBuildable(func(bi BuildInit) *ResCompiler {
	return &ResCompiler{}
})
