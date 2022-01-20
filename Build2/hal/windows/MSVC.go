package windows

import (
	. "build/compile"
	. "build/utils"
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os/exec"
	"strings"
	"time"
)

/***************************************
 * MSVC Compiler
 ***************************************/

type MsvcCompiler struct {
	Arch ArchType

	MSC_VER         MsvcVersion
	MinorVer        string
	Host            string
	Target          string
	PlatformToolset string
	VSInstallName   string
	VSInstallPath   Directory
	VCToolsPath     Directory

	CompilerRules
	*WindowsSDK
	ProductInstall *MsvcProductInstall
}

func (msvc *MsvcCompiler) GetCompiler() *CompilerRules { return &msvc.CompilerRules }

func (msvc *MsvcCompiler) GetDigestable(o *bytes.Buffer) {
	msvc.MSC_VER.GetDigestable(o)
	o.WriteString(msvc.MinorVer)
	o.WriteString(msvc.Host)
	o.WriteString(msvc.Target)
	o.WriteString(msvc.PlatformToolset)
	o.WriteString(msvc.VSInstallName)
	msvc.VSInstallPath.GetDigestable(o)
	msvc.VCToolsPath.GetDigestable(o)
	msvc.CompilerRules.GetDigestable(o)
	msvc.WindowsSDK.GetDigestable(o)
	msvc.ProductInstall.GetDigestable(o)
}

func (msvc *MsvcCompiler) EnvPath() DirSet {
	return NewDirSet(
		msvc.WorkingDir(),
		msvc.WindowsSDK.ResourceCompiler.Dirname)
}
func (msvc *MsvcCompiler) WorkingDir() Directory {
	return msvc.ProductInstall.VcToolsHostPath()
}
func (msvc *MsvcCompiler) Extname(x PayloadType) string {
	switch x {
	case PAYLOAD_EXECUTABLE:
		return ".exe"
	case PAYLOAD_OBJECTLIST:
		return ".obj"
	case PAYLOAD_STATICLIB:
		return ".lib"
	case PAYLOAD_SHAREDLIB:
		return ".dll"
	case PAYLOAD_PRECOMPILEDHEADER:
		return ".pch"
	case PAYLOAD_HEADERS:
		return ".h"
	default:
		UnexpectedValue(x)
		return ""
	}
}

func (msvc *MsvcCompiler) CppRtti(f *Facet, enabled bool) {
	if enabled {
		f.Defines.Append("PPE_HAS_CXXRTTI=1")
		f.AddCompilationFlag("/GR")
	} else {
		f.Defines.Append("PPE_HAS_CXXRTTI=0")
		f.AddCompilationFlag("/GR-")
	}
}
func (msvc *MsvcCompiler) CppStd(f *Facet, std CppStdType) {
	maxSupported := getCppStdFromMsc(msvc.MSC_VER)
	if int32(std) > int32(maxSupported) {
		std = maxSupported
	}
	switch std {
	case CPPSTD_20:
		f.AddCompilationFlag("/std:c++20")
	case CPPSTD_17:
		f.AddCompilationFlag("/std:c++17")
	case CPPSTD_14:
		f.AddCompilationFlag("/std:c++14")
	case CPPSTD_11:
		f.AddCompilationFlag("/std:c++11")
	}

}
func (msvc *MsvcCompiler) Define(f *Facet, def ...string) {
	for _, x := range def {
		f.AddCompilationFlag("/D" + x)
	}
}
func (msvc *MsvcCompiler) DebugSymbols(f *Facet, sym DebugType, output Filename) {
	switch sym {
	case DEBUG_DISABLED:
		f.LinkerOptions.Append("/DEBUG:NONE")
		return
	case DEBUG_SYMBOLS:
		pdbPath := output.ReplaceExt(".pdb").String()
		f.CompilerOptions.Append("/Z7")
		f.PrecompiledHeaderOptions.Append("/Z7")
		f.LinkerOptions.Append("/DEBUG", "/PDB:\""+pdbPath+"\"")
		return
	case DEBUG_EMBEDDED:
		f.CompilerOptions.Append("/Z7")
		f.PrecompiledHeaderOptions.Append("/Z7")
		f.LinkerOptions.Append("/DEBUG")
		return
	default:
		UnexpectedValue(sym)
	}
}
func (msvc *MsvcCompiler) Link(f *Facet, lnk LinkType) {
	switch lnk {
	case LINK_STATIC:
		return // nothing to do
	case LINK_DYNAMIC:
		// https://msdn.microsoft.com/en-us/library/527z7zfs.aspx
		f.LinkerOptions.Append("/DLL")
	default:
		UnexpectedValue(lnk)
	}
}
func (msvc *MsvcCompiler) PrecompiledHeader(f *Facet, mode PrecompiledHeaderType, header Filename, source Filename, object Filename) {
	switch mode {
	case PCH_MONOLITHIC:
		f.Defines.Append("BUILD_PCH=1")
		f.CompilerOptions.Append(
			"/Yu\""+header.Basename+"\"",
			"/Fp\""+object.String()+"\"")
		f.PrecompiledHeaderOptions.Append("/Yc\"" + header.Basename + "\"")
	case PCH_DISABLED:
		f.Defines.Append("BUILD_PCH=0")
	default:
		UnexpectedValue(mode)
	}
}
func (msvc *MsvcCompiler) Sanitizer(f *Facet, sanitizer SanitizerType) {
	switch sanitizer {
	case SANITIZER_NONE:
		return
	case SANITIZER_ADDRESS:
		// https://devblogs.microsoft.com/cppblog/addresssanitizer-asan-for-windows-with-msvc/
		f.Defines.Append("USE_PPE_SANITIZER=1")
		f.AddCompilationFlag("/fsanitize=address")
	default:
		UnexpectedValue(sanitizer)
	}
}

