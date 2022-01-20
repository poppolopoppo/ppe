package windows

import (
	. "build/compile"
	"build/utils"
	. "build/utils"
	"bytes"
	"fmt"
	"time"
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
		f.AnalysisOptions.Append("/I\"" + x.String() + "\"")
		externalInc := "/imsvc\"" + x.String() + "\""
		f.CompilerOptions.Append(externalInc)
		f.PrecompiledHeaderOptions.Append(externalInc)
		f.PreprocessorOptions.Append(externalInc)
	}
}
func (clang *ClangCompiler) SystemIncludePath(facet *Facet, dirs ...Directory) {
	clang.ExternIncludePath(facet, dirs...)
}
func (clang *ClangCompiler) Decorate(compileEnv *CompileEnv, u *Unit) {
	clang.MsvcCompiler.Decorate(compileEnv, u)

	u.RemoveCompilationFlag("/WX", "/JMC-")
	u.LibrarianOptions.Remove("/WX", "/SUBSYSTEM:WINDOWS", "/NODEFAULTLIB")
	u.LinkerOptions.Remove("/WX", "/LTCG", "/LTCG:INCREMENTAL", "/LTCG:OFF", "/NODEFAULTLIB")

	switch compileEnv.GetPlatform().Arch {
	case ARCH_ARM:
	case ARCH_X86:
		u.AddCompilationFlag("-m32")
	case ARCH_ARM64:
	case ARCH_X64:
		u.AddCompilationFlag("-m64")
	default:
		UnexpectedValue(compileEnv.GetPlatform().Arch)
	}
}

func (clang *ClangCompiler) Alias() BuildAlias {
	return MakeBuildAlias("Compiler", "Clang_"+clang.Arch.String())
}
func (clang *ClangCompiler) Build(bc BuildContext) (BuildStamp, error) {
	llvm := GetLlvmProductInstall()
	msvc := GetMsvcCompiler(clang.Arch)
	windowsFlags := WindowsFlags.Need(CommandEnv.Flags)
	bc.DependsOn(llvm, msvc, windowsFlags)

	clang.ProductInstall = llvm
	clang.MsvcCompiler = *msvc

	rules := clang.GetCompiler()
	rules.CompilerFamily = "clang-cl"
	rules.Executable = clang.ProductInstall.ClangCl_exe
	rules.Librarian = clang.ProductInstall.LlvmLib_exe
	rules.Linker = clang.ProductInstall.LldLink_exe
	rules.ExtraFiles = NewFileSet(
		clang.ProductInstall.InstallDir.Folder("bin").File("msvcp140.dll"),
		clang.ProductInstall.InstallDir.Folder("bin").File("vcruntime140.dll"),
	)

	rules.Defines.Append("CPP_CLANG", "LLVM_FOR_WINDOWS", "_CRT_SECURE_NO_WARNINGS")
	rules.AddCompilationFlag(
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
		"/clang:-ftemplate-backtrace-limit=0",
	)

	if windowsFlags.Permissive {
		rules.AddCompilationFlag("-Wno-error")
	} else {
		rules.LibrarianOptions.Append("-Werror")
		rules.AddCompilationFlag(
			"-Werror",
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

	return MakeTimedBuildStamp(time.Now())
}

/***************************************
 * Product install
 ***************************************/

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
		MakeDirectory("C:/Program Files (x86)/LLVM"),
	)

	for _, x := range candidatePaths {
		if x.Exists() {
			llvm.InstallDir = x
			LogTrace("using LLVM for Windows found in '%v'", llvm.InstallDir)
			llvm.ClangCl_exe = x.Folder("bin").File("clang-cl.exe")
			llvm.LlvmLib_exe = x.Folder("bin").File("llvm-lib.exe")
			llvm.LldLink_exe = x.Folder("bin").File("lld-link.exe")
			bc.NeedFile(
				llvm.ClangCl_exe,
				llvm.LlvmLib_exe,
				llvm.LldLink_exe)
			return MakeBuildStamp(llvm)
		}
	}

	return BuildStamp{}, fmt.Errorf("can't find LLVM for Windows install path")
}

var GetLlvmProductInstall = utils.Memoize(func() *LlvmProductInstall {
	builder := &LlvmProductInstall{}
	return CommandEnv.BuildGraph().Create(builder).GetBuildable().(*LlvmProductInstall)
})

var GetClangCompiler = utils.MemoizeArg(func(arch ArchType) *ClangCompiler {
	result := &ClangCompiler{}
	result.Arch = arch
	return CommandEnv.BuildGraph().
		Create(result).
		GetBuildable().(*ClangCompiler)
})
