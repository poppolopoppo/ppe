package windows

import (
	. "build/compile"
	. "build/utils"
	"bytes"
	"fmt"
)

type ResourceCompiler struct {
	CompilerRules
}

func (res *ResourceCompiler) GetCompiler() *CompilerRules { return &res.CompilerRules }

func (res *ResourceCompiler) FriendlyName() string       { return "rc" }
func (res *ResourceCompiler) EnvPath() DirSet            { return NewDirSet(res.WorkingDir()) }
func (res *ResourceCompiler) WorkingDir() Directory      { return res.CompilerRules.Executable.Dirname }
func (res *ResourceCompiler) Extname(PayloadType) string { return ".res" }

func (res *ResourceCompiler) CppRtti(*Facet, bool)                     {}
func (res *ResourceCompiler) CppStd(*Facet, CppStdType)                {}
func (res *ResourceCompiler) DebugSymbols(*Facet, DebugType, Filename) {}
func (res *ResourceCompiler) Define(facet *Facet, def ...string) {
	for _, x := range def {
		facet.AddCompilationFlag(fmt.Sprintf("/d%s", x))
	}
}
func (res *ResourceCompiler) Link(*Facet, LinkType) {}
func (res *ResourceCompiler) PrecompiledHeader(*Facet, PrecompiledHeaderType, Filename, Filename, Filename) {
}
func (res *ResourceCompiler) Sanitizer(*Facet, SanitizerType) {}

func (res *ResourceCompiler) ForceInclude(*Facet, ...Filename) {}
func (res *ResourceCompiler) IncludePath(facet *Facet, dirs ...Directory) {
	for _, x := range dirs {
		facet.AddCompilationFlag(fmt.Sprintf("/i\"%v\"", x))
	}
}
func (res *ResourceCompiler) ExternIncludePath(facet *Facet, dirs ...Directory) {
	res.IncludePath(facet, dirs...)
}
func (res *ResourceCompiler) SystemIncludePath(facet *Facet, dirs ...Directory) {
	res.IncludePath(facet, dirs...)
}
func (res *ResourceCompiler) Library(*Facet, ...Filename)      {}
func (res *ResourceCompiler) LibraryPath(*Facet, ...Directory) {}

func (res *ResourceCompiler) Decorate(_ *CompileEnv, u *Unit) {
	if u.Payload == PAYLOAD_SHAREDLIB {
		// Generate minimal resources for DLLs
		u.CompilerOptions.Append("/q")
	}
}

func (res *ResourceCompiler) Alias() BuildAlias {
	return MakeBuildAlias("HAL", "WindowsResourcCompiler")
}
func (res *ResourceCompiler) Build(bc BuildContext) (BuildStamp, error) {
	windowsFlags := WindowsFlags.Need(CommandEnv.Flags)
	windowsSDKInstall := GetWindowsSDKInstall(windowsFlags.WindowsSDK)
	bc.DependsOn(windowsFlags, windowsSDKInstall)

	res.CompilerRules.CompilerName = "RC_" + windowsSDKInstall.Version
	res.CompilerRules.CompilerFamily = "custom"
	res.CompilerRules.Executable = windowsSDKInstall.ResourceCompiler
	bc.NeedFile(res.CompilerRules.Executable)

	return MakeBuildStamp(res)
}
func (res *ResourceCompiler) GetDigestable(o *bytes.Buffer) {
	res.CompilerRules.GetDigestable(o)
}

var WindowsResourceCompiler = MakeBuildable(func(_ BuildInit) *ResourceCompiler {
	result := &ResourceCompiler{}
	result.CompilerOptions.Append(
		"/nologo",   // no copyright when compiling
		"/fo\"%2\"", // output file injection
		"\"%1\"",    // input file
	)
	return result
})
