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
	Arch    ArchType
	Version LlvmVersion
	CompilerRules
	ProductInstall *LlvmProductInstall
}

func (llvm *LlvmCompiler) GetCompiler() *CompilerRules { return &llvm.CompilerRules }

func (llvm *LlvmCompiler) GetDigestable(o *bytes.Buffer) {
	llvm.Arch.GetDigestable(o)
	llvm.Version.GetDigestable(o)
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
	maxSupported := getCppStdFromLlvm(llvm.Version)
	if int32(std) > int32(maxSupported) {
		std = maxSupported
	}
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
		LogVeryVerbose("not available on linux: DEBUG_SYMBOLS")
	case DEBUG_HOTRELOAD:
		LogVeryVerbose("not available on linux: DEBUG_HOTRELOAD")
	case DEBUG_EMBEDDED:
	default:
		UnexpectedValue(sym)
	}

	f.CompilerOptions.Append("-g") // embedded debug info
}
func (llvm *LlvmCompiler) Link(f *Facet, lnk LinkType) {
	switch lnk {
	case LINK_STATIC:
		return // nothing to do
	case LINK_DYNAMIC:
		f.LinkerOptions.Append("-shared")
	default:
		UnexpectedValue(lnk)
	}
}
func (llvm *LlvmCompiler) PrecompiledHeader(f *Facet, mode PrecompiledHeaderType, header Filename, source Filename, object Filename) {
	switch mode {
	case PCH_MONOLITHIC, PCH_SHARED:
		f.Defines.Append("BUILD_PCH=1")
		f.CompilerOptions.Append(
			"-include "+header.String(),
			"-include-pch "+object.String())
		if mode != PCH_SHARED {
			f.PrecompiledHeaderOptions.Prepend("-emit-pch", "-x c++-header")
		}
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
		f.AddCompilationFlag_NoPreprocessor("-fsanitize=address")
	case SANITIZER_THREAD:
		f.AddCompilationFlag_NoPreprocessor("-fsanitize=thread")
	case SANITIZER_UNDEFINED_BEHAVIOR:
		f.AddCompilationFlag_NoPreprocessor("-fsanitize=ub")
	default:
		UnexpectedValue(sanitizer)
	}
	f.Defines.Append("USE_PPE_SANITIZER=1")
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
	version LlvmVersion,
	executable Filename,
	librarian Filename,
	linker Filename,
	extraFiles ...Filename) {
	result.Version = version

	compileFlags := CompileFlags.FindOrAdd(CommandEnv.Flags)
	linuxFlags := LinuxFlags.FindOrAdd(CommandEnv.Flags)

	bc.DependsOn(compileFlags, linuxFlags)

	result.CompilerRules.CompilerName = fmt.Sprintf("Llvm_%v_%v",
		SanitizeIdentifier(result.Version.String()), result.ProductInstall.Arch)
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
		"-fsized-deallocation", // https://isocpp.org/files/papers/n3778.html
		"-c",                   // compile only
		"-o \"%2\" \"%1\"",     // input file injection
	)

	if compileFlags.Benchmark {
		// https: //aras-p.info/blog/2019/01/16/time-trace-timeline-flame-chart-profiler-for-Clang/
		facet.CompilerOptions.Append("-ftime-trace")
	}

	facet.LibrarianOptions.Append("rcs \"%2\" \"%1\"")
	facet.LinkerOptions.Append("\"%1\" -o \"%2\"")

	if int(version) >= int(LLVM_14) {
		facet.AddCompilationFlag_NoPreprocessor("-Xclang -fuse-ctor-homing")
	}

	switch linuxFlags.DumpRecordLayouts {
	case DUMPRECORDLAYOUTS_NONE:
	case DUMPRECORDLAYOUTS_SIMPLE:
		facet.CompilerOptions.Append("-Xclang -fdump-record-layouts-simple")
	case DUMPRECORDLAYOUTS_FULL:
		facet.CompilerOptions.Append("-Xclang -fdump-record-layouts")
	default:
		UnexpectedValue(linuxFlags.DumpRecordLayouts)
	}
}

func (llvm *LlvmCompiler) Decorate(compileEnv *CompileEnv, u *Unit) {
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
}

/***************************************
 * Compiler options per configuration
 ***************************************/

