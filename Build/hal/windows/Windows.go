package windows

import (
	. "build/compile"
	. "build/utils"
	"bytes"
	"encoding/gob"
	"strconv"
)

type WindowsFlagsT struct {
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

var WindowsFlags = MakeServiceAccessor[ParsableFlags](newWindowsFlags)

func newWindowsFlags() *WindowsFlagsT {
	return CommandEnv.BuildGraph().Create(&WindowsFlagsT{
		Compiler:   COMPILER_MSVC,
		Analyze:    false,
		Insider:    false,
		JustMyCode: false,
		MscVer:     MSC_VER_LATEST,
		PerfSDK:    false,
		Permissive: false,
		StackSize:  2000000,
		StaticCRT:  false,
		WindowsSDK: Directory{},
	}).GetBuildable().(*WindowsFlagsT)
}
func (flags *WindowsFlagsT) InitFlags(cfg *PersistentMap) {
	cfg.Persistent(&flags.Compiler, "Compiler", "select windows compiler ["+JoinString(",", CompilerTypes()...)+"]")
	cfg.Persistent(&flags.Analyze, "Analyze", "enable/disable MSCV analysis")
	cfg.Persistent(&flags.Insider, "Insider", "enable/disable support for pre-release toolchain")
	cfg.Persistent(&flags.JustMyCode, "JustMyCode", "enable/disable MSCV just-my-code")
	cfg.Persistent(&flags.MscVer, "MscVer", "select MSVC toolchain version ["+JoinString(",", MsvcVersions()...)+"]")
	cfg.Persistent(&flags.PerfSDK, "PerfSDK", "enable/disable Visual Studio Performance SDK")
	cfg.Persistent(&flags.Permissive, "Permissive", "enable/disable MSCV permissive")
	cfg.Persistent(&flags.StackSize, "StackSize", "set default thread stack size in bytes")
	cfg.Persistent(&flags.StaticCRT, "StaticCRT", "use static CRT libraries instead of dynamic (/MT vs /MD)")
	cfg.Persistent(&flags.WindowsSDK, "WindowsSDK", "override Windows SDK install path (use latest otherwise)")
}
func (flags *WindowsFlagsT) ApplyVars(cfg *PersistentMap) {
}

func (flags *WindowsFlagsT) Alias() BuildAlias {
	return MakeBuildAlias("Flags", "WindowsFlags")
}
func (flags *WindowsFlagsT) Build(BuildContext) (BuildStamp, error) {
	return MakeBuildStamp(flags)
}
func (flags *WindowsFlagsT) GetDigestable(o *bytes.Buffer) {
	flags.Compiler.GetDigestable(o)
	flags.Analyze.GetDigestable(o)
	flags.Insider.GetDigestable(o)
	flags.JustMyCode.GetDigestable(o)
	flags.MscVer.GetDigestable(o)
	flags.PerfSDK.GetDigestable(o)
	flags.Permissive.GetDigestable(o)
	flags.StackSize.GetDigestable(o)
	flags.StaticCRT.GetDigestable(o)
	flags.WindowsSDK.GetDigestable(o)
}

type WindowsPlatform struct {
	PlatformRules
}

func (win *WindowsPlatform) GetCompiler(bc BuildContext) (result Compiler) {
	flags := WindowsFlags.Need(CommandEnv.Flags)
	bc.DependsOn(flags)

	switch flags.Compiler {
	case COMPILER_MSVC:
		result = GetMsvcCompiler(win.Arch)
	case COMPILER_CLANGCL:
		result = GetClangCompiler(win.Arch)
	default:
		UnexpectedValue(flags.Compiler)
	}

	bc.DependsOn(result)
	return result
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
	p.PlatformName = "Win32"
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
	p.PlatformName = "Win64"
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

func InitWindows() {
	LogTrace("build/hal/window.Init()")

	gob.Register(&WindowsFlagsT{})
	gob.Register(&WindowsPlatform{})
	gob.Register(&MsvcCompiler{})
	gob.Register(&MsvcProductInstall{})
	gob.Register(&ResourceCompiler{})
	gob.Register(&WindowsSDKBuilder{})
	gob.Register(&WindowsSDK{})
	gob.Register(&ClangCompiler{})
	gob.Register(&LlvmProductInstall{})

	AllCompilationFlags.Append(WindowsFlags.Add)

	AllPlatforms.Add("Win32", getWindowsPlatform_X86())
	AllPlatforms.Add("Win64", getWindowsPlatform_X64())

	AllCompilers.Append(
		COMPILER_CLANGCL.String(),
		COMPILER_MSVC.String())
}