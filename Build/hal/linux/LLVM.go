package linux

import (
	. "build/compile"
	. "build/utils"
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

func (llvm *LlvmCompiler) Serialize(ar Archive) {
	ar.Serializable(&llvm.Arch)
	ar.Serializable(&llvm.Version)
	ar.Serializable(&llvm.CompilerRules)
	SerializeExternal(ar, &llvm.ProductInstall)
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

func (llvm *LlvmCompiler) AllowCaching(u *Unit, payload PayloadType) CacheModeType {
	// #TODO: support deterministic builds with LLVM
	// https://reproducible-builds.org/
	return CACHE_NONE
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
	case CPPSTD_LATEST, CPPSTD_20:
		f.AddCompilationFlag("-std=c++20")
	case CPPSTD_17:
		f.AddCompilationFlag("-std=c++17")
	case CPPSTD_14:
		f.AddCompilationFlag("-std=c++14")
	case CPPSTD_11:
		f.AddCompilationFlag("-std=c++11")
	default:
		UnexpectedValue(std)
	}
}
func (llvm *LlvmCompiler) Define(f *Facet, def ...string) {
	for _, x := range def {
		f.AddCompilationFlag("-D" + x)
	}
}
func (llvm *LlvmCompiler) DebugSymbols(u *Unit) {
	switch u.DebugSymbols {
	case DEBUG_DISABLED:
		return
	case DEBUG_SYMBOLS:
		LogVeryVerbose("not available on linux: DEBUG_SYMBOLS")
	case DEBUG_HOTRELOAD:
		LogVeryVerbose("not available on linux: DEBUG_HOTRELOAD")
	case DEBUG_EMBEDDED:
	default:
		UnexpectedValue(u.DebugSymbols)
	}

	u.CompilerOptions.Append("-g") // embedded debug info
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
func (llvm *LlvmCompiler) PrecompiledHeader(u *Unit) {
	switch u.PCH {
	case PCH_MONOLITHIC, PCH_SHARED:
		u.Defines.Append("BUILD_PCH=1")
		u.CompilerOptions.Append(
			"-include "+u.PrecompiledHeader.Relative(UFS.Source),
			"-include-pch "+MakeLocalFilename(u.PrecompiledObject))
		if u.PCH != PCH_SHARED {
			u.PrecompiledHeaderOptions.Prepend("-emit-pch", "-x c++-header")
		}
	case PCH_DISABLED:
		u.Defines.Append("BUILD_PCH=0")
	default:
		UnexpectedValue(u.PCH)
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
		f.AddCompilationFlag_NoAnalysis("-include" + x.Relative(UFS.Source))
	}
}
func (llvm *LlvmCompiler) IncludePath(f *Facet, dirs ...Directory) {
	for _, x := range dirs {
		f.AddCompilationFlag_NoAnalysis("-I" + MakeLocalDirectory(x))
	}
}
func (llvm *LlvmCompiler) ExternIncludePath(f *Facet, dirs ...Directory) {
	for _, x := range dirs {
		f.AddCompilationFlag_NoAnalysis("-iframework" + MakeLocalDirectory(x))
	}
}
func (llvm *LlvmCompiler) SystemIncludePath(f *Facet, dirs ...Directory) {
	for _, x := range dirs {
		f.AddCompilationFlag_NoAnalysis("-isystem" + MakeLocalDirectory(x))
	}
}
func (llvm *LlvmCompiler) Library(f *Facet, lib ...Filename) {
	for _, x := range lib {
		s := MakeLocalFilename(x)
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
func (llvm *LlvmCompiler) SourceDependencies(obj *ActionRules) Action {
	// #TODO: included headers tracking with llvm
	return obj
}
func (llvm *LlvmCompiler) Decorate(compileEnv *CompileEnv, u *Unit) error {
	if u.CompilerVerbose.Get() {
		u.CompilerOptions.AppendUniq("/v")
	}

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
		decorateLlvmConfig_Debug(u)
	case CONFIG_FASTDEBUG:
		decorateLlvmConfig_FastDebug(u)
	case CONFIG_DEVEL:
		decorateLlvmConfig_Devel(u)
	case CONFIG_TEST:
		decorateLlvmConfig_Test(u)
	case CONFIG_SHIPPING:
		decorateLlvmConfig_Shipping(u)
	default:
		UnexpectedValue(compileEnv.GetConfig().ConfigType)
	}

	return nil
}

/***************************************
 * Compiler options per configuration
 ***************************************/

func llvm_CXX_linkTimeCodeGeneration(u *Unit, enabled bool, incremental bool) {
	if enabled {
		u.LibrarianOptions.Append("-T")
		if incremental {
			LogVeryVerbose("%v: using llvm thin link time optimization with caching", u)
			u.CompilerOptions.Append("-flto=thin")
			u.LinkerOptions.Append("-Wl,--thinlto-cache-dir=" + UFS.Transient.AbsoluteFolder("ThinLTO").String())
		} else {
			LogVeryVerbose("%v: using llvm link time optimization", u)
			u.CompilerOptions.Append("-flto")
		}
	} else {
		LogVeryVerbose("%v: disable llvm link time optimization", u)
		u.CompilerOptions.Append("-fno-lto")
	}
}
func llvm_CXX_runtimeChecks(u *Unit, enabled bool, strong bool) {
	if enabled {
		if strong {
			LogVeryVerbose("%v: using llvm strong stack protector", u)
			u.AddCompilationFlag_NoPreprocessor("-fstack-protector-strong")
		} else {
			LogVeryVerbose("%v: using llvm stack protector", u)
			u.AddCompilationFlag_NoPreprocessor("-fstack-protector")
		}
	} else {
		LogVeryVerbose("%v: disable llvm stack protector", u)
		u.AddCompilationFlag_NoPreprocessor("-fno-stack-protector")
	}
}

func decorateLlvmConfig_Debug(u *Unit) {
	u.AddCompilationFlag("-O0", "-fno-pie")
	llvm_CXX_linkTimeCodeGeneration(u, false, u.Incremental.Get())
	llvm_CXX_runtimeChecks(u, u.RuntimeChecks.Get(), true)
}
func decorateLlvmConfig_FastDebug(u *Unit) {
	u.AddCompilationFlag("-O1", "-fno-pie")
	llvm_CXX_linkTimeCodeGeneration(u, true, u.Incremental.Get())
	llvm_CXX_runtimeChecks(u, u.RuntimeChecks.Get(), false)
}
func decorateLlvmConfig_Devel(u *Unit) {
	u.AddCompilationFlag("-O2", "-fno-pie")
	llvm_CXX_linkTimeCodeGeneration(u, true, u.Incremental.Get())
	llvm_CXX_runtimeChecks(u, false, false)
}
func decorateLlvmConfig_Test(u *Unit) {
	u.AddCompilationFlag("-O3", "-fpie", "-ffast-math")
	llvm_CXX_linkTimeCodeGeneration(u, true, u.Incremental.Get())
	llvm_CXX_runtimeChecks(u, false, false)
}
func decorateLlvmConfig_Shipping(u *Unit) {
	u.AddCompilationFlag("-O3", "-fpie", "-ffast-math")
	llvm_CXX_linkTimeCodeGeneration(u, true, u.Incremental.Get())
	llvm_CXX_runtimeChecks(u, false, false)
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
	return MakeBuildAlias("HAL", "Linux", "LLVM", x.WantedVer.String(), x.Arch)
}
func (x *LlvmProductInstall) Serialize(ar Archive) {
	ar.String(&x.Arch)
	ar.Serializable(&x.WantedVer)

	ar.Serializable(&x.ActualVer)
	ar.Serializable(&x.InstallDir)
	ar.Serializable(&x.Ar)
	ar.Serializable(&x.Clang)
	ar.Serializable(&x.ClangPlusPlus)
}
func (x *LlvmProductInstall) Build(bc BuildContext) error {
	buildCompilerVer := func(suffix string) error {
		LogDebug("llvm: looking for clang-%s...", suffix)
		c := exec.Command("/bin/sh", "-c", "which clang++"+suffix)
		if outp, err := c.Output(); err == nil {
			x.ClangPlusPlus = MakeFilename(strings.TrimSpace(UnsafeStringFromBytes(outp)))
		} else {
			return err
		}

		c = exec.Command("/bin/sh", "-c", "realpath $(which clang"+suffix+")")
		if outp, err := c.Output(); err == nil {
			x.Clang = MakeFilename(strings.TrimSpace(UnsafeStringFromBytes(outp)))
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
			parsed := strings.TrimSpace(UnsafeStringFromBytes(outp))
			if n := strings.IndexByte(parsed, '.'); n != -1 {
				parsed = parsed[:n]
			}
			if err = x.ActualVer.Set(parsed); err != nil {
				return err
			}
		} else {
			return err
		}

		if err := bc.NeedDirectory(x.InstallDir); err != nil {
			return err
		}
		if err := bc.NeedFile(x.Ar, x.Clang, x.ClangPlusPlus); err != nil {
			return err
		}

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

	return err
}

func (llvm *LlvmCompiler) Build(bc BuildContext) error {
	compileFlags := GetCompileFlags()
	linuxFlags := GetLinuxFlags()
	if err := bc.NeedFactories(
		GetBuildableFlags(compileFlags),
		GetBuildableFlags(linuxFlags)); err != nil {
		return err
	}

	var err error
	llvm.ProductInstall, err = GetLlvmProductInstall(LlvmProductVer{
		Arch:    llvm.Arch,
		LlvmVer: linuxFlags.LlvmVer,
	}).Need(bc)
	if err != nil {
		return err
	}

	err = bc.NeedBuildable(llvm.ProductInstall)
	if err != nil {
		return err
	}

	llvm.Version = llvm.ProductInstall.ActualVer
	llvm.CompilerRules.CppStd = CPPSTD_17
	llvm.CompilerRules.Features = MakeEnumSet(
		COMPILER_ALLOW_CACHING,
		COMPILER_ALLOW_DISTRIBUTION,
		COMPILER_ALLOW_SOURCEMAPPING)

	llvm.CompilerRules.Executable = llvm.ProductInstall.ClangPlusPlus
	llvm.CompilerRules.Librarian = llvm.ProductInstall.Ar
	llvm.CompilerRules.Linker = llvm.ProductInstall.Clang

	llvm.CompilerRules.Environment = NewProcessEnvironment()
	llvm.CompilerRules.Facet = NewFacet()
	facet := &llvm.CompilerRules.Facet

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

	if compileFlags.Benchmark.Get() {
		// https: //aras-p.info/blog/2019/01/16/time-trace-timeline-flame-chart-profiler-for-Clang/
		facet.CompilerOptions.Append("-ftime-trace")
	}

	facet.LibrarianOptions.Append("rcs \"%2\" \"%1\"")
	facet.LinkerOptions.Append("\"%1\" -o \"%2\"")

	if int(llvm.Version) >= int(LLVM_14) {
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

	return nil
}

type LlvmProductVer struct {
	Arch    ArchType
	LlvmVer LlvmVersion
}

func GetLlvmProductInstall(productVer LlvmProductVer) BuildFactoryTyped[*LlvmProductInstall] {
	return func(bi BuildInitializer) (*LlvmProductInstall, error) {
		return &LlvmProductInstall{
			Arch:      productVer.Arch.String(),
			WantedVer: productVer.LlvmVer,
		}, nil
	}
}

func GetLlvmCompiler(arch ArchType) BuildFactoryTyped[Compiler] {
	return func(bi BuildInitializer) (Compiler, error) {
		return &LlvmCompiler{
			Arch:          arch,
			CompilerRules: NewCompilerRules(NewCompilerAlias("clang", "llvm", arch.String())),
		}, nil
	}
}