func llvm_CXX_linkTimeCodeGeneration(f *Facet, enabled bool, incremental bool) {
	if enabled {
		f.LibrarianOptions.Append("-T")
		if incremental {
			f.CompilerOptions.Append("-flto=thin")
			f.LinkerOptions.Append("-Wl,--thinlto-cache-dir=" + UFS.Transient.AbsoluteFolder("ThinLTO").String())
		} else {
			f.CompilerOptions.Append("-flto")
		}

	} else {
		f.CompilerOptions.Append("-fno-lto")
	}
}
func llvm_CXX_runtimeChecks(facet *Facet, enabled bool, strong bool) {
	if enabled {
		if strong {
			facet.AddCompilationFlag_NoPreprocessor("-fstack-protector")
		} else {
			facet.AddCompilationFlag_NoPreprocessor("-fstack-protector-strong")
		}
	} else {
		facet.AddCompilationFlag_NoPreprocessor("-fno-stack-protector")
	}
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
	Arch      string
	WantedVer LlvmVersion

	ActualVer     LlvmVersion
	InstallDir    Directory
	Ar            Filename
	Clang         Filename
	ClangPlusPlus Filename
}

func (x *LlvmProductInstall) Alias() BuildAlias {
	name := fmt.Sprintf("LlvmProductInstall_%s_%s", x.Arch, x.WantedVer)
	return MakeBuildAlias("HAL", name)
}
func (x *LlvmProductInstall) GetDigestable(o *bytes.Buffer) {
	o.WriteString(x.Arch)
	x.WantedVer.GetDigestable(o)
	x.ActualVer.GetDigestable(o)
	x.InstallDir.GetDigestable(o)
	x.Ar.GetDigestable(o)
	x.Clang.GetDigestable(o)
	x.ClangPlusPlus.GetDigestable(o)
}
func (x *LlvmProductInstall) Build(bc BuildContext) (BuildStamp, error) {
	buildCompilerVer := func(suffix string) error {
		LogDebug("llvm: looking for clang-%s...", suffix)
		c := exec.Command("/bin/sh", "-c", "which clang++"+suffix)
		if outp, err := c.Output(); err == nil {
			x.ClangPlusPlus = MakeFilename(strings.TrimSpace(string(outp)))
		} else {
			return err
		}

		c = exec.Command("/bin/sh", "-c", "realpath $(which clang"+suffix+")")
		if outp, err := c.Output(); err == nil {
			x.Clang = MakeFilename(strings.TrimSpace(string(outp)))
		} else {
			return err
		}

		bin := x.Clang.Dirname
		x.InstallDir = bin.Parent()
		x.Ar = bin.File("llvm-ar")

		if _, err := x.Ar.Info(); err != nil {
			return err
		}

		c = exec.Command("llvm-config"+suffix, "--version")
		if outp, err := c.Output(); err == nil {
			parsed := strings.TrimSpace(string(outp))
			if n := strings.IndexByte(parsed, '.'); n != -1 {
				parsed = parsed[:n]
			}
			if err = x.ActualVer.Set(parsed); err != nil {
				return err
			}
		} else {
			return err
		}

		bc.NeedFolder(x.InstallDir)
		bc.NeedFile(x.Ar, x.Clang, x.ClangPlusPlus)
		return nil
	}
	var err error
	switch x.WantedVer {
	case LLVM_LATEST:
		for _, actualVer := range LlvmVersions() {
			if err = buildCompilerVer("-" + actualVer.String()); err == nil {
				break
			}
		}
	case llvm_any:
		err = buildCompilerVer("" /* no suffix */)
	default:
		err = buildCompilerVer("-" + x.WantedVer.String())
	}
	if err != nil {
		return BuildStamp{}, err
	}
	return MakeBuildStamp(x)
}

func (llvm *LlvmCompiler) Alias() BuildAlias {
	return MakeBuildAlias("Compiler", "Llvm_"+llvm.Arch.String())
}
func (llvm *LlvmCompiler) Build(bc BuildContext) (BuildStamp, error) {
	*llvm = LlvmCompiler{Arch: llvm.Arch}

	linuxFlags := LinuxFlags.Need(CommandEnv.Flags)
	llvm.ProductInstall = GetLlvmProductInstall(llvm.Arch, linuxFlags.LlvmVer)
	bc.DependsOn(llvm.ProductInstall)

	makeLlvmCompiler(
		llvm,
		bc,
		llvm.ProductInstall.ActualVer,
		llvm.ProductInstall.ClangPlusPlus,
		llvm.ProductInstall.Ar,
		llvm.ProductInstall.Clang)

	return MakeBuildStamp(llvm)
}

var GetLlvmProductInstall = MemoizeArgs2(func(arch ArchType, ver LlvmVersion) *LlvmProductInstall {
	builder := &LlvmProductInstall{
		Arch:      arch.String(),
		WantedVer: ver,
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
