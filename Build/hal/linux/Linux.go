package linux

import (
	. "build/compile"
	. "build/utils"
	"bytes"
	"encoding/gob"
)

type LinuxFlagsT struct {
	Compiler  CompilerType
	LlvmVer   LlvmVersion
	StackSize IntVar
}

var LinuxFlags = MakeServiceAccessor[ParsableFlags](newLinuxFlags)

func newLinuxFlags() *LinuxFlagsT {
	return CommandEnv.BuildGraph().Create(&LinuxFlagsT{
		Compiler:  COMPILER_CLANG,
		LlvmVer:   llvm_any,
		StackSize: 2000000,
	}).GetBuildable().(*LinuxFlagsT)
}
func (flags *LinuxFlagsT) InitFlags(cfg *PersistentMap) {
	cfg.Persistent(&flags.Compiler, "Compiler", "select windows compiler ["+JoinString(",", CompilerTypes()...)+"]")
	cfg.Persistent(&flags.LlvmVer, "LlvmVer", "select LLVM toolchain version ["+JoinString(",", LlvmVersions()...)+"]")
	cfg.Persistent(&flags.StackSize, "StackSize", "set default thread stack size in bytes")
}
func (flags *LinuxFlagsT) ApplyVars(cfg *PersistentMap) {
}

func (flags *LinuxFlagsT) Alias() BuildAlias {
	return MakeBuildAlias("Flags", "LinuxFlags")
}
func (flags *LinuxFlagsT) Build(BuildContext) (BuildStamp, error) {
	return MakeBuildStamp(flags)
}
func (flags *LinuxFlagsT) GetDigestable(o *bytes.Buffer) {
	flags.Compiler.GetDigestable(o)
	flags.LlvmVer.GetDigestable(o)
	flags.StackSize.GetDigestable(o)
}

type LinuxPlatform struct {
	PlatformRules
}

func (linux *LinuxPlatform) GetCompiler(bc BuildContext) (result Compiler) {
	flags := LinuxFlags.Need(CommandEnv.Flags)
	bc.DependsOn(flags)

	switch flags.Compiler {
	case COMPILER_CLANG:
		result = GetLlvmCompiler(linux.Arch)
	case COMPILER_GCC:
		NotImplemented("need to implement GCC support")
	default:
		UnexpectedValue(flags.Compiler)
	}

	bc.DependsOn(result)
	return result
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
	p.PlatformName = "Linux32"
	p.Defines.Append("_LINUX32", "_POSIX32", "__X86__")
	return p
}
func getLinuxPlatform_X64() Platform {
	p := &LinuxPlatform{}
	p.Arch = Platform_X64.Arch
	p.Facet = NewFacet()
	p.Facet.Append(Platform_X64)
	makeLinuxPlatform(&p.PlatformRules)
	p.PlatformName = "Linux64"
	p.Defines.Append("_LINUX64", "_POSIX64", "__X64__")
	return p
}

func InitLinux() {
	LogTrace("build/hal/linux.Init()")

	gob.Register(&LinuxFlagsT{})
	gob.Register(&LinuxPlatform{})
	gob.Register(&LlvmProductInstall{})
	gob.Register(&LlvmCompiler{})

	AllCompilationFlags.Append(LinuxFlags.Add)

	AllPlatforms.Add("Linux32", getLinuxPlatform_X86())
	AllPlatforms.Add("Linux64", getLinuxPlatform_X64())

	AllCompilers.Append(
		COMPILER_CLANG.String(),
		COMPILER_GCC.String())
}
