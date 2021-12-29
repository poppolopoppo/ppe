package linux

import (
	. "build/compile"
	. "build/utils"
	"bytes"
	"encoding/gob"
)

type LinuxFlagsT struct {
	Compiler  CompilerType
	StackSize IntVar
}

var LinuxFlags = MakeServiceAccessor[ParsableFlags](newLinuxFlags)

func newLinuxFlags() *LinuxFlagsT {
	return &LinuxFlagsT{
		Compiler:  COMPILER_CLANG,
		StackSize: 2000000,
	}
}
func (flags *LinuxFlagsT) InitFlags(cfg *PersistentMap) {
	cfg.Persistent(&flags.Compiler, "Compiler", "select windows compiler ["+Join(",", CompilerTypes()...)+"]")
	cfg.Persistent(&flags.StackSize, "StackSize", "set default thread stack size in bytes")
}
func (flags *LinuxFlagsT) ApplyVars(cfg *PersistentMap) {
}

func (flags *LinuxFlagsT) Alias() BuildAlias {
	return MakeBuildAlias("Flags", "LinuxFlags")
}
func (flags *LinuxFlagsT) Build(BuildContext) (BuildStamp, error) {
	// flags.InitFlags(CommandEnv.Persistent())
	return MakeBuildStamp(flags)
}
func (flags *LinuxFlagsT) GetDigestable(o *bytes.Buffer) {
	flags.Compiler.GetDigestable(o)
	flags.StackSize.GetDigestable(o)
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
	p := &PlatformRules{}
	p.Arch = Platform_X86.Arch
	p.Facet = NewFacet()
	p.Facet.Append(Platform_X86)
	makeLinuxPlatform(p)
	p.PlatformName = "Linux32"
	p.Defines.Append("_LINUX32", "_POSIX32", "__X86__")
	return p
}
func getLinuxPlatform_X64() Platform {
	p := &PlatformRules{}
	p.Arch = Platform_X64.Arch
	p.Facet = NewFacet()
	p.Facet.Append(Platform_X64)
	makeLinuxPlatform(p)
	p.PlatformName = "Linux64"
	p.Defines.Append("_LINUX64", "_POSIX64", "__X64__")
	return p
}

func InitLinux() {
	LogTrace("build/hal/linux.Init()")

	gob.Register(&LinuxFlagsT{})
	gob.Register(&LlvmProductInstall{})
	gob.Register(&LlvmCompiler{})

	AllCompilationFlags.Append(LinuxFlags.Add)

	AllPlatforms.Add("Linux32", getLinuxPlatform_X86())
	AllPlatforms.Add("Linux64", getLinuxPlatform_X64())

	for _, x := range CompilerTypes() {
		var factory CompilerFactory
		switch x {
		case COMPILER_CLANG:
			factory = func(arch ArchType) Compiler {
				return GetLlvmCompiler(arch)
			}
		case COMPILER_GCC:
			factory = func(ArchType) Compiler {
				return nil
			}
		}
		AllCompilers.Add(x.String(), factory)
	}
}
