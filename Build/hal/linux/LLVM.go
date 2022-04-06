package linux

import (
	. "build/compile"
	. "build/utils"
	"bytes"
	"fmt"
	"os/exec"
	"strings"
)

/***************************************
 * LLVM Compiler
 ***************************************/

type LlvmCompiler struct {
	Arch ArchType
	CompilerRules
	ProductInstall *LlvmProductInstall
}

func (llvm *LlvmCompiler) GetCompiler() *CompilerRules { return &llvm.CompilerRules }

func (llvm *LlvmCompiler) GetDigestable(o *bytes.Buffer) {
	llvm.Arch.GetDigestable(o)
	llvm.CompilerRules.GetDigestable(o)
	llvm.ProductInstall.GetDigestable(o)
}

func (llvm *LlvmCompiler) FriendlyName() string {
	return "clang"
}
func (llvm *LlvmCompiler) EnvPath() DirSet {
	return NewDirSet()
}
func (llvm *LlvmCompiler) WorkingDir() Directory {
	return llvm.CompilerRules.Executable.Dirname
}
func (llvm *LlvmCompiler) Extname(x PayloadType) string {
	switch x {
	case PAYLOAD_EXECUTABLE:
		return ".out"
	case PAYLOAD_OBJECTLIST:
		return ".o"
	case PAYLOAD_STATICLIB:
		return ".a"
	case PAYLOAD_SHAREDLIB:
		return ".so"
	case PAYLOAD_PRECOMPILEDHEADER:
		return ".pch"
	case PAYLOAD_HEADERS:
		return ".h"
	default:
		UnexpectedValue(x)
		return ""
	}
}

func (llvm *LlvmCompiler) CppRtti(f *Facet, enabled bool) {
	if enabled {
		f.Defines.Append("PPE_HAS_CXXRTTI=1")
		f.AddCompilationFlag("-frtti")
	} else {
		f.Defines.Append("PPE_HAS_CXXRTTI=0")
		f.AddCompilationFlag("-fno-rtti")
	}
}
func (llvm *LlvmCompiler) CppStd(f *Facet, std CppStdType) {
	switch std {
	case CPPSTD_20:
		f.AddCompilationFlag("-std=c++20")
	case CPPSTD_17:
		f.AddCompilationFlag("-std=c++17")
	case CPPSTD_14:
		f.AddCompilationFlag("-std=c++14")
	case CPPSTD_11:
		f.AddCompilationFlag("-std=c++11")
	}
}
func (llvm *LlvmCompiler) Define(f *Facet, def ...string) {
	for _, x := range def {
		f.AddCompilationFlag("-D" + x)
	}
}
func (llvm *LlvmCompiler) DebugSymbols(f *Facet, sym DebugType, output Filename, intermediate Directory) {
	switch sym {
	case DEBUG_DISABLED:
		return
	case DEBUG_SYMBOLS:
		NotImplemented("DEBUG_SYMBOLS")
	case DEBUG_EMBEDDED:
		f.CompilerOptions.Append("-g")
	case DEBUG_HOTRELOAD:
		NotImplemented("DEBUG_HOTRELOAD")
	default:
		UnexpectedValue(sym)
	}
}
func (llvm *LlvmCompiler) Link(f *Facet, lnk LinkType) {
	switch lnk {
	case LINK_STATIC:
		return // nothing to do
	case LINK_DYNAMIC:
		return // nothing to do
	default:
		UnexpectedValue(lnk)
	}
}
func (llvm *LlvmCompiler) PrecompiledHeader(f *Facet, mode PrecompiledHeaderType, header Filename, source Filename, object Filename) {
	switch mode {
	case PCH_MONOLITHIC, PCH_SHARED:
		NotImplemented("%v", mode)
		fallthrough
	case PCH_DISABLED:
		f.Defines.Append("BUILD_PCH=0")
	default:
		UnexpectedValue(mode)
	}
}
func (llvm *LlvmCompiler) Sanitizer(f *Facet, sanitizer SanitizerType) {
	switch sanitizer {
	case SANITIZER_NONE:
		return
	case SANITIZER_ADDRESS:
		// https://devblogs.microsoft.com/cppblog/addresssanitizer-asan-for-windows-with-llvm/
		f.Defines.Append("USE_PPE_SANITIZER=1")
		f.AddCompilationFlag_NoAnalysis("-fsanitize=address")
	default:
		UnexpectedValue(sanitizer)
	}
}