func (msvc *MsvcCompiler) CompilationFlag(f *Facet, flag ...string) {
	f.AnalysisOptions.Append(flag...)
	f.CompilerOptions.Append(flag...)
	f.PreprocessorOptions.Append(flag...)
	f.PrecompiledHeaderOptions.Append(flag...)
}
func (msvc *MsvcCompiler) ForceInclude(f *Facet, inc ...Filename) {
	for _, x := range inc {
		f.AddCompilationFlag("/FI\"" + x.String() + "\"")
	}
}
func (msvc *MsvcCompiler) IncludePath(f *Facet, dirs ...Directory) {
	for _, x := range dirs {
		f.AddCompilationFlag("/I\"" + x.String() + "\"")
	}
}
func (msvc *MsvcCompiler) ExternIncludePath(f *Facet, dirs ...Directory) {
	for _, x := range dirs {
		f.AnalysisOptions.Append("/I\"" + x.String() + "\"")
		externalInc := "/external:I\"" + x.String() + "\""
		f.CompilerOptions.Append(externalInc)
		f.PrecompiledHeaderOptions.Append(externalInc)
		f.PreprocessorOptions.Append(externalInc)
	}
}
func (msvc *MsvcCompiler) SystemIncludePath(facet *Facet, dirs ...Directory) {
	msvc.ExternIncludePath(facet, dirs...)
}
func (msvc *MsvcCompiler) Library(f *Facet, lib ...Filename) {
	for _, x := range lib {
		libInc := "\"" + x.String() + "\""
		f.LibrarianOptions.Append(libInc)
		f.LinkerOptions.Append(libInc)
	}
}
func (msvc *MsvcCompiler) LibraryPath(f *Facet, dirs ...Directory) {
	for _, x := range dirs {
		libPath := "/LIBPATH:\"" + x.String() + "\""
		f.LibrarianOptions.Append(libPath)
		f.LinkerOptions.Append(libPath)
	}
}

