package windows

import (
	. "build/compile"
	. "build/utils"
	"encoding/json"
	"fmt"
	"os"
	"os/exec"
	"strings"
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

	WindowsSDK       *WindowsSDK
	ProductInstall   *MsvcProductInstall
	ResourceCompiler *ResourceCompiler
}

func (msvc *MsvcCompiler) GetCompiler() *CompilerRules { return &msvc.CompilerRules }

func (msvc *MsvcCompiler) Serialize(ar Archive) {
	ar.Serializable(&msvc.Arch)

	ar.Serializable(&msvc.MSC_VER)
	ar.String(&msvc.MinorVer)
	ar.String(&msvc.Host)
	ar.String(&msvc.Target)
	ar.String(&msvc.PlatformToolset)
	ar.String(&msvc.VSInstallName)
	ar.Serializable(&msvc.VSInstallPath)
	ar.Serializable(&msvc.VCToolsPath)

	ar.Serializable(&msvc.CompilerRules)
	SerializeExternal(ar, &msvc.WindowsSDK)
	SerializeExternal(ar, &msvc.ProductInstall)
	SerializeExternal(ar, &msvc.ResourceCompiler)
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
	case PAYLOAD_DEBUGSYMBOLS:
		return ".pdb"
	case PAYLOAD_DEPENDENCIES:
		return ".obj.json"
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
	case CPPSTD_LATEST, CPPSTD_20:
		f.AddCompilationFlag("/std:c++20")
	case CPPSTD_17:
		f.AddCompilationFlag("/std:c++17")
	case CPPSTD_14:
		f.AddCompilationFlag("/std:c++14")
	case CPPSTD_11:
		f.AddCompilationFlag("/std:c++11")
	default:
		UnexpectedValue(std)
	}

}
func (msvc *MsvcCompiler) AllowCaching(u *Unit, payload PayloadType) (result CacheModeType) {
	switch payload {
	case PAYLOAD_OBJECTLIST, PAYLOAD_PRECOMPILEDHEADER:
		if u.DebugSymbols == DEBUG_EMBEDDED {
			result = CACHE_READWRITE
		} else {
			result = CACHE_NONE
			LogDebug("%v/%v: can't use caching with %v debug symbols", u, payload, u.DebugSymbols)
		}
	case PAYLOAD_EXECUTABLE, PAYLOAD_SHAREDLIB:
		if !u.Incremental.Get() {
			result = CACHE_READWRITE
		} else {
			result = CACHE_NONE
			LogDebug("%v/%v: can't use caching with incremental linker", u, payload)
		}
	case PAYLOAD_STATICLIB, PAYLOAD_DEBUGSYMBOLS:
		result = CACHE_READWRITE
	}
	if result == CACHE_INHERIT {
		result = CACHE_NONE
	}
	if result != CACHE_NONE && !u.Deterministic.Get() {
		result = CACHE_NONE
		LogDebug("%v/%v: can't use caching without determinism", u, payload)
	}
	return result
}
func (msvc *MsvcCompiler) Define(f *Facet, def ...string) {
	for _, x := range def {
		f.AddCompilationFlag_NoAnalysis("/D" + x)
	}
}
func (msvc *MsvcCompiler) DebugSymbols(u *Unit) {
	artifactPDB := u.OutputFile.ReplaceExt(".pdb")

	switch u.DebugSymbols {
	case DEBUG_DISABLED:
		u.LinkerOptions.Append("/DEBUG:NONE")
		return

	case DEBUG_EMBEDDED:
		u.AddCompilationFlag_NoPreprocessor("/Z7")

		if u.Payload.HasLinker() {
			u.SymbolsFile = artifactPDB
			u.LinkerOptions.Append("/DEBUG", "/PDB:\""+MakeLocalFilename(artifactPDB)+"\"")

			if u.DebugFastLink.Get() {
				u.LinkerOptions.Append("/DEBUG:FASTLINK")
			}
		}

	case DEBUG_SYMBOLS:
		intermediatePDB := u.OutputFile.ReplaceExt("-Intermediate.pdb")
		intermediatePDB.Dirname = u.IntermediateDir

		u.SymbolsFile = artifactPDB
		if u.Payload.HasLinker() {
			u.SymbolsFile = artifactPDB
			u.ExtraFiles.Append(intermediatePDB)
			u.LinkerOptions.Append("/DEBUG", "/PDB:\""+MakeLocalFilename(artifactPDB)+"\"")

			if u.DebugFastLink.Get() {
				u.LinkerOptions.Append("/DEBUG:FASTLINK")
			}
		} else {
			u.SymbolsFile = intermediatePDB
		}

		u.AddCompilationFlag_NoPreprocessor("/Zi", "/Zf", "/FS", "/Fd\""+MakeLocalFilename(intermediatePDB)+"\"")

	case DEBUG_HOTRELOAD:
		editAndContinuePDB := u.OutputFile.ReplaceExt("-EditAndContinue.pdb")
		editAndContinuePDB.Dirname = u.IntermediateDir

		if u.Payload.HasLinker() {
			u.SymbolsFile = artifactPDB
			u.ExtraFiles.Append(editAndContinuePDB)
			u.LinkerOptions.Append("/DEBUG", "/EDITANDCONTINUE", "/PDB:\""+MakeLocalFilename(artifactPDB)+"\"")
			u.LinkerOptions.AppendUniq("/INCREMENTAL")

			if u.DebugFastLink.Get() {
				u.LinkerOptions.Append("/DEBUG:FASTLINK")
			}
		} else {
			u.SymbolsFile = editAndContinuePDB
		}

		u.AddCompilationFlag_NoPreprocessor("/ZI", "/Zf", "/FS", "/Fd\""+MakeLocalFilename(editAndContinuePDB)+"\"")

	default:
		UnexpectedValue(u.DebugSymbols)
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
func (msvc *MsvcCompiler) PrecompiledHeader(u *Unit) {
	switch u.PCH {
	case PCH_MONOLITHIC, PCH_SHARED:
		u.Defines.Append("BUILD_PCH=1")
		u.CompilerOptions.Append(
			"/FI\""+u.PrecompiledHeader.Basename+"\"",
			"/Yu\""+u.PrecompiledHeader.Basename+"\"",
			"/Fp\""+MakeLocalFilename(u.PrecompiledObject)+"\"")
		if u.PCH != PCH_SHARED {
			u.PrecompiledHeaderOptions.Append("/Yc\"" + u.PrecompiledHeader.Basename + "\"")
		}
	case PCH_DISABLED:
		u.Defines.Append("BUILD_PCH=0")
	default:
		UnexpectedValue(u.PCH)
	}
}
func (msvc *MsvcCompiler) Sanitizer(f *Facet, sanitizer SanitizerType) {
	switch sanitizer {
	case SANITIZER_NONE:
		return
	case SANITIZER_ADDRESS:
		// https://devblogs.microsoft.com/cppblog/addresssanitizer-asan-for-windows-with-msvc/
		f.Defines.Append("USE_PPE_SANITIZER=1")
		f.AddCompilationFlag_NoAnalysis("/fsanitize=address")
	default:
		UnexpectedValue(sanitizer)
	}
}

func (msvc *MsvcCompiler) ForceInclude(f *Facet, inc ...Filename) {
	for _, x := range inc {
		f.AddCompilationFlag_NoAnalysis("/FI\"" + x.Relative(UFS.Source) + "\"")
	}
}
func (msvc *MsvcCompiler) IncludePath(f *Facet, dirs ...Directory) {
	for _, x := range dirs {
		f.AddCompilationFlag_NoAnalysis("/I\"" + MakeLocalDirectory(x) + "\"")
	}
}
func (msvc *MsvcCompiler) ExternIncludePath(f *Facet, dirs ...Directory) {
	for _, x := range dirs {
		f.AddCompilationFlag_NoAnalysis("/external:I\"" + MakeLocalDirectory(x) + "\"")
	}
}
func (msvc *MsvcCompiler) SystemIncludePath(facet *Facet, dirs ...Directory) {
	msvc.ExternIncludePath(facet, dirs...)
}
func (msvc *MsvcCompiler) Library(f *Facet, lib ...Filename) {
	for _, x := range lib {
		libInc := "\"" + MakeLocalFilename(x) + "\""
		f.LibrarianOptions.Append(libInc)
		f.LinkerOptions.Append(libInc)
	}
}
func (msvc *MsvcCompiler) LibraryPath(f *Facet, dirs ...Directory) {
	for _, x := range dirs {
		libPath := "/LIBPATH:\"" + MakeLocalDirectory(x) + "\""
		f.LibrarianOptions.Append(libPath)
		f.LinkerOptions.Append(libPath)
	}
}
func (msvc *MsvcCompiler) SourceDependencies(obj *ActionRules) Action {
	actionOutput := obj.GetAction().Outputs[0]
	return NewMsvcSourceDependenciesAction(obj, actionOutput.ReplaceExt(msvc.Extname(PAYLOAD_DEPENDENCIES)))
}

func (msvc *MsvcCompiler) AddResources(compileEnv *CompileEnv, u *Unit, rc Filename) error {
	LogVeryVerbose("MSVC: add resource compiler custom unit to %v", u.Alias())

	resources := CustomUnit{
		Unit: Unit{
			Target:          u.Target,
			ModuleDir:       u.ModuleDir,
			GeneratedDir:    u.GeneratedDir,
			IntermediateDir: u.IntermediateDir,
			Payload:         u.Payload,
			Facet:           u.Facet,
			CompilerAlias:   msvc.ResourceCompiler.GetCompiler().CompilerAlias,
			CppRules: CppRules{
				PCH:   PCH_DISABLED,
				Unity: UNITY_DISABLED,
			},
		},
	}

	resources.Target.ModuleName += "-RC"
	resources.Source.SourceFiles.Append(rc)
	resources.AnalysisOptions.Clear()
	resources.CompilerOptions.Clear()
	resources.PreprocessorOptions.Clear()
	resources.LibrarianOptions.Clear()
	resources.LinkerOptions.Clear()

	if err := resources.Decorate(compileEnv, resources.GetCompiler()); err != nil {
		return err
	}
	resources.Append(resources.GetCompiler()) // compiler options need to be at the end of command-line

	u.CustomUnits.Append(resources)

	return nil
}

func (msvc *MsvcCompiler) Decorate(compileEnv *CompileEnv, u *Unit) error {
	windowsFlags := GetWindowsFlags()

	if u.LinkerVerbose.Get() {
		u.LinkerOptions.Append(
			"/VERBOSE",
			"/VERBOSE:LIB",
			"/VERBOSE:ICF",
			"/VERBOSE:REF",
			"/VERBOSE:INCR",
			"/VERBOSE:UNUSEDLIBS",
		)
	}

	// set architecture options
	switch compileEnv.GetPlatform().Arch {
	case ARCH_X86:
		u.AddCompilationFlag_NoAnalysis("/arch:AVX2")
		u.LibrarianOptions.Append("/MACHINE:x86")
		u.LinkerOptions.Append("/MACHINE:x86")
		u.LibraryPaths.Append(
			msvc.VSInstallPath.Folder("VC", "Tools", "MSVC", msvc.MinorVer, "lib", "x86"),
			msvc.VSInstallPath.Folder("VC", "Auxiliary", "VS", "lib", "x86"))

		if u.DebugSymbols != DEBUG_HOTRELOAD {
			u.LinkerOptions.Append("/SAFESEH")
		}

	case ARCH_X64:
		u.AddCompilationFlag_NoAnalysis("/arch:AVX2")
		u.LibrarianOptions.Append("/MACHINE:x64")
		u.LinkerOptions.Append("/MACHINE:x64")
		u.LibraryPaths.Append(
			msvc.VSInstallPath.Folder("VC", "Tools", "MSVC", msvc.MinorVer, "lib", "x64"),
			msvc.VSInstallPath.Folder("VC", "Auxiliary", "VS", "lib", "x64"))
	default:
		UnexpectedValue(compileEnv.GetPlatform().Arch)
	}

	// set compiler options from configuration
	switch compileEnv.GetConfig().ConfigType {
	case CONFIG_DEBUG:
		decorateMsvcConfig_Debug(u)
	case CONFIG_FASTDEBUG:
		decorateMsvcConfig_FastDebug(u)
	case CONFIG_DEVEL:
		decorateMsvcConfig_Devel(u)
	case CONFIG_TEST:
		decorateMsvcConfig_Test(u)
	case CONFIG_SHIPPING:
		decorateMsvcConfig_Shipping(u)
	default:
		UnexpectedValue(compileEnv.GetConfig().ConfigType)
	}

	// C++20 deprecates /Gm
	if u.CppStd >= CPPSTD_20 {
		// Command line warning D9035 : option 'Gm' has been deprecated and will be removed in a future release
		// Command line error D8016 : '/Gm' and '/std:c++20' command-line options are incompatible
		u.RemoveCompilationFlag("/Gm-", "/Gm")
	}

	// set default thread stack size
	stackSize := windowsFlags.StackSize
	if u.Sanitizer != SANITIZER_NONE {
		stackSize *= 2
		LogVeryVerbose("%v: doubling thread stack size due to msvc sanitizer (%d)", u, stackSize)
	}
	u.AddCompilationFlag(fmt.Sprintf("/F\"%d\"", stackSize))
	u.LinkerOptions.Append(fmt.Sprintf("/STACK:%d", stackSize))

	if windowsFlags.Analyze.Get() {
		LogVeryVerbose("%v: using msvc static analysis", u)

		u.AnalysisOptions.Append(
			"/analyze",
			"/analyze:external-", // disable analysis of external headers
			fmt.Sprint("/analyse:stacksize", stackSize),
			fmt.Sprintf("/analyze:plugin\"%v\"", msvc.ProductInstall.VcToolsHostPath().File("EspXEngine.dll")),
		)
		u.Defines.Append("ANALYZE")
	}

	// set dependent linker options

	if u.Sanitizer != SANITIZER_NONE {
		LogVeryVerbose("%v: using sanitizer %v", u, u.Sanitizer)
		u.Environment["ASAN_OPTIONS"] = []string{"windows_hook_rtl_allocators=true"}
		if u.CompilerOptions.Contains("/LTCG") {
			LogVeryVerbose("%v: disable LTCG due to %v", u, u.Sanitizer)
			u.LinkerOptions.Remove("/LTCG")
		}
	}

	if u.Deterministic.Get() {
		switch u.DebugSymbols {
		case DEBUG_SYMBOLS, DEBUG_EMBEDDED, DEBUG_DISABLED:
			// https://nikhilism.com/post/2020/windows-deterministic-builds/
			u.Incremental.Disable()
			pathMap := fmt.Sprintf("/pathmap:%v=.", UFS.Root)
			u.AddCompilationFlag("/Brepro", "/experimental:deterministic", pathMap, "/d1nodatetime")
			//u.AddCompilationFlag("/d1trimfile:"+UFS.Root.String()) // implied by /experimental:deterministic + /pathmap: ?
			u.PrecompiledHeaderOptions.Append("/wd5049") // Embedding a full path may result in machine-dependent output (always happen with PCH)
			u.LibrarianOptions.Append("/Brepro", "/experimental:deterministic")
			if !u.Incremental.Get() {
				u.LinkerOptions.Append("/Brepro", "/experimental:deterministic", pathMap, "/pdbaltpath:%_PDB%")
			}
		case DEBUG_HOTRELOAD:
			LogWarning("%v: deterministic build is not compatible with %v", u, u.DebugSymbols)
		default:
			UnexpectedValuePanic(u.DebugSymbols, u.DebugSymbols)
		}
	}

	if u.Incremental.Get() && u.Sanitizer == SANITIZER_NONE {
		LogVeryVerbose("%v: using msvc incremental linker", u)
		if u.LinkerOptions.Contains("/INCREMENTAL") {
			u.LinkerOptions.Remove("/LTCG")
		} else if u.LinkerOptions.Contains("/LTCG") {
			u.LinkerOptions.Remove("/LTCG")
			u.LinkerOptions.Append("/LTCG:INCREMENTAL")
			u.LinkerOptions.Remove("/OPT:NOREF")
		} else if !u.LinkerOptions.Contains("/LTCG:INCREMENTAL") {
			u.LinkerOptions.Append("/INCREMENTAL")
		} else {
			u.LinkerOptions.Remove("/OPT:NOREF")
		}
	} else if !u.LinkerOptions.Contains("/INCREMENTAL") {
		if u.Incremental.Get() && u.Sanitizer != SANITIZER_NONE {
			LogWarning("%v: incremental linker is not compatbile with %v", u, u.Sanitizer)
		}
		LogVeryVerbose("%v: using non-incremental msvc linker", u)
		u.LinkerOptions.Append("/INCREMENTAL:NO")
	}

	// eventually detects resource file to compile on Windows
	if u.Payload == PAYLOAD_EXECUTABLE || u.Payload == PAYLOAD_SHAREDLIB {
		resource_rc := u.ModuleDir.File("resource.rc")
		if resource_rc.Exists() {
			if err := msvc.AddResources(compileEnv, u, resource_rc); err != nil {
				return err
			}
		}
	}

	// enable perfSDK if necessary
	if windowsFlags.PerfSDK.Get() {
		LogVeryVerbose("%v: using Windows PerfSDK", u)
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

	// register extra files generated by the compiler
	switch u.Payload {
	case PAYLOAD_EXECUTABLE, PAYLOAD_SHAREDLIB:
		if u.Payload == PAYLOAD_SHAREDLIB {
			u.ExtraFiles.Append(u.OutputFile.ReplaceExt(".lib"))
			u.ExtraFiles.Append(u.OutputFile.ReplaceExt(".exp"))
		}
		if u.LinkerOptions.Contains("/INCREMENTAL") {
			u.ExtraFiles.Append(u.OutputFile.ReplaceExt(".ilk"))
		}

	case PAYLOAD_STATICLIB:
	case PAYLOAD_OBJECTLIST, PAYLOAD_HEADERS:
	default:
		UnexpectedValuePanic(u.Payload, u.Payload)
	}

	// don't forget the current windows SDK
	return msvc.WindowsSDK.Decorate(compileEnv, u)
}

/***************************************
 * Compiler options per configuration
 ***************************************/

func msvc_CXX_runtimeLibrary(u *Unit, staticCrt bool, debug bool) {
	if u.CompilerOptions.Any("/MD", "/MDd", "/MT", "/MTd") {
		// don't override user configuration
		return
	}

	var runtimeFlag string
	var suffix string
	if debug {
		suffix = "d"
	}

	if staticCrt {
		LogVeryVerbose("%v: using msvc static CRT libraries (debug=%v)", u, debug)
		runtimeFlag = "/MT"
		u.AddCompilationFlag(
			"LIBCMT"+suffix+".LIB",
			"libvcruntime"+suffix+".lib",
			"libucrt"+suffix+".lib")
	} else {
		LogVeryVerbose("%v: using msvc dynamic CRT libraries (debug=%v)", u, debug)
		runtimeFlag = "/MD"
		u.Defines.Append("_DLL")
	}

	u.AddCompilationFlag(runtimeFlag + suffix)
}
func msvc_CXX_linkTimeCodeGeneration(u *Unit, enabled bool) {
	if !u.LinkerOptions.Any("/LTCG", "/LTCG:OFF", "/LTCG:INCREMENTAL") {
		if enabled {
			LogVeryVerbose("%v: using msvc link time code generation", u)
			u.LinkerOptions.Append("/LTCG")
		} else {
			LogVeryVerbose("%v: disabling msvc link time code generation", u)
			u.LinkerOptions.Append("/LTCG:OFF")
		}
	}
}
func msvc_CXX_runtimeChecks(u *Unit, enabled bool, rtc1 bool) {
	if enabled {
		LogVeryVerbose("%v: using msvc runtime checks and control flow guard", u)
		// https://msdn.microsoft.com/fr-fr/library/jj161081(v=vs.140).aspx
		u.AddCompilationFlag("/GS", "/sdl")
		if rtc1 {
			LogVeryVerbose("%v: using msvc RTC1 checks", u)
			// https://msdn.microsoft.com/fr-fr/library/8wtf2dfz.aspx
			u.AddCompilationFlag("/RTC1")
		}
		u.LinkerOptions.Append("/GUARD:CF")
	} else {
		LogVeryVerbose("%v: disabling msvc runtime checks", u)
		u.AddCompilationFlag("/GS-", "/sdl-")
		u.LinkerOptions.Append("/GUARD:NO")
	}
}
func msvc_STL_debugHeap(u *Unit, enabled bool) {
	if !enabled {
		LogVeryVerbose("%v: disabling msvc debug heap", u)
		u.Defines.Append("_NO_DEBUG_HEAP=1")
	}
}
func msvc_STL_iteratorDebug(u *Unit, enabled bool) {
	if enabled {
		LogVeryVerbose("%v: enable msvc STL iterator debugging", u)
		u.Defines.Append(
			"_SECURE_SCL=1",             // https://msdn.microsoft.com/fr-fr/library/aa985896.aspx
			"_ITERATOR_DEBUG_LEVEL=2",   // https://msdn.microsoft.com/fr-fr/library/hh697468.aspx
			"_HAS_ITERATOR_DEBUGGING=1") // https://msdn.microsoft.com/fr-fr/library/aa985939.aspx")
	} else {
		LogVeryVerbose("%v: disable msvc STL iterator debugging", u)
		u.Defines.Append(
			"_SECURE_SCL=0",             // https://msdn.microsoft.com/fr-fr/library/aa985896.aspx
			"_ITERATOR_DEBUG_LEVEL=0",   // https://msdn.microsoft.com/fr-fr/library/hh697468.aspx
			"_HAS_ITERATOR_DEBUGGING=0") // https://msdn.microsoft.com/fr-fr/library/aa985939.aspx
	}
}

func decorateMsvcConfig_Debug(u *Unit) {
	u.AddCompilationFlag("/Od", "/Oy-", "/Gm-", "/Gw-")
	u.LinkerOptions.Append("/DYNAMICBASE:NO", "/HIGHENTROPYVA:NO", "/OPT:NOREF", "/OPT:NOICF")
	msvc_CXX_runtimeLibrary(u, GetWindowsFlags().StaticCRT.Get(), true)
	msvc_CXX_linkTimeCodeGeneration(u, u.LTO.Get())
	msvc_CXX_runtimeChecks(u, u.RuntimeChecks.Get(), true)
	msvc_STL_debugHeap(u, true)
	msvc_STL_iteratorDebug(u, true)
}
func decorateMsvcConfig_FastDebug(u *Unit) {
	u.AddCompilationFlag("/Ob1", "/Oy-", "/Gw-", "/Gm")
	u.LinkerOptions.Append("/DYNAMICBASE:NO", "/HIGHENTROPYVA:NO")
	msvc_CXX_runtimeLibrary(u, GetWindowsFlags().StaticCRT.Get(), true)
	msvc_CXX_linkTimeCodeGeneration(u, u.LTO.Get())
	msvc_CXX_runtimeChecks(u, u.RuntimeChecks.Get(), false)
	msvc_STL_debugHeap(u, true)
	msvc_STL_iteratorDebug(u, true)
}
func decorateMsvcConfig_Devel(u *Unit) {
	u.AddCompilationFlag("/O2", "/Oy-", "/GA", "/Gm-", "/Zo", "/GL")
	u.LinkerOptions.Append("/DYNAMICBASE:NO", "/HIGHENTROPYVA:NO", "/OPT:NOICF")
	msvc_CXX_runtimeLibrary(u, GetWindowsFlags().StaticCRT.Get(), false)
	msvc_CXX_linkTimeCodeGeneration(u, u.LTO.Get())
	msvc_CXX_runtimeChecks(u, u.RuntimeChecks.Get(), false)
	msvc_STL_debugHeap(u, false)
	msvc_STL_iteratorDebug(u, false)
}
func decorateMsvcConfig_Test(u *Unit) {
	u.AddCompilationFlag("/O2", "/Ob3", "/Gw", "/Gm-", "/Gy", "/GL", "/GA", "/Zo")
	u.LinkerOptions.Append("/DYNAMICBASE", "/HIGHENTROPYVA", "/PROFILE", "/OPT:REF")
	msvc_CXX_runtimeLibrary(u, GetWindowsFlags().StaticCRT.Get(), false)
	msvc_CXX_linkTimeCodeGeneration(u, u.LTO.Get())
	msvc_CXX_runtimeChecks(u, false, false)
	msvc_STL_debugHeap(u, false)
	msvc_STL_iteratorDebug(u, false)
}
func decorateMsvcConfig_Shipping(u *Unit) {
	u.AddCompilationFlag("/O2", "/Ob3", "/Gw", "/Gm-", "/Gy", "/GL", "/GA", "/Zo-")
	u.LinkerOptions.Append("/DYNAMICBASE", "/HIGHENTROPYVA", "/OPT:REF", "/OPT:ICF=3")
	msvc_CXX_runtimeLibrary(u, GetWindowsFlags().StaticCRT.Get(), false)
	msvc_CXX_linkTimeCodeGeneration(u, u.LTO.Get())
	msvc_CXX_runtimeChecks(u, false, false)
	msvc_STL_debugHeap(u, false)
	msvc_STL_iteratorDebug(u, false)
}

/***************************************
 * Compiler detection
 ***************************************/

var MSVC_VSWHERE_EXE = UFS.Build.Folder("hal", "windows", "bin").File("vswhere.exe")

type VsWhereCatalog struct {
	ProductDisplayVersion string
	ProductLineVersion    string
}

func (x *VsWhereCatalog) Serialize(ar Archive) {
	ar.String(&x.ProductDisplayVersion)
	ar.String(&x.ProductLineVersion)
}

type VsWhereEntry struct {
	InstallationName string
	InstallationPath string
	Catalog          VsWhereCatalog
}

func (x *VsWhereEntry) Serialize(ar Archive) {
	ar.String(&x.InstallationName)
	ar.String(&x.InstallationPath)
	ar.Serializable(&x.Catalog)
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

	Cl_exe   Filename
	Lib_exe  Filename
	Link_exe Filename
}

func (x *MsvcProductInstall) VcToolsHostPath() Directory {
	return x.VcToolsPath.Folder("bin", "Host"+x.HostArch, x.Arch)
}

func (x *MsvcProductInstall) Alias() BuildAlias {
	variant := "Stable"
	if x.Insider {
		variant = "Insider"
	}
	return MakeBuildAlias("HAL", "Windows", "MSVC", x.WantedVer.String(), x.Arch, variant)
}
func (x *MsvcProductInstall) Serialize(ar Archive) {
	ar.String(&x.Arch)
	ar.Serializable(&x.WantedVer)
	ar.Bool(&x.Insider)

	ar.Serializable(&x.ActualVer)
	ar.String(&x.HostArch)
	ar.Serializable(&x.Selected)
	ar.Serializable(&x.VsInstallPath)
	ar.Serializable(&x.VcToolsPath)
	ar.Serializable(&x.VcToolsFileSet)

	ar.Serializable(&x.Cl_exe)
	ar.Serializable(&x.Lib_exe)
	ar.Serializable(&x.Link_exe)
}
func (x *MsvcProductInstall) Build(bc BuildContext) error {
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

	cmd := exec.Command(MSVC_VSWHERE_EXE.String(), args...)

	var entries []VsWhereEntry
	if outp, err := cmd.Output(); err != nil {
		return err
	} else if len(outp) > 0 {
		if err := json.Unmarshal(outp, &entries); err != nil {
			return err
		}
	}

	if len(entries) == 0 {
		return fmt.Errorf("msvc: vswhere did not find any compiler")
	}

	x.Selected = entries[0]
	x.VsInstallPath = MakeDirectory(x.Selected.InstallationPath)
	if _, err := x.VsInstallPath.Info(); err != nil {
		return err
	}

	if err := x.ActualVer.Set(x.Selected.Catalog.ProductLineVersion); err != nil {
		return err
	}

	var vcToolsVersion string
	vcToolsVersionFile := x.VsInstallPath.Folder("VC", "Auxiliary", "Build").File("Microsoft.VCToolsVersion.default.txt")

	if data, err := os.ReadFile(vcToolsVersionFile.String()); err == nil {
		vcToolsVersion = strings.TrimSpace(string(data))
	} else {
		return err
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
		return err
	}

	x.Cl_exe = vcToolsHostPath.File("cl.exe")
	x.Lib_exe = vcToolsHostPath.File("lib.exe")
	x.Link_exe = vcToolsHostPath.File("link.exe")
	if err := bc.NeedFile(vcToolsVersionFile, x.Cl_exe, x.Lib_exe, x.Link_exe); err != nil {
		return err
	}
	if err := bc.NeedFile(x.VcToolsFileSet...); err != nil {
		return err
	}
	if err := bc.NeedDirectory(x.VcToolsPath, x.VsInstallPath); err != nil {
		return err
	}

	return nil
}

func (msvc *MsvcCompiler) Build(bc BuildContext) (err error) {
	compileFlags := GetCompileFlags()
	windowsFlags := GetWindowsFlags()

	msvc.ProductInstall, err = GetMsvcProductInstall(MsvcProductVer{
		Arch:    msvc.Arch,
		MscVer:  windowsFlags.MscVer,
		Insider: windowsFlags.Insider,
	}).Need(bc)
	if err != nil {
		return
	}

	msvc.ResourceCompiler, err = GetWindowsResourceCompiler().Need(bc)
	if err != nil {
		return
	}

	windowsSDKInstall := GetWindowsSDKInstall(bc, windowsFlags.WindowsSDK)

	msc_ver := msvc.ProductInstall.ActualVer

	msvc.MSC_VER = msc_ver
	msvc.MinorVer = msvc.ProductInstall.VcToolsPath.Basename()
	msvc.Host = msvc.ProductInstall.HostArch
	msvc.Target = msvc.Arch.String()
	msvc.PlatformToolset = fmt.Sprintf("%s%s%s", msvc.MinorVer[0:1], msvc.MinorVer[1:2], msvc.MinorVer[3:4])
	msvc.VSInstallName = msvc.ProductInstall.Selected.InstallationName
	msvc.VSInstallPath = msvc.ProductInstall.VsInstallPath
	msvc.VCToolsPath = msvc.ProductInstall.VcToolsPath
	msvc.WindowsSDK = &windowsSDKInstall.WindowsSDK

	msvc.CompilerRules.CppStd = getCppStdFromMsc(msc_ver)
	msvc.CompilerRules.Features = MakeEnumSet(
		COMPILER_ALLOW_CACHING,
		COMPILER_ALLOW_DISTRIBUTION,
		COMPILER_ALLOW_RESPONSEFILE,
		COMPILER_ALLOW_SOURCEMAPPING)

	msvc.CompilerRules.Executable = msvc.ProductInstall.Cl_exe
	msvc.CompilerRules.Librarian = msvc.ProductInstall.Lib_exe
	msvc.CompilerRules.Linker = msvc.ProductInstall.Link_exe

	tmpDir := UFS.Transient.Folder("TMP")
	if err := CreateDirectory(bc, tmpDir); err != nil {
		return err
	}

	msvc.CompilerRules.Environment = ProcessEnvironment{
		"PATH": []string{
			msvc.ProductInstall.VcToolsHostPath().String(),
			msvc.WindowsSDK.ResourceCompiler.Dirname.String(),
			"%PATH%"},
		"SystemRoot": []string{os.Getenv("SystemRoot")},
		"TMP":        []string{tmpDir.String()},
	}
	msvc.CompilerRules.ExtraFiles = msvc.ProductInstall.VcToolsFileSet

	msvc.CompilerRules.Facet = NewFacet()
	facet := &msvc.CompilerRules.Facet

	facet.Append(msvc.WindowsSDK)

	facet.Defines.Append(
		"CPP_VISUALSTUDIO",
		"_ENABLE_EXTENDED_ALIGNED_STORAGE", // https://devblogs.microsoft.com/cppblog/stl-features-and-fixes-in-vs-2017-15-8/
	)

	facet.Exports.Add("VisualStudio/Path", msvc.VSInstallPath.String())
	facet.Exports.Add("VisualStudio/PlatformToolset", msvc.PlatformToolset)
	facet.Exports.Add("VisualStudio/Tools", msvc.VCToolsPath.String())
	facet.Exports.Add("VisualStudio/Version", msvc.MinorVer)

	facet.SystemIncludePaths.Append(
		msvc.VSInstallPath.Folder("VC", "Auxiliary", "VS", "include"),
		msvc.VSInstallPath.Folder("VC", "Tools", "MSVC", msvc.MinorVer, "crt", "src"),
		msvc.VSInstallPath.Folder("VC", "Tools", "MSVC", msvc.MinorVer, "include"))

	facet.AddCompilationFlag_NoAnalysis(
		"/nologo",   // no copyright when compiling
		"/c \"%1\"", // input file injection
	)

	facet.CompilerOptions.Append("/Fo\"%2\"")
	facet.PrecompiledHeaderOptions.Append("/Fp\"%2\"", "/Fo\"%3\"")
	facet.PreprocessorOptions.Append("/Fo\"%2\"")

	facet.AddCompilationFlag(
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
		"/Zc:sizedDealloc",     // https://learn.microsoft.com/en-us/cpp/build/reference/zc-sizeddealloc-enable-global-sized-dealloc-functions?view=msvc-170
		"/Zc:__cplusplus",      // https://docs.microsoft.com/en-us/cpp/build/reference/zc-cplusplus?view=msvc-170
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
		"Advapi32.lib",
		"Shlwapi.lib",
		"Version.lib",
		"/nologo",            // no copyright when compiling
		"/TLBID:1",           // https://msdn.microsoft.com/fr-fr/library/b1kw34cb.aspx
		"/IGNORE:4001",       // https://msdn.microsoft.com/en-us/library/aa234697(v=vs.60).aspx
		"/IGNORE:4099",       // don't have PDB for some externals
		"/NXCOMPAT:NO",       // disable Data Execution Prevention (DEP)
		"/LARGEADDRESSAWARE", // indicate support for VM > 2Gb (if 3Gb flag is toggled)
		"/SUBSYSTEM:WINDOWS", // ~Windows~ application type (vs Console)
		"/fastfail",          // better error reporting
	)

	// strict vs permissive
	if windowsFlags.Permissive.Get() {
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

	if compileFlags.Benchmark.Get() {
		LogVeryVerbose("MSVC: will dump compilation timings")
		facet.CompilerOptions.Append("/d2cgsummary", "/Bt+")
		facet.LinkerOptions.Append("/d2:-cgsummary")
	}

	if msc_ver >= MSC_VER_2019 {
		if windowsFlags.JustMyCode.Get() {
			LogVeryVerbose("MSVC: using just-my-code")
			facet.AddCompilationFlag_NoAnalysis("/JMC")
		} else {
			facet.AddCompilationFlag_NoAnalysis("/JMC-")
		}
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

	return nil
}

type MsvcProductVer struct {
	Arch    ArchType
	MscVer  MsvcVersion
	Insider BoolVar
}

func GetMsvcProductInstall(prms MsvcProductVer) BuildFactoryTyped[*MsvcProductInstall] {
	return func(bi BuildInitializer) (*MsvcProductInstall, error) {
		if err := bi.NeedFile(MSVC_VSWHERE_EXE); err != nil {
			return nil, err
		}

		if prms.MscVer == MSC_VER_LATEST {
			prms.MscVer = msc_ver_any
		}

		return &MsvcProductInstall{
			WantedVer: prms.MscVer,
			Arch:      prms.Arch.String(),
			Insider:   prms.Insider.Get(),
		}, nil
	}
}

func GetMsvcCompiler(arch ArchType) BuildFactoryTyped[Compiler] {
	return func(bi BuildInitializer) (Compiler, error) {
		if err := bi.NeedFactories(
			GetBuildableFlags(GetCompileFlags()),
			GetBuildableFlags(GetWindowsFlags())); err != nil {
			return nil, err
		}

		return &MsvcCompiler{
			Arch:          arch,
			CompilerRules: NewCompilerRules(NewCompilerAlias("msvc", "VisualStudio", arch.String())),
		}, nil
	}
}