func (llvm *LlvmCompiler) ForceInclude(f *Facet, inc ...Filename) {
	for _, x := range inc {
		f.AddCompilationFlag_NoAnalysis("-include" + x.String())
	}
}
func (llvm *LlvmCompiler) IncludePath(f *Facet, dirs ...Directory) {
	for _, x := range dirs {
		f.AddCompilationFlag_NoAnalysis("-I" + x.String())
	}
}
func (llvm *LlvmCompiler) ExternIncludePath(f *Facet, dirs ...Directory) {
	for _, x := range dirs {
		f.AddCompilationFlag_NoAnalysis("-iframework" + x.String())
	}
}
func (llvm *LlvmCompiler) SystemIncludePath(f *Facet, dirs ...Directory) {
	for _, x := range dirs {
		f.AddCompilationFlag_NoAnalysis("-isystem" + x.String())
	}
}
func (llvm *LlvmCompiler) Library(f *Facet, lib ...Filename) {
	for _, x := range lib {
		s := x.String()
		f.LibrarianOptions.Append(s)
		f.LinkerOptions.Append(s)
	}
}
func (llvm *LlvmCompiler) LibraryPath(f *Facet, dirs ...Directory) {
	for _, x := range dirs {
		s := x.String()
		llvm.AddCompilationFlag_NoAnalysis("-L" + s)
		f.LinkerOptions.Append("-I" + s)
	}
}

func makeLlvmCompiler(
	result *LlvmCompiler,
	bc BuildContext,
	executable Filename,
	librarian Filename,
	linker Filename,
	extraFiles ...Filename) {
	compileFlags := CompileFlags.FindOrAdd(CommandEnv.Flags)
	linuxFlags := LinuxFlags.FindOrAdd(CommandEnv.Flags)

	bc.DependsOn(compileFlags, linuxFlags)

	result.CompilerRules.CompilerName = fmt.Sprintf("Llvm_%v_%v",
		SanitizeIdentifier(result.ProductInstall.Version), result.ProductInstall.Arch)
	result.CompilerRules.CompilerFamily = "clang"
	result.CompilerRules.CppStd = CPPSTD_17
	result.CompilerRules.Executable = executable
	result.CompilerRules.Librarian = librarian
	result.CompilerRules.Linker = linker
	result.CompilerRules.ExtraFiles = extraFiles

	result.CompilerRules.Facet = NewFacet()
	facet := &result.CompilerRules.Facet

	facet.Defines.Append(
		"CPP_CLANG",
		"LLVM_FOR_POSIX",
	)

	facet.AddCompilationFlag_NoAnalysis(
		"-Wall", "-Wextra", "-Werror", "-Wfatal-errors",
		"-Wshadow",
		"-Wno-unused-command-line-argument", // #TODO: unsilence this warning
		"-fcolor-diagnostics",
		"-march=x86-64-v3 ",
		"-mavx", "-msse4.2",
		"-mlzcnt", "-mpopcnt",
		"-cc1", "-fuse-ctor-homing",
		"-c",               // compile only
		"-o \"%2\" \"%1\"", // input file injection
	)

	facet.LibrarianOptions.Append("rcs \"%2\" \"%1\"")
	facet.LinkerOptions.Append("\"%1\" -o \"%2\"")
}

func (llvm *LlvmCompiler) Decorate(compileEnv *CompileEnv, u *Unit) {
	compileFlags := CompileFlags.FindOrAdd(CommandEnv.Flags)

	switch compileEnv.GetPlatform().Arch {
	case ARCH_X86:
		u.AddCompilationFlag_NoAnalysis("-m32")
	case ARCH_X64:
		u.AddCompilationFlag_NoAnalysis("-m64")
	default:
		UnexpectedValue(compileEnv.GetPlatform().Arch)
	}

	switch compileEnv.GetConfig().ConfigType {
	case CONFIG_DEBUG:
		decorateLlvmConfig_Debug(&u.Facet)
	case CONFIG_FASTDEBUG:
		decorateLlvmConfig_FastDebug(&u.Facet)
	case CONFIG_DEVEL:
		decorateLlvmConfig_Devel(&u.Facet)
	case CONFIG_TEST:
		decorateLlvmConfig_Test(&u.Facet)
	case CONFIG_SHIPPING:
		decorateLlvmConfig_Shipping(&u.Facet)
	default:
		UnexpectedValue(compileEnv.GetConfig().ConfigType)
	}

	switch u.Sanitizer {
	case SANITIZER_ADDRESS:
		u.AddCompilationFlag_NoAnalysis("-fsanitize=address")
	case SANITIZER_THREAD:
		u.AddCompilationFlag_NoAnalysis("-fsanitize=thread")
	case SANITIZER_UNDEFINED_BEHAVIOR:
		u.AddCompilationFlag_NoAnalysis("-fsanitize=ub")
	case SANITIZER_NONE:
	}

	// set dependent linker options
	if compileFlags.Incremental && u.Sanitizer == SANITIZER_NONE {
		NotImplemented("incremental linker")
	} else {

	}
}

/***************************************
 * Compiler options per configuration
 ***************************************/

func llvm_CXX_linkTimeCodeGeneration(f *Facet, enabled bool, incremental bool) {
	if enabled {
		f.LibrarianOptions.Append("-T")
		if incremental {
			f.LinkerOptions.Append("-flto=thin")
		} else {
			f.LinkerOptions.Append("-flto")
		}

	} else {
		f.LinkerOptions.Append("-fno-lto")
	}
}
func llvm_CXX_runtimeChecks(facet *Facet, enabled bool, rtc1 bool) {
	NotImplemented("runtime checks")
}