func makeMsvcCompiler(
	result *MsvcCompiler,
	bc BuildContext,
	msc_ver MsvcVersion,
	minorVer string,
	host string,
	target string,
	vsInstallName string,
	vsInstallPath Directory,
	vcToolsPath Directory,
	executable Filename,
	librarian Filename,
	linker Filename,
	extraFiles ...Filename) {
	compileFlags := CompileFlags.FindOrAdd(CommandEnv.Flags)
	windowsFlags := WindowsFlags.FindOrAdd(CommandEnv.Flags)
	windowsSDKInstall := GetWindowsSDKInstall(windowsFlags.WindowsSDK)
	bc.DependsOn(compileFlags, windowsFlags, windowsSDKInstall)

	platformToolset := fmt.Sprintf("%s%s%s", minorVer[0:1], minorVer[1:2], minorVer[3:4])

	result.MSC_VER = msc_ver
	result.MinorVer = minorVer
	result.Host = host
	result.Target = target
	result.PlatformToolset = platformToolset
	result.VSInstallName = vsInstallName
	result.VSInstallPath = vsInstallPath
	result.VCToolsPath = vcToolsPath
	result.WindowsSDK = windowsSDKInstall.WindowsSDK

	result.CompilerRules.CompilerName = fmt.Sprintf("Msvc_%v_%v", msc_ver, target)
	result.CompilerRules.CompilerFamily = "msvc"
	result.CompilerRules.CppStd = getCppStdFromMsc(msc_ver)
	result.CompilerRules.Executable = executable
	result.CompilerRules.Librarian = librarian
	result.CompilerRules.Linker = linker
	result.CompilerRules.ExtraFiles = extraFiles

	result.CompilerRules.Facet = NewFacet()
	facet := &result.CompilerRules.Facet

	facet.Append(result.WindowsSDK)

	facet.Defines.Append(
		"CPP_VISUALSTUDIO",
		"_ENABLE_EXTENDED_ALIGNED_STORAGE", // https://devblogs.microsoft.com/cppblog/stl-features-and-fixes-in-vs-2017-15-8/
	)

	facet.Exports.Add("VisualStudio/Path", result.VSInstallPath.String())
	facet.Exports.Add("VisualStudio/PlatformToolset", result.PlatformToolset)
	facet.Exports.Add("VisualStudio/Tools", result.VCToolsPath.String())
	facet.Exports.Add("VisualStudio/Version", result.MinorVer)

	facet.SystemIncludePaths.Append(
		vsInstallPath.Folder("VC", "Auxiliary", "VS", "include"),
		vsInstallPath.Folder("VC", "Tools", "MSVC", result.MinorVer, "crt", "src"),
		vsInstallPath.Folder("VC", "Tools", "MSVC", result.MinorVer, "include"))

	compilationArgs := []string{
		"/nologo",   // no copyright when compiling
		"/c \"%1\"", // input file injection
	}
	facet.CompilerOptions.Append(compilationArgs...)
	facet.PrecompiledHeaderOptions.Append(compilationArgs...)
	facet.PreprocessorOptions.Append(compilationArgs...)

	facet.CompilerOptions.Append("/Fo\"%2\"")
	facet.PrecompiledHeaderOptions.Append("/Fp\"%2\"", "/Fo\"%3\"")
	facet.PreprocessorOptions.Append("/Fo\"%2\"")

	facet.AddCompilationFlag(
		"/Gm-",     // minimal rebuild is handled by FASTBuild
		"/GF",      // string pooling
		"/GT",      // fiber safe optimizations (https://msdn.microsoft.com/fr-fr/library/6e298fy4.aspx)
		"/bigobj",  // more sections inside obj files, support larger translation units, needed for unity builds
		"/d2FH4",   // https://devblogs.microsoft.com/cppblog/msvc-backend-updates-in-visual-studio-2019-preview-2/
		"/EHsc",    // structure exception support (#TODO: optional ?)
		"/fp:fast", // non-deterministic, allow vendor specific float intrinsics (https://msdn.microsoft.com/fr-fr/library/tzkfha43.aspx)
		"/vmb",     // class is always defined before pointer to member (https://docs.microsoft.com/en-us/cpp/build/reference/vmb-vmg-representation-method?view=vs-2019)
		"/openmp-", // disable OpenMP automatic parallelization
		//"/Za",                // disable non-ANSI features
		"/Zc:inline",           // https://msdn.microsoft.com/fr-fr/library/dn642448.aspx
		"/Zc:implicitNoexcept", // https://msdn.microsoft.com/fr-fr/library/dn818588.aspx
		"/Zc:preprocessor",     // https://devblogs.microsoft.com/cppblog/announcing-full-support-for-a-c-c-conformant-preprocessor-in-msvc/
		"/Zc:rvalueCast",       // https://msdn.microsoft.com/fr-fr/library/dn449507.aspx
		"/Zc:strictStrings",    // https://msdn.microsoft.com/fr-fr/library/dn449508.aspx
		"/Zc:wchar_t",          // promote wchar_t as a native type
		"/Zc:forScope",         // prevent from spilling iterators outside loops
		"/utf-8",               // https://docs.microsoft.com/fr-fr/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8
		"/W4",                  // warning level 4 (verbose)
		"/TP",                  // compile as C++
	)

	// ignored warnings
	facet.AddCompilationFlag(
		"/wd4201", // nonstandard extension used: nameless struct/union'
		"/wd4251", // 'XXX' needs to have dll-interface to be used by clients of class 'YYY'
	)

	// configure librarian
	facet.LibrarianOptions.Append(
		"/OUT:\"%2\"",
		"\"%1\"",
		"/nologo",
		"/SUBSYSTEM:WINDOWS",
		"/IGNORE:4221",
	)

	// configure linker
	facet.LinkerOptions.Append(
		"/OUT:\"%2\"",
		"\"%1\"",
		"kernel32.lib",
		"Shell32.lib",
		"Gdi32.lib",
		"Shlwapi.lib",
		"Version.lib",
		"/nologo",            // no copyright when compiling
		"/TLBID:1",           // https://msdn.microsoft.com/fr-fr/library/b1kw34cb.aspx
		"/IGNORE:4001",       // https://msdn.microsoft.com/en-us/library/aa234697(v=vs.60).aspx
		"/IGNORE:4099",       // don't have PDB for some externals
		"/NXCOMPAT:NO",       // disable Data Execution Prevention (DEP)
		"/LARGEADDRESSAWARE", // indicate support for VM > 2Gb (if 3Gb flag is toggled)
		"/VERBOSE:INCR",      // incremental linker diagnosis
		"/SUBSYSTEM:WINDOWS", // ~Windows~ application type (vs Console)
		"/fastfail",          // better error reporting
	)

	// strict vs permissive
	if windowsFlags.Permissive {
		LogVeryVerbose("MSVC: using permissive compilation options")

		facet.AddCompilationFlag("/permissive", "/WX-")
		//facet.LinkerOptions.Append("/WX-")
		facet.LibrarianOptions.Append("/WX-")
	} else {
		LogVeryVerbose("MSVC: using strict warnings and warings as error")

		facet.AddCompilationFlag(
			// https://docs.microsoft.com/en-us/cpp/build/reference/permissive-standards-conformance
			"/permissive-",
			// warning as errors
			"/WX",
			// promote some warnings as errors
			"/we4062", // enumerator 'identifier' in a switch of enum 'enumeration' is not handled
			"/we4263", // 'function' : member function does not override any base class virtual member function
			"/we4265", // 'class': class has virtual functions, but destructor is not virtual // not handler by boost and stl
			"/we4296", // 'operator': expression is always false
			"/we4555", // expression has no effect; expected expression with side-effect
			"/we4619", // #pragma warning : there is no warning number 'number'
			"/we4640", // 'instance' : construction of local static object is not thread-safe
			"/we4826", // Conversion from 'type1 ' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
			"/we4836", // nonstandard extension used : 'type' : local types or unnamed types cannot be used as template arguments
			"/we4905", // wide string literal cast to 'LPSTR'
			"/we4906", // string literal cast to 'LPWSTR'
		)

		// warning as errors also for librarian and linker
		facet.LibrarianOptions.Append("/WX")
		//facet.LinkerOptions.Append("/WX") // #TODO: **DON'T**, will freeze link.exe ¯\_(ツ)_/¯
	}

	if compileFlags.Benchmark {
		LogVeryVerbose("MSVC: will dump compilation timings")
		facet.CompilerOptions.Append("/d2cgsummary", "/Bt+")
		facet.LinkerOptions.Append("/d2:-cgsummary")
	}

	if windowsFlags.JustMyCode && msc_ver >= MSC_VER_2019 {
		LogVeryVerbose("MSVC: using just-my-code")
		facet.AddCompilationFlag_NoAnalysis("/JMC")
	} else if msc_ver >= MSC_VER_2019 {
		facet.AddCompilationFlag_NoAnalysis("/JMC-")
	}

	if msc_ver >= MSC_VER_2019 {
		LogVeryVerbose("MSCV: using external headers to ignore warnings in foreign code")
		// https://docs.microsoft.com/fr-fr/cpp/build/reference/jmc?view=msvc-160
		facet.Defines.Append("USE_PPE_MSVC_PRAGMA_SYSTEMHEADER")
		facet.AddCompilationFlag_NoAnalysis(
			"/experimental:external",
			"/external:W0",
			"/external:anglebrackets")
	}

	// Windows 10 slow-down workaround
	facet.LinkerOptions.Append(
		"delayimp.lib",
		"/DELAYLOAD:Shell32.dll",
		"/IGNORE:4199", // warning LNK4199: /DELAYLOAD:XXX.dll ignored; no imports found from XXX.dll, caused by our added .libs
	)
}

