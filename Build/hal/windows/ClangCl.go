package windows

import (
	. "build/compile"
	. "build/utils"
	"bytes"
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

func (clang *ClangCompiler) FriendlyName() string {
	return "clang"
}
func (clang *ClangCompiler) EnvPath() DirSet {
	return NewDirSet(
		clang.WorkingDir(),
		clang.WindowsSDK.ResourceCompiler.Dirname)
}
func (clang *ClangCompiler) WorkingDir() Directory {
	return clang.ProductInstall.InstallDir.Folder("bin")
}
func (clang *ClangCompiler) ExternIncludePath(f *Facet, dirs ...Directory) {
	for _, x := range dirs {
		f.AddCompilationFlag_NoAnalysis("/imsvc\"" + x.String() + "\"")
	}
}
func (clang *ClangCompiler) SystemIncludePath(facet *Facet, dirs ...Directory) {
	clang.ExternIncludePath(facet, dirs...)
}
func (clang *ClangCompiler) DebugSymbols(f *Facet, sym DebugType, output Filename, intermediate Directory) {
	clang.MsvcCompiler.DebugSymbols(f, sym, output, intermediate)

	// https://blog.llvm.org/2018/01/improving-link-time-on-windows-with.html
	if f.LinkerOptions.Contains("/DEBUG") {
		//f.CompilerOptions.Append("-mllvm", "-emit-codeview-ghash-section")
		f.LinkerOptions.Remove("/DEBUG")
		f.LinkerOptions.Append("/DEBUG:GHASH")
	}

	// not supported by clang-cl
	f.RemoveCompilationFlag("/Zf")
}
func (clang *ClangCompiler) Decorate(compileEnv *CompileEnv, u *Unit) {
	clang.MsvcCompiler.Decorate(compileEnv, u)

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

	// if u.Payload == PAYLOAD_SHAREDLIB {
	// 	// https://blog.llvm.org/2018/11/30-faster-windows-builds-with-clang-cl_14.html
	// 	u.CompilerOptions.Append("/Zc:dllexportInlines-") // not workig with /MD and std
	// 	u.PrecompiledHeaderOptions.Append("/Zc:dllexportInlines-")
	// }
}
func (clang *ClangCompiler) GetDigestable(o *bytes.Buffer) {
	clang.ProductInstall.GetDigestable(o)
	clang.MsvcCompiler.GetDigestable(o)
}

func (clang *ClangCompiler) Alias() BuildAlias {
	return MakeBuildAlias("Compiler", "Clang_"+clang.Arch.String())
}
func (clang *ClangCompiler) Build(bc BuildContext) (BuildStamp, error) {
	llvm := GetLlvmProductInstall()
	msvc := GetMsvcCompiler(clang.Arch)
	compileFlags := CompileFlags.Need(CommandEnv.Flags)
	windowsFlags := WindowsFlags.Need(CommandEnv.Flags)
	bc.DependsOn(llvm, msvc, compileFlags, windowsFlags)

	clang.ProductInstall = llvm
	clang.MsvcCompiler = *msvc

	rules := clang.GetCompiler()
	rules.CompilerName = fmt.Sprintf("ClangCl_%s_%s", SanitizeIdentifier(llvm.Version), rules.CompilerName)
	rules.CompilerFamily = "clang-cl"
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

	return MakeBuildStamp(clang)
}

/***************************************
 * Product install
 ***************************************/

var re_clangClVersion = regexp.MustCompile(`(?m)^clang\s+version\s+([\.\d]+)$`)

func (llvm *LlvmProductInstall) Alias() BuildAlias {
	return MakeBuildAlias("HAL", "LlvmForWindows_Latest")
}
func (llvm *LlvmProductInstall) GetDigestable(o *bytes.Buffer) {
	o.WriteString(llvm.Version)
	llvm.InstallDir.GetDigestable(o)
	llvm.ClangCl_exe.GetDigestable(o)
	llvm.LlvmLib_exe.GetDigestable(o)
	llvm.LldLink_exe.GetDigestable(o)
}
func (llvm *LlvmProductInstall) Build(bc BuildContext) (BuildStamp, error) {
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
					return BuildStamp{}, fmt.Errorf("failed to parse clang-cl version: %v", fullVersion)
				}
				llvm.Version = parsed[1]
			} else {
				return BuildStamp{}, err
			}

			LogTrace("using LLVM v%s for Windows found in '%v'", llvm.Version, llvm.InstallDir)
			bc.NeedFile(
				llvm.ClangCl_exe,
				llvm.LlvmLib_exe,
				llvm.LldLink_exe)
			return MakeBuildStamp(llvm)
		}
	}

	return BuildStamp{}, fmt.Errorf("can't find LLVM for Windows install path")
}

var GetLlvmProductInstall = Memoize(func() *LlvmProductInstall {
	builder := &LlvmProductInstall{}
	return CommandEnv.BuildGraph().Create(builder).GetBuildable().(*LlvmProductInstall)
})

var GetClangCompiler = MemoizeArg(func(arch ArchType) *ClangCompiler {
	result := &ClangCompiler{}
	result.Arch = arch
	return CommandEnv.BuildGraph().
		Create(result).
		GetBuildable().(*ClangCompiler)
})
