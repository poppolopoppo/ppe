package windows

import (
	. "build/compile"
	. "build/utils"
	"fmt"
	"os/exec"
	"regexp"
)

/***************************************
 * LLVM for Windows
 ***************************************/

type LlvmProductInstall struct {
	Version     string
	InstallDir  Directory
	ClangCl_exe Filename
	LlvmLib_exe Filename
	LldLink_exe Filename
}

type ClangCompiler struct {
	ProductInstall *LlvmProductInstall
	MsvcCompiler
}

func (clang *ClangCompiler) ExternIncludePath(f *Facet, dirs ...Directory) {
	for _, x := range dirs {
		f.AddCompilationFlag_NoAnalysis("/imsvc\"" + x.String() + "\"")
	}
}
func (clang *ClangCompiler) SystemIncludePath(facet *Facet, dirs ...Directory) {
	clang.ExternIncludePath(facet, dirs...)
}
func (clang *ClangCompiler) DebugSymbols(u *Unit) {
	clang.MsvcCompiler.DebugSymbols(u)

	// https://blog.llvm.org/2018/01/improving-link-time-on-windows-with.html
	if u.LinkerOptions.Contains("/DEBUG") {
		//f.CompilerOptions.Append("-mllvm", "-emit-codeview-ghash-section")
		u.LinkerOptions.Remove("/DEBUG")
		u.LinkerOptions.Append("/DEBUG:GHASH")
	}

	// not supported by clang-cl
	u.RemoveCompilationFlag("/Zf")
}
func (clang *ClangCompiler) Decorate(compileEnv *CompileEnv, u *Unit) error {
	err := clang.MsvcCompiler.Decorate(compileEnv, u)
	if err != nil {
		return err
	}

	// flags added by msvc but not supported by clang-cl, llvm-lib or lld-link
	u.RemoveCompilationFlag("/WX", "/JMC-")
	u.LibrarianOptions.Remove("/WX", "/SUBSYSTEM:WINDOWS", "/NODEFAULTLIB")
	u.LinkerOptions.Remove("/WX", "/LTCG", "/LTCG:INCREMENTAL", "/LTCG:OFF", "/NODEFAULTLIB", "/d2:-cgsummary")

	switch compileEnv.GetPlatform().Arch {
	case ARCH_ARM, ARCH_X86:
		u.AddCompilationFlag_NoAnalysis("-m32")
	case ARCH_ARM64, ARCH_X64:
		u.AddCompilationFlag_NoAnalysis("-m64")
	default:
		UnexpectedValue(compileEnv.GetPlatform().Arch)
	}

	if u.CppRules.CompilerVerbose.Get() {
		// enable compiler verbose output
		u.CompilerOptions.AppendUniq("-v")
	}

	// #TODO: wait for MSTL/llvm to be fixed with this optimization
	// if u.Payload == PAYLOAD_SHAREDLIB {
	// 	// https://blog.llvm.org/2018/11/30-faster-windows-builds-with-clang-cl_14.html
	// 	u.CompilerOptions.Append("/Zc:dllexportInlines-") // not workig with /MD and std
	// 	u.PrecompiledHeaderOptions.Append("/Zc:dllexportInlines-")
	// }

	return nil
}
func (clang *ClangCompiler) Serialize(ar Archive) {
	SerializeExternal(ar, &clang.ProductInstall)
	ar.Serializable(&clang.MsvcCompiler)
}

