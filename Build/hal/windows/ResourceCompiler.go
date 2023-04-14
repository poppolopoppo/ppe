package windows

import (
	. "build/compile"
	"build/utils"
	. "build/utils"
	"fmt"
)

type ResourceCompiler struct {
	CompilerRules
}

func (res *ResourceCompiler) GetCompiler() *CompilerRules { return &res.CompilerRules }

func (res *ResourceCompiler) Extname(PayloadType) string { return ".res" }

func (res *ResourceCompiler) CppRtti(*Facet, bool)      {}
func (res *ResourceCompiler) CppStd(*Facet, CppStdType) {}

func (res *ResourceCompiler) DebugSymbols(*Facet, DebugType, Filename, Directory) {}

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
func (res *ResourceCompiler) SourceDependencies(obj *ActionRules) Action {
	return obj
}

func (res *ResourceCompiler) Decorate(_ *CompileEnv, u *Unit) error {
	if u.Payload == PAYLOAD_SHAREDLIB {
		// Generate minimal resources for DLLs
		u.CompilerOptions.Append("/q")
	}
	return nil
}

func (res *ResourceCompiler) Build(bc BuildContext) error {
	windowsFlags := GetWindowsFlags()
	if _, err := utils.GetBuildableFlags(windowsFlags).Need(bc); err != nil {
		return err
	}

	windowsSDKInstall := GetWindowsSDKInstall(bc, windowsFlags.WindowsSDK)

	res.CompilerRules.Executable = windowsSDKInstall.ResourceCompiler
	if err := bc.NeedFile(res.CompilerRules.Executable); err != nil {
		return err
	}

	res.CompilerRules.WorkingDir = res.CompilerRules.Executable.Dirname
	res.CompilerRules.Environment = ProcessEnvironment{
		"PATH": []string{res.CompilerRules.WorkingDir.String(), "%PATH%"},
	}

	res.CompilerOptions = StringSet{
		"/nologo",   // no copyright when compiling
		"/fo\"%2\"", // output file injection
		"\"%1\"",    // input file
	}

	return nil
}
func (res *ResourceCompiler) Serialize(ar Archive) {
	ar.Serializable(&res.CompilerRules)
}

func GetWindowsResourceCompiler() BuildFactoryTyped[*ResourceCompiler] {
	return func(bi BuildInitializer) (*ResourceCompiler, error) {
		return &ResourceCompiler{
			CompilerRules: NewCompilerRules(NewCompilerAlias("custom", "rc", "windows_sdk")),
		}, nil
	}
}