func (msvc *MsvcCompiler) Decorate(compileEnv *CompileEnv, u *Unit) {
	compileFlags := CompileFlags.FindOrAdd(CommandEnv.Flags)
	windowsFlags := WindowsFlags.FindOrAdd(CommandEnv.Flags)

	switch compileEnv.GetPlatform().Arch {
	case ARCH_X86:
		u.AddCompilationFlag("/favor:blend", "/arch:AVX2")
		u.LibrarianOptions.Append("/MACHINE:x86")
		u.LinkerOptions.Append("/MACHINE:x86", "/SAFESEH")
		u.LibraryPaths.Append(
			msvc.VSInstallPath.Folder("VC", "Tools", "MSVC", msvc.MinorVer, "lib", "x86"),
			msvc.VSInstallPath.Folder("VC", "Auxiliary", "VS", "lib", "x86"))
	case ARCH_X64:
		u.AddCompilationFlag("/favor:AMD64", "/arch:AVX2")
		u.LibrarianOptions.Append("/MACHINE:x64")
		u.LinkerOptions.Append("/MACHINE:x64")
		u.LibraryPaths.Append(
			msvc.VSInstallPath.Folder("VC", "Tools", "MSVC", msvc.MinorVer, "lib", "x64"),
			msvc.VSInstallPath.Folder("VC", "Auxiliary", "VS", "lib", "x64"))
	default:
		UnexpectedValue(compileEnv.GetPlatform().Arch)
	}

	switch compileEnv.GetConfig().ConfigType {
	case CONFIG_DEBUG:
		decorateMsvcConfig_Debug(&u.Facet)
	case CONFIG_FASTDEBUG:
		decorateMsvcConfig_FastDebug(&u.Facet)
	case CONFIG_DEVEL:
		decorateMsvcConfig_Devel(&u.Facet)
	case CONFIG_TEST:
		decorateMsvcConfig_Test(&u.Facet)
	case CONFIG_SHIPPING:
		decorateMsvcConfig_Shipping(&u.Facet)
	default:
		UnexpectedValue(compileEnv.GetConfig().ConfigType)
	}

	// set default thread stack size
	stackSize := windowsFlags.StackSize
	if u.Sanitizer != SANITIZER_NONE {
		stackSize *= 2
		LogVeryVerbose("MSVC: doubling thread stack size due to sanitizer (%d)", stackSize)
	}
	u.AddCompilationFlag(fmt.Sprintf("/F\"%d\"", stackSize))
	u.LinkerOptions.Append(fmt.Sprintf("/STACK:%d", stackSize))

	if windowsFlags.Analyze {
		LogVeryVerbose("MSVC: using static analysis")

		u.AnalysisOptions.Append(
			"/analyze",
			fmt.Sprint("/analyse:stacksize", stackSize),
			fmt.Sprintf("/analyze:plugin\"%v\"", msvc.ProductInstall.VcToolsHostPath().File("EspXEngine.dll")),
		)
		u.Defines.Append("ANALYZE")
		u.LinkerOptions.Append(
			"/VERBOSE",
			// "/VERBOSE:LIB",
			// "/VERBOSE:ICF",
			// "/VERBOSE:REF",
			"/VERBOSE:UNUSEDLIBS",
		)
	}

	// set dependent linker options

	if u.Sanitizer != SANITIZER_NONE && u.CompilerOptions.Contains("/LTCG") {
		LogVeryVerbose("MSVC: disable LTCG due to %v", u.Sanitizer)
		u.LinkerOptions.Remove("/LTCG")
	}

	if compileFlags.Incremental && u.Sanitizer == SANITIZER_NONE {
		LogVeryVerbose("MSVC: using incremental linker with fastlink")
		if u.LinkerOptions.Contains("/LTCG") {
			u.LinkerOptions.Remove("/LTCG")
			u.LinkerOptions.Append("/LTCG:INCREMENTAL")
		} else if !u.LinkerOptions.Contains("/LTCG:INCREMENTAL") {
			u.LinkerOptions.Append("/INCREMENTAL")
		}
	} else if !u.LinkerOptions.Contains("/INCREMENTAL") {
		LogVeryVerbose("MSVC: using non-incremental linker")
		u.LinkerOptions.Append("/INCREMENTAL:NO")
	}

	// enable perfSDK if necessary
	if windowsFlags.PerfSDK {
		LogVeryVerbose("MSVC: using Windows PerfSDK")
		var perfSDK Directory
		switch compileEnv.GetPlatform().Arch {
		case ARCH_X64:
			perfSDK = msvc.VSInstallPath.Folder("Team Tools", "Performance Tools", "x64", "PerfSDK")
		case ARCH_X86:
			perfSDK = msvc.VSInstallPath.Folder("Team Tools", "Performance Tools", "PerfSDK")
		default:
			UnexpectedValue(compileEnv.GetPlatform().Arch)
		}
		u.Defines.Append("WITH_VISUALSTUDIO_PERFSDK")
		u.ExternIncludePaths.Append(perfSDK)
		u.LibraryPaths.Append(perfSDK)
	}

	// don't forget the current windows SDK
	msvc.WindowsSDK.Decorate(compileEnv, u)
}

