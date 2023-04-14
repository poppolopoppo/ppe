package linux

import (
	. "build/compile"
	. "build/utils"
)

func InitLinux() {
	LogTrace("build/hal/linux.Init()")

	RegisterSerializable(&LinuxPlatform{})
	RegisterSerializable(&LlvmProductInstall{})
	RegisterSerializable(&LlvmCompiler{})

	AllPlatforms.Add("Linux32", getLinuxPlatform_X86())
	AllPlatforms.Add("Linux64", getLinuxPlatform_X64())

	AllCompilers.Append(
		COMPILER_CLANG.String(),
		COMPILER_GCC.String())
}

/***************************************
 * Linux Flags
 ***************************************/

type LinuxFlags struct {
	Compiler          CompilerType
	LlvmVer           LlvmVersion
	DumpRecordLayouts DumpRecordLayoutsType
	StackSize         IntVar
}

var GetLinuxFlags = NewCompilationFlags("linux_flags", "linux-specific compilation flags", &LinuxFlags{
	Compiler:          COMPILER_CLANG,
	LlvmVer:           llvm_any,
	DumpRecordLayouts: DUMPRECORDLAYOUTS_NONE,
	StackSize:         2000000,
})

func (flags *LinuxFlags) Flags(cfv CommandFlagsVisitor) {
	cfv.Persistent("Compiler", "select windows compiler ["+JoinString(",", CompilerTypes()...)+"]", &flags.Compiler)
	cfv.Persistent("LlvmVer", "select LLVM toolchain version ["+JoinString(",", LlvmVersions()...)+"]", &flags.LlvmVer)
	cfv.Persistent("DumpRecordLayouts", "use to investigate structure layouts ["+JoinString(",", DumpRecordLayouts()...)+"]", &flags.DumpRecordLayouts)
	cfv.Persistent("StackSize", "set default thread stack size in bytes", &flags.StackSize)
}

/***************************************
 * Linux Platform
 ***************************************/

type LinuxPlatform struct {
	PlatformRules
	CompilerType CompilerType
}

func (linux *LinuxPlatform) Build(bc BuildContext) (err error) {
	linux.CompilerType = GetLinuxFlags().Compiler
	return linux.PlatformRules.Build(bc)
}
func (linux *LinuxPlatform) Serialize(ar Archive) {
	ar.Serializable(&linux.PlatformRules)
	ar.Serializable(&linux.CompilerType)
}
func (linux *LinuxPlatform) GetCompiler() BuildFactoryTyped[Compiler] {
	switch linux.CompilerType {
	case COMPILER_CLANG:
		return GetLlvmCompiler(linux.Arch)
	case COMPILER_GCC:
		NotImplemented("need to implement GCC support")
		return nil
	default:
		UnexpectedValue(linux.CompilerType)
		return nil
	}
}

func makeLinuxPlatform(p *PlatformRules) {
	p.Os = "Linux"
	p.Defines.Append(
		"PLATFORM_PC",
		"PLATFORM_GLFW",
		"PLATFORM_LINUX",
		"PLATFORM_POSIX",
		"__LINUX__",
	)
}
func getLinuxPlatform_X86() Platform {
	p := &LinuxPlatform{}
	p.Arch = Platform_X86.Arch
	p.Facet = NewFacet()
	p.Facet.Append(Platform_X86)
	makeLinuxPlatform(&p.PlatformRules)
	p.PlatformAlias.PlatformName = "Linux32"
	p.Defines.Append("_LINUX32", "_POSIX32", "__X86__")
	return p
}
func getLinuxPlatform_X64() Platform {
	p := &LinuxPlatform{}
	p.Arch = Platform_X64.Arch
	p.Facet = NewFacet()
	p.Facet.Append(Platform_X64)
	makeLinuxPlatform(&p.PlatformRules)
	p.PlatformAlias.PlatformName = "Linux64"
	p.Defines.Append("_LINUX64", "_POSIX64", "__X64__")
	return p
}
