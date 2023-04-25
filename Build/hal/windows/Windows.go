package windows

import (
	. "build/compile"
	"build/utils"
	. "build/utils"
	"strconv"
)

var HalTag = MakeArchiveTag(MakeFourCC('W', 'I', 'N', 'X'))

func InitWindows() {
	LogTrace("build/hal/window.Init()")

	utils.RegisterSerializable(&WindowsPlatform{})
	utils.RegisterSerializable(&MsvcCompiler{})
	utils.RegisterSerializable(&MsvcProductInstall{})
	utils.RegisterSerializable(&ResourceCompiler{})
	utils.RegisterSerializable(&WindowsSDKBuilder{})
	utils.RegisterSerializable(&WindowsSDK{})
	utils.RegisterSerializable(&ClangCompiler{})
	utils.RegisterSerializable(&LlvmProductInstall{})
	utils.RegisterSerializable(&SourceDependenciesAction{})

	AllPlatforms.Add("Win32", getWindowsPlatform_X86())
	AllPlatforms.Add("Win64", getWindowsPlatform_X64())

	AllCompilers.Append(
		COMPILER_CLANGCL.String(),
		COMPILER_MSVC.String())
}

/***************************************
 * Windows Flags
 ***************************************/

type WindowsFlags struct {
	Compiler   CompilerType
	Analyze    BoolVar
	Insider    BoolVar
	JustMyCode BoolVar
	MscVer     MsvcVersion
	PerfSDK    BoolVar
	Permissive BoolVar
	StackSize  IntVar
	StaticCRT  BoolVar
	WindowsSDK Directory
}

var GetWindowsFlags = NewCompilationFlags("windows_flags", "windows-specific compilation flags", &WindowsFlags{
	Compiler:   COMPILER_MSVC,
	Analyze:    INHERITABLE_FALSE,
	Insider:    INHERITABLE_FALSE,
	JustMyCode: INHERITABLE_FALSE,
	MscVer:     MSC_VER_LATEST,
	PerfSDK:    INHERITABLE_FALSE,
	Permissive: INHERITABLE_FALSE,
	StackSize:  2000000,
	StaticCRT:  INHERITABLE_FALSE,
	WindowsSDK: Directory{},
})

func (flags *WindowsFlags) Flags(cfv CommandFlagsVisitor) {
	cfv.Persistent("Compiler", "select windows compiler ["+JoinString(",", CompilerTypes()...)+"]", &flags.Compiler)
	cfv.Persistent("Analyze", "enable/disable MSCV analysis", &flags.Analyze)
	cfv.Persistent("Insider", "enable/disable support for pre-release toolchain", &flags.Insider)
	cfv.Persistent("JustMyCode", "enable/disable MSCV just-my-code", &flags.JustMyCode)
	cfv.Persistent("MscVer", "select MSVC toolchain version ["+JoinString(",", MsvcVersions()...)+"]", &flags.MscVer)
	cfv.Persistent("PerfSDK", "enable/disable Visual Studio Performance SDK", &flags.PerfSDK)
	cfv.Persistent("Permissive", "enable/disable MSCV permissive", &flags.Permissive)
	cfv.Persistent("StackSize", "set default thread stack size in bytes", &flags.StackSize)
	cfv.Persistent("StaticCRT", "use static CRT libraries instead of dynamic (/MT vs /MD)", &flags.StaticCRT)
	cfv.Persistent("WindowsSDK", "override Windows SDK install path (use latest otherwise)", &flags.WindowsSDK)
}

/***************************************
 * Windows Platform
 ***************************************/

type WindowsPlatform struct {
	PlatformRules
	CompilerType CompilerType
}

func (win *WindowsPlatform) Build(bc BuildContext) error {
	if err := win.PlatformRules.Build(bc); err != nil {
		return err
	}

	flags := GetWindowsFlags()
	if _, err := GetBuildableFlags(flags).Need(bc); err != nil {
		return err
	}

	win.CompilerType = flags.Compiler
	return nil
}
func (win *WindowsPlatform) Serialize(ar Archive) {
	ar.Serializable(&win.PlatformRules)
	ar.Serializable(&win.CompilerType)
}
func (win *WindowsPlatform) GetCompiler() BuildFactoryTyped[Compiler] {
	switch win.CompilerType {
	case COMPILER_MSVC:
		return GetMsvcCompiler(win.Arch)
	case COMPILER_CLANGCL:
		return GetClangCompiler(win.Arch)
	default:
		UnexpectedValue(win.CompilerType)
		return nil
	}
}

func makeWindowsPlatform(p *PlatformRules) {
	p.Os = "Windows"
	p.Defines.Append(
		"PLATFORM_PC",
		"PLATFORM_WINDOWS",
		"WIN32", "__WINDOWS__",
	)
	p.ForceIncludes.Append(UFS.Source.File("winnt_version.h"))
}
func getWindowsPlatform_X86() Platform {
	p := &WindowsPlatform{}
	p.Arch = Platform_X86.Arch
	p.Facet = NewFacet()
	p.Facet.Append(Platform_X86)
	makeWindowsPlatform(&p.PlatformRules)
	p.PlatformAlias.PlatformName = "Win32"
	p.Defines.Append("_WIN32", "__X86__")
	p.Exports.Add("Windows/Platform", "x86")
	return p
}
func getWindowsPlatform_X64() Platform {
	p := &WindowsPlatform{}
	p.Arch = Platform_X64.Arch
	p.Facet = NewFacet()
	p.Facet.Append(Platform_X64)
	makeWindowsPlatform(&p.PlatformRules)
	p.PlatformAlias.PlatformName = "Win64"
	p.Defines.Append("_WIN64", "__X64__")
	p.Exports.Add("Windows/Platform", "x64")
	return p
}

func getWindowsHostPlatform() string {
	switch strconv.IntSize {
	case 32:
		return "x86"
	case 64:
		return "x64"
	default:
		UnexpectedValue(strconv.IntSize)
		return ""
	}
}