/***************************************
 * Compiler options per configuration
 ***************************************/

func msvc_CXX_runtimeLibrary(f *Facet, staticCrt bool, debug bool) {
	var runtimeFlag string
	var suffix string
	if debug {
		suffix = "d"
	}
	if staticCrt {
		runtimeFlag = "/MT"
		f.AddCompilationFlag(
			"LIBCMT"+suffix+".LIB",
			"libvcruntime"+suffix+".lib",
			"libucrt"+suffix+".lib")
	} else {
		runtimeFlag = "/MD"
	}
	f.AddCompilationFlag(runtimeFlag + suffix)
}
func msvc_CXX_linkTimeCodeGeneration(f *Facet, enabled bool) {
	if !f.LinkerOptions.Any("/LTCG", "/LTCG:OFF", "/LTCG:INCREMENTAL") {
		if enabled {
			f.LinkerOptions.Append("/LTCG")
		} else {
			f.LinkerOptions.Append("/LTCG:OFF")
		}
	}
}
func msvc_CXX_runtimeChecks(f *Facet, enabled bool, rtc1 bool) {
	if enabled {
		// https://msdn.microsoft.com/fr-fr/library/jj161081(v=vs.140).aspx
		f.AddCompilationFlag("/GS", "/sdl")
		if rtc1 {
			// https://msdn.microsoft.com/fr-fr/library/8wtf2dfz.aspx
			f.AddCompilationFlag("/RTC1")
		}
		f.LinkerOptions.Append("/GUARD:CF")
	} else {
		f.AddCompilationFlag("/GS-", "/sdl-")
		f.LinkerOptions.Append("/GUARD:NO")
	}
}
func msvc_STL_debugHeap(f *Facet, enabled bool) {
	if !enabled {
		f.Defines.Append("_NO_DEBUG_HEAP=1")
	}
}
func msvc_STL_iteratorDebug(f *Facet, enabled bool) {
	if enabled {
		f.Defines.Append(
			"_SECURE_SCL=1",             // https://msdn.microsoft.com/fr-fr/library/aa985896.aspx
			"_ITERATOR_DEBUG_LEVEL=2",   // https://msdn.microsoft.com/fr-fr/library/hh697468.aspx
			"_HAS_ITERATOR_DEBUGGING=1") // https://msdn.microsoft.com/fr-fr/library/aa985939.aspx")
	} else {
		f.Defines.Append(
			"_SECURE_SCL=0",             // https://msdn.microsoft.com/fr-fr/library/aa985896.aspx
			"_ITERATOR_DEBUG_LEVEL=0",   // https://msdn.microsoft.com/fr-fr/library/hh697468.aspx
			"_HAS_ITERATOR_DEBUGGING=0") // https://msdn.microsoft.com/fr-fr/library/aa985939.aspx
	}
}