func (clang *ClangCompiler) Build(bc BuildContext) error {
	if llvm, err := GetLlvmProductInstall().Need(bc); err == nil {
		clang.ProductInstall = llvm
	} else {
		return err
	}

	if msvc, err := GetMsvcCompiler(clang.Arch).Need(bc); err == nil {
		compilerAlias := clang.CompilerAlias
		clang.MsvcCompiler = *(msvc.(*MsvcCompiler))
		clang.CompilerAlias = compilerAlias
	} else {
		return err
	}

	compileFlags := GetCompileFlags()
	windowsFlags := GetWindowsFlags()

	rules := clang.GetCompiler()
	rules.Executable = clang.ProductInstall.ClangCl_exe
	rules.Librarian = clang.ProductInstall.LlvmLib_exe
	rules.Linker = clang.ProductInstall.LldLink_exe
	rules.ExtraFiles = NewFileSet(
		clang.ProductInstall.InstallDir.Folder("bin").File("msvcp140.dll"),
		clang.ProductInstall.InstallDir.Folder("bin").File("vcruntime140.dll"),
	)

	rules.Defines.Append("CPP_CLANG", "LLVM_FOR_WINDOWS", "_CRT_SECURE_NO_WARNINGS")
	rules.AddCompilationFlag_NoAnalysis(
		// compiler output
		"-msse4.2",
		// msvc compatibility
		"-fmsc-version="+clang.MsvcCompiler.MSC_VER.String(),
		"-fms-compatibility",
		"-fms-extensions",
		// error reporting
		"-fcolor-diagnostics",
		"/clang:-fno-elide-type",
		"/clang:-fdiagnostics-show-template-tree",
		"/clang:-fmacro-backtrace-limit=0",
		"/clang:-ftemplate-backtrace-limit=0",
	)

	if compileFlags.Benchmark.Get() {
		// https: //aras-p.info/blog/2019/01/16/time-trace-timeline-flame-chart-profiler-for-Clang/
		rules.CompilerOptions.Append("/clang:-ftime-trace")
	}

	if windowsFlags.Permissive.Get() {
		rules.AddCompilationFlag_NoAnalysis("-Wno-error")
	} else {
		rules.AddCompilationFlag_NoAnalysis(
			"-Werror",
			"-Wno-assume",                        // the argument to '__assume' has side effects that will be discarded
			"-Wno-ignored-pragma-optimize",       // pragma optimize n'est pas supporté
			"-Wno-unused-command-line-argument",  // ignore les options non suportées par CLANG (sinon échoue a cause de /WError)
			"-Wno-ignored-attributes",            // ignore les attributs de classe/fonction non supportées par CLANG (sinon échoue a cause de /WError)
			"-Wno-unknown-pragmas",               // ignore les directives pragma non supportées par CLANG (sinon échoue a cause de /WError)
			"-Wno-unused-local-typedef",          // ignore les typedefs locaux non utilisés (nécessaire pour STATIC_ASSERT(x))
			"-Wno-#pragma-messages",              // don't consider #pragma message as warnings
			"-Wno-unneeded-internal-declaration", // ignore unused internal functions beeing stripped)
		)
	}

	rules.SystemIncludePaths.Append(
		clang.ProductInstall.InstallDir.Folder("include", "clang-c"),
		clang.ProductInstall.InstallDir.Folder("include", "llvm-c"),
		clang.ProductInstall.InstallDir.Folder("lib", "clang", clang.ProductInstall.Version, "include"),
	)
	rules.LibraryPaths.Append(
		clang.ProductInstall.InstallDir.Folder("lib"),
		clang.ProductInstall.InstallDir.Folder("lib", "clang", clang.ProductInstall.Version, "lib", "windows"),
	)

	// https://blog.llvm.org/posts/2021-04-05-constructor-homing-for-debug-info/
	rules.CompilerOptions.Append("-Xclang", "-fuse-ctor-homing")
	rules.PrecompiledHeaderOptions.Append("-Xclang", "-fuse-ctor-homing")

	return nil
}

/***************************************
 * Product install
 ***************************************/

var re_clangClVersion = regexp.MustCompile(`(?m)^clang\s+version\s+([\.\d]+)$`)

func (llvm *LlvmProductInstall) Alias() BuildAlias {
	return MakeBuildAlias("HAL", "Windows", "LLVM", "Latest")
}
func (llvm *LlvmProductInstall) Serialize(ar Archive) {
	ar.String(&llvm.Version)
	ar.Serializable(&llvm.InstallDir)
	ar.Serializable(&llvm.ClangCl_exe)
	ar.Serializable(&llvm.LlvmLib_exe)
	ar.Serializable(&llvm.LldLink_exe)
}
func (llvm *LlvmProductInstall) Build(bc BuildContext) error {
	candidatePaths := NewDirSet(
		MakeDirectory("C:/Program Files/LLVM"),
		MakeDirectory("C:/Program Files (x86)/LLVM"))

	for _, x := range candidatePaths {
		if x.Exists() {
			llvm.InstallDir = x
			llvm.ClangCl_exe = x.Folder("bin").File("clang-cl.exe")
			llvm.LlvmLib_exe = x.Folder("bin").File("llvm-lib.exe")
			llvm.LldLink_exe = x.Folder("bin").File("lld-link.exe")

			if fullVersion, err := exec.Command(llvm.ClangCl_exe.String(), "--version").Output(); err == nil {
				parsed := re_clangClVersion.FindStringSubmatch(string(fullVersion))
				if nil == parsed {
					return fmt.Errorf("failed to parse clang-cl version: %v", fullVersion)
				}
				llvm.Version = parsed[1]
			} else {
				return err
			}

			LogTrace("using LLVM v%s for Windows found in '%v'", llvm.Version, llvm.InstallDir)
			if err := bc.NeedFile(llvm.ClangCl_exe, llvm.LlvmLib_exe, llvm.LldLink_exe); err != nil {
				return err
			}

			return nil
		}
	}

	return fmt.Errorf("can't find LLVM for Windows install path")
}

func GetLlvmProductInstall() BuildFactoryTyped[*LlvmProductInstall] {
	return func(bi BuildInitializer) (*LlvmProductInstall, error) {
		return &LlvmProductInstall{}, nil
	}
}

func GetClangCompiler(arch ArchType) BuildFactoryTyped[Compiler] {
	return func(bi BuildInitializer) (Compiler, error) {
		compileFlags := GetCompileFlags()
		windowsFlags := GetWindowsFlags()
		if err := bi.NeedFactories(
			GetBuildableFlags(compileFlags),
			GetBuildableFlags(windowsFlags)); err != nil {
			return nil, err
		}

		return &ClangCompiler{
			MsvcCompiler: MsvcCompiler{
				Arch:          arch,
				CompilerRules: NewCompilerRules(NewCompilerAlias("clang-cl", "llvm", arch.String())),
			},
		}, nil
	}
}