func decorateLlvmConfig_Debug(f *Facet) {
	f.AddCompilationFlag("-O0", "-fno-pie")
	compileFlags := CompileFlags.Need(CommandEnv.Flags)
	llvm_CXX_linkTimeCodeGeneration(f, false, compileFlags.Incremental.Get())
	llvm_CXX_runtimeChecks(f, compileFlags.RuntimeChecks.Get(), true)
}
func decorateLlvmConfig_FastDebug(f *Facet) {
	f.AddCompilationFlag("-O1", "-fno-pie")
	compileFlags := CompileFlags.Need(CommandEnv.Flags)
	llvm_CXX_linkTimeCodeGeneration(f, true, compileFlags.Incremental.Get())
	llvm_CXX_runtimeChecks(f, compileFlags.RuntimeChecks.Get(), false)
}
func decorateLlvmConfig_Devel(f *Facet) {
	f.AddCompilationFlag("-O2", "-fno-pie")
	compileFlags := CompileFlags.Need(CommandEnv.Flags)
	llvm_CXX_linkTimeCodeGeneration(f, true, compileFlags.Incremental.Get())
	llvm_CXX_runtimeChecks(f, false, false)
}
func decorateLlvmConfig_Test(f *Facet) {
	f.AddCompilationFlag("-O3", "-fpie", "-ffast-math")
	compileFlags := CompileFlags.Need(CommandEnv.Flags)
	llvm_CXX_linkTimeCodeGeneration(f, true, compileFlags.Incremental.Get())
	llvm_CXX_runtimeChecks(f, false, false)
}
func decorateLlvmConfig_Shipping(f *Facet) {
	f.AddCompilationFlag("-O3", "-fpie", "-ffast-math")
	compileFlags := CompileFlags.Need(CommandEnv.Flags)
	llvm_CXX_linkTimeCodeGeneration(f, true, compileFlags.Incremental.Get())
	llvm_CXX_runtimeChecks(f, false, false)
}

/***************************************
 * Compiler detection
 ***************************************/

type LlvmProductInstall struct {
	Arch string

	Version       string
	InstallDir    Directory
	Ar            Filename
	Clang         Filename
	ClangPlusPlus Filename
}

func (x *LlvmProductInstall) Alias() BuildAlias {
	name := fmt.Sprintf("LlvmProductInstall_%s", x.Arch)
	return MakeBuildAlias("HAL", name)
}
func (x *LlvmProductInstall) GetDigestable(o *bytes.Buffer) {
	o.WriteString(x.Arch)
	o.WriteString(x.Version)
	x.InstallDir.GetDigestable(o)
	x.Ar.GetDigestable(o)
	x.Clang.GetDigestable(o)
	x.ClangPlusPlus.GetDigestable(o)
}
func (x *LlvmProductInstall) Build(bc BuildContext) (BuildStamp, error) {
	c := exec.Command("/bin/sh", "-c", "which clang++")
	if outp, err := c.Output(); err == nil {
		x.ClangPlusPlus = MakeFilename(strings.TrimSpace(string(outp)))
	} else {
		return BuildStamp{}, err
	}

	c = exec.Command("/bin/sh", "-c", "realpath $(which clang)")
	if outp, err := c.Output(); err == nil {
		x.Clang = MakeFilename(strings.TrimSpace(string(outp)))
	} else {
		return BuildStamp{}, err
	}

	bin := x.Clang.Dirname
	x.InstallDir = bin.Parent()
	x.Ar = bin.File("llvm-ar")

	if _, err := x.Ar.Info(); err != nil {
		return BuildStamp{}, err
	}

	c = exec.Command("llvm-config", "--version")
	if outp, err := c.Output(); err == nil {
		x.Version = strings.TrimSpace(string(outp))
	} else {
		return BuildStamp{}, err
	}

	bc.NeedFolder(x.InstallDir)
	bc.NeedFile(x.Ar, x.Clang, x.ClangPlusPlus)
	return MakeBuildStamp(x)
}

func (llvm *LlvmCompiler) Alias() BuildAlias {
	return MakeBuildAlias("Compiler", "Llvm_"+llvm.Arch.String())
}
func (llvm *LlvmCompiler) Build(bc BuildContext) (BuildStamp, error) {
	*llvm = LlvmCompiler{Arch: llvm.Arch}

	llvm.ProductInstall = GetLlvmProductInstall(llvm.Arch)
	bc.DependsOn(llvm.ProductInstall)

	makeLlvmCompiler(
		llvm,
		bc,
		llvm.ProductInstall.ClangPlusPlus,
		llvm.ProductInstall.Ar,
		llvm.ProductInstall.Clang)

	return MakeBuildStamp(llvm)
}

var GetLlvmProductInstall = MemoizeArg(func(arch ArchType) *LlvmProductInstall {
	builder := &LlvmProductInstall{
		Arch: arch.String(),
	}
	return CommandEnv.BuildGraph().Create(builder).GetBuildable().(*LlvmProductInstall)
})

var GetLlvmCompiler = MemoizeArg(func(arch ArchType) *LlvmCompiler {
	result := &LlvmCompiler{
		Arch: arch,
	}
	return CommandEnv.BuildGraph().
		Create(result).
		GetBuildable().(*LlvmCompiler)
})