func decorateMsvcConfig_Debug(f *Facet) {
	f.AddCompilationFlag("/Od", "/Oy-", "/Gw-")
	f.LinkerOptions.Append("/DYNAMICBASE:NO", "/OPT:NOREF", "/OPT:NOICF")
	compileFlags := CompileFlags.Need(CommandEnv.Flags)
	msvc_CXX_runtimeLibrary(f, WindowsFlags.Need(CommandEnv.Flags).StaticCRT.Get(), true)
	msvc_CXX_linkTimeCodeGeneration(f, false)
	msvc_CXX_runtimeChecks(f, compileFlags.RuntimeChecks.Get(), true)
	msvc_STL_debugHeap(f, true)
	msvc_STL_iteratorDebug(f, true)
}
func decorateMsvcConfig_FastDebug(f *Facet) {
	f.AddCompilationFlag("/Ob1", "/Oy-", "/Gw-", "/Zo")
	f.LinkerOptions.Append("/DYNAMICBASE:NO", "/OPT:NOREF", "/OPT:NOICF")
	compileFlags := CompileFlags.Need(CommandEnv.Flags)
	msvc_CXX_runtimeLibrary(f, WindowsFlags.Need(CommandEnv.Flags).StaticCRT.Get(), true)
	msvc_CXX_linkTimeCodeGeneration(f, false)
	msvc_CXX_runtimeChecks(f, compileFlags.RuntimeChecks.Get(), false)
	msvc_STL_debugHeap(f, true)
	msvc_STL_iteratorDebug(f, true)
}
func decorateMsvcConfig_Devel(f *Facet) {
	f.AddCompilationFlag("/O2", "/Oy-", "/GA", "/Zo", "/GL")
	f.LinkerOptions.Append("/DYNAMICBASE:NO", "/OPT:NOICF")
	msvc_CXX_runtimeLibrary(f, WindowsFlags.Need(CommandEnv.Flags).StaticCRT.Get(), false)
	msvc_CXX_linkTimeCodeGeneration(f, true)
	msvc_CXX_runtimeChecks(f, false, false)
	msvc_STL_debugHeap(f, false)
	msvc_STL_iteratorDebug(f, false)
}
func decorateMsvcConfig_Test(f *Facet) {
	f.AddCompilationFlag("/O2", "/Ob3", "/Gw", "/Gy", "/GL", "/GA", "/Zo")
	f.LinkerOptions.Append("/DYNAMICBASE", "/PROFILE", "/OPT:REF")
	msvc_CXX_runtimeLibrary(f, WindowsFlags.Need(CommandEnv.Flags).StaticCRT.Get(), false)
	msvc_CXX_linkTimeCodeGeneration(f, true)
	msvc_CXX_runtimeChecks(f, false, false)
	msvc_STL_debugHeap(f, false)
	msvc_STL_iteratorDebug(f, false)
}
func decorateMsvcConfig_Shipping(f *Facet) {
	f.AddCompilationFlag("/O2", "/Ob3", "/Gw", "/Gy", "/GL", "/GA", "/Zo")
	f.LinkerOptions.Append("/DYNAMICBASE", "/OPT:REF", "/OPT:ICF=3")
	msvc_CXX_runtimeLibrary(f, WindowsFlags.Need(CommandEnv.Flags).StaticCRT.Get(), false)
	msvc_CXX_linkTimeCodeGeneration(f, true)
	msvc_CXX_runtimeChecks(f, false, false)
	msvc_STL_debugHeap(f, false)
	msvc_STL_iteratorDebug(f, false)
}

/***************************************
 * Compiler detection
 ***************************************/

var MSVC_VSWHERE_EXE = UFS.Build.Folder("hal", "windows", "bin").File("vswhere.exe")

type VsWhereCatalog struct {
	ProductDisplayVersion string
	ProductLineVersion    string
}

func (x *VsWhereCatalog) GetDigestable(o *bytes.Buffer) {
	o.WriteString(x.ProductDisplayVersion)
	o.WriteString(x.ProductLineVersion)
}

type VsWhereEntry struct {
	InstallationName string
	InstallationPath string
	Catalog          VsWhereCatalog
}

func (x *VsWhereEntry) GetDigestable(o *bytes.Buffer) {
	o.WriteString(x.InstallationName)
	o.WriteString(x.InstallationPath)
	x.Catalog.GetDigestable(o)
}

type MsvcProductInstall struct {
	Arch      string
	WantedVer MsvcVersion
	Insider   bool

	ActualVer      MsvcVersion
	HostArch       string
	Selected       VsWhereEntry
	VsInstallPath  Directory
	VcToolsPath    Directory
	VcToolsFileSet FileSet
	Cl_exe         Filename
	Lib_exe        Filename
	Link_exe       Filename
}

func (x *MsvcProductInstall) VcToolsHostPath() Directory {
	return x.VcToolsPath.Folder("bin", "Host"+x.HostArch, x.Arch)
}

func (x *MsvcProductInstall) Alias() BuildAlias {
	name := fmt.Sprintf("MsvcProductInstall_%s_%s", x.WantedVer, x.Arch)
	if x.Insider {
		name += "_Insider"
	}
	return MakeBuildAlias("HAL", name)
}
func (x *MsvcProductInstall) GetDigestable(o *bytes.Buffer) {
	o.WriteString(x.Arch)
	x.WantedVer.GetDigestable(o)
	if x.Insider {
		o.WriteByte(0xFF)
	} else {
		o.WriteByte(0x00)
	}
	x.ActualVer.GetDigestable(o)
	o.WriteString(x.HostArch)
	x.Selected.GetDigestable(o)
	x.VsInstallPath.GetDigestable(o)
	x.VcToolsPath.GetDigestable(o)
	x.VcToolsFileSet.GetDigestable(o)
	x.Cl_exe.GetDigestable(o)
	x.Lib_exe.GetDigestable(o)
	x.Link_exe.GetDigestable(o)
}
func (x *MsvcProductInstall) Build(bc BuildContext) (BuildStamp, error) {
	x.HostArch = getWindowsHostPlatform()

	name := fmt.Sprintf("MSVC_Host%v_%v", x.HostArch, x.Arch)
	if x.Insider {
		name += "_Insider"
	}

	// https://github.com/microsoft/vswhere/wiki/Find-VC#powershell
	var args = []string{
		"-format", "json",
		"-latest",
		"-products", "*",
		"-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
	}

	switch x.WantedVer {
	case msc_ver_any: // don't filter
	case MSC_VER_2022:
		args = append(args, "-version", "[17.0,18.0)")
	case MSC_VER_2019:
		args = append(args, "-version", "[16.0,17.0)")
	case MSC_VER_2017:
		args = append(args, "-version", "[15.0,16.0)")
	case MSC_VER_2015:
		args = append(args, "-version", "[14.0,15.0)")
	case MSC_VER_2013:
		args = append(args, "-version", "[13.0,14.0)")
	default:
		UnexpectedValue(x.WantedVer)
	}

	if x.Insider {
		args = append(args, "-prerelease")
	}

	LogTrace(strings.Join(append([]string{MSVC_VSWHERE_EXE.String()}, args...), " "))

	cmd := exec.Command(MSVC_VSWHERE_EXE.String(), args...)

	var entries []VsWhereEntry
	if outp, err := cmd.Output(); err != nil {
		return BuildStamp{}, err
	} else if len(outp) > 0 {
		if err := json.Unmarshal(outp, &entries); err != nil {
			return BuildStamp{}, err
		}
	}

	if len(entries) == 0 {
		return BuildStamp{}, fmt.Errorf("msvc: vswhere did not find any compiler")
	}

	x.Selected = entries[0]
	x.VsInstallPath = MakeDirectory(x.Selected.InstallationPath)
	if _, err := x.VsInstallPath.Info(); err != nil {
		return BuildStamp{}, err
	}

	if err := x.ActualVer.Set(x.Selected.Catalog.ProductLineVersion); err != nil {
		return BuildStamp{}, err
	}

	var vcToolsVersion string
	vcToolsVersionFile := x.VsInstallPath.Folder("VC", "Auxiliary", "Build").File("Microsoft.VCToolsVersion.default.txt")
	if data, err := ioutil.ReadFile(vcToolsVersionFile.String()); err == nil {
		vcToolsVersion = strings.TrimSpace(string(data))
	} else {
		return BuildStamp{}, err
	}

	x.VcToolsPath = x.VsInstallPath.Folder("VC", "Tools", "MSVC", vcToolsVersion)

	vcToolsHostPath := x.VcToolsHostPath()

	x.VcToolsFileSet = FileSet{}
	x.VcToolsFileSet.Append(
		vcToolsHostPath.File("c1.dll"),
		vcToolsHostPath.File("c1xx.dll"),
		vcToolsHostPath.File("c2.dll"),
		vcToolsHostPath.File("msobj140.dll"),
		vcToolsHostPath.File("mspdb140.dll"),
		vcToolsHostPath.File("mspdbcore.dll"),
		vcToolsHostPath.File("mspdbsrv.exe"),
		vcToolsHostPath.File("mspft140.dll"),
		vcToolsHostPath.File("msvcp140.dll"),
		vcToolsHostPath.File("msvcp140_atomic_wait.dll"), // Required circa 16.8.3 (14.28.29333)
		vcToolsHostPath.File("tbbmalloc.dll"),            // Required as of 16.2 (14.22.27905)
		vcToolsHostPath.File("vcruntime140.dll"),
		vcToolsHostPath.File("vcruntime140_1.dll")) // Required as of 16.5.1 (14.25.28610)

	if cluiDll, err := vcToolsHostPath.FindFileRec(MakeGlobRegexp("clui.dll")); err == nil {
		x.VcToolsFileSet.Append(
			cluiDll,
			cluiDll.Dirname.File("mspft140ui.dll")) // Localized messages for static analysis
	} else {
		return BuildStamp{}, err
	}

	x.Cl_exe = vcToolsHostPath.File("cl.exe")
	x.Lib_exe = vcToolsHostPath.File("lib.exe")
	x.Link_exe = vcToolsHostPath.File("link.exe")
	bc.NeedFile(MSVC_VSWHERE_EXE, vcToolsVersionFile, x.Cl_exe, x.Lib_exe, x.Link_exe)
	bc.NeedFile(x.VcToolsFileSet...)
	bc.NeedFolder(x.VcToolsPath, x.VsInstallPath)

	return MakeBuildStamp(x)
}

func (msvc *MsvcCompiler) Alias() BuildAlias {
	return MakeBuildAlias("Compiler", "Msvc_"+msvc.Arch.String())
}
func (msvc *MsvcCompiler) Build(bc BuildContext) (BuildStamp, error) {
	*msvc = MsvcCompiler{Arch: msvc.Arch}

	windowsFlags := WindowsFlags.Need(CommandEnv.Flags)
	msvc.ProductInstall = GetMsvcProductInstall(msvc.Arch, windowsFlags.MscVer, windowsFlags.Insider)
	bc.DependsOn(windowsFlags, msvc.ProductInstall)

	makeMsvcCompiler(
		msvc,
		bc,
		msvc.ProductInstall.ActualVer,
		msvc.ProductInstall.VcToolsPath.Basename(),
		msvc.ProductInstall.HostArch,
		msvc.Arch.String(),
		msvc.ProductInstall.Selected.InstallationName,
		msvc.ProductInstall.VsInstallPath,
		msvc.ProductInstall.VcToolsPath,
		msvc.ProductInstall.Cl_exe,
		msvc.ProductInstall.Lib_exe,
		msvc.ProductInstall.Link_exe,
		msvc.ProductInstall.VcToolsFileSet...)
	return MakeTimedBuildStamp(time.Now())
}

var GetMsvcProductInstall = MemoizeArgs3(func(arch ArchType, msc_ver MsvcVersion, insider BoolVar) *MsvcProductInstall {
	if msc_ver == MSC_VER_LATEST {
		msc_ver = msc_ver_any
	}

	builder := &MsvcProductInstall{
		WantedVer: msc_ver,
		Arch:      arch.String(),
		Insider:   insider.Get(),
	}

	return CommandEnv.BuildGraph().Create(builder).GetBuildable().(*MsvcProductInstall)
})

var GetMsvcCompiler = MemoizeArg(func(arch ArchType) *MsvcCompiler {
	result := &MsvcCompiler{
		Arch: arch,
	}
	return CommandEnv.BuildGraph().
		Create(result).
		GetBuildable().(*MsvcCompiler)
})
