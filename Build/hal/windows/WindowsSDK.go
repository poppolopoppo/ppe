package windows

import (
	"build/compile"
	"build/utils"
	"bytes"
	"regexp"
	"sort"
)

type WindowsSDK struct {
	Name             string
	RootDir          utils.Directory
	Version          string
	ResourceCompiler utils.Filename
	compile.Facet
}

func newWindowsSDK(rootDir utils.Directory, version string) (result *WindowsSDK) {
	result = &WindowsSDK{
		Name:             "WindowsSDK_" + version,
		RootDir:          rootDir,
		Version:          version,
		ResourceCompiler: rootDir.Folder("Bin", version, "x64").File("RC.exe"),
		Facet:            compile.NewFacet(),
	}
	result.Facet.SystemIncludePaths.Append(
		rootDir.Folder("Include", version, "ucrt"),
		rootDir.Folder("Include", version, "um"),
		rootDir.Folder("Include", version, "shared"),
	)
	result.Facet.Defines.Append(
		"STRICT",                   // https://msdn.microsoft.com/en-us/library/windows/desktop/aa383681(v=vs.85).aspx
		"NOMINMAX",                 // https://support.microsoft.com/en-us/kb/143208
		"VC_EXTRALEAN",             // https://support.microsoft.com/en-us/kb/166474
		"WIN32_LEAN_AND_MEAN",      // https://support.microsoft.com/en-us/kb/166474
		"_NO_W32_PSEUDO_MODIFIERS", // Prevent windows from #defining IN or OUT (undocumented)
		"DBGHELP_TRANSLATE_TCHAR",  // https://msdn.microsoft.com/en-us/library/windows/desktop/ms679294(v=vs.85).aspx
		"_UNICODE",                 // https://msdn.microsoft.com/fr-fr/library/dybsewaf.aspx
		"UNICODE",                  // defaults to UTF-8
		"_HAS_EXCEPTIONS=0",        // Disable STL exceptions
		"OEMRESOURCE",              // https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-setsystemcursor
	)
	return result
}

func (sdk *WindowsSDK) GetFacet() *compile.Facet {
	return &sdk.Facet
}
func (sdk *WindowsSDK) GetDigestable(o *bytes.Buffer) {
	o.WriteString(sdk.Name)
	sdk.RootDir.GetDigestable(o)
	o.WriteString(sdk.Version)
	sdk.ResourceCompiler.GetDigestable(o)
	sdk.Facet.GetDigestable(o)
}
func (sdk *WindowsSDK) Decorate(compileEnv *compile.CompileEnv, u *compile.Unit) {
	switch compileEnv.GetPlatform().Arch {
	case compile.ARCH_X64:
		u.LibraryPaths.Append(
			sdk.RootDir.Folder("Lib", sdk.Version, "ucrt", "x64"),
			sdk.RootDir.Folder("Lib", sdk.Version, "um", "x64"),
		)
	case compile.ARCH_X86:
		u.LibraryPaths.Append(
			sdk.RootDir.Folder("Lib", sdk.Version, "ucrt", "x86"),
			sdk.RootDir.Folder("Lib", sdk.Version, "um", "x86"),
		)
	default:
		utils.UnexpectedValue(compileEnv.GetPlatform().Arch)
	}
}

type WindowsSDKBuilder struct {
	MajorVer   string
	SearchDir  utils.Directory
	SearchGlob string
	*WindowsSDK
}

func (x *WindowsSDKBuilder) Alias() utils.BuildAlias {
	return utils.MakeBuildAlias("HAL", "WindowsSDK_"+x.MajorVer)
}
func (x *WindowsSDKBuilder) Build(bc utils.BuildContext) (utils.BuildStamp, error) {
	var dirs utils.DirSet
	var err error
	if x.MajorVer != "User" {
		err = x.SearchDir.MatchDirectories(func(d utils.Directory) error {
			dirs.Append(d)
			return nil
		}, regexp.MustCompile(x.SearchGlob))
	} else {
		windowsFlags := WindowsFlags.Need(utils.CommandEnv.Flags)
		bc.DependsOn(windowsFlags)
		dirs.Append(windowsFlags.WindowsSDK)
		_, err = windowsFlags.WindowsSDK.Info()
	}
	if err == nil && len(dirs) > 0 {
		sort.Sort(dirs)
		lib := dirs[len(dirs)-1]
		bc.NeedFolder(lib)
		utils.LogDebug("found WindowsSDK@%v in '%v'", x.MajorVer, lib)
		ver := lib[len(lib)-1]
		x.WindowsSDK = newWindowsSDK(lib.Parent().Parent(), ver)
		bc.NeedFile(x.WindowsSDK.ResourceCompiler)
		return utils.MakeBuildStamp(x)
	} else {
		return utils.BuildStamp{}, err
	}
}
func (x *WindowsSDKBuilder) GetDigestable(o *bytes.Buffer) {
	o.WriteString(x.MajorVer)
	x.SearchDir.GetDigestable(o)
	o.WriteString(x.SearchGlob)
	x.WindowsSDK.GetDigestable(o)
}

var windowsSDK_10 = utils.MakeBuildable(func(bi utils.BuildInit) (result *WindowsSDKBuilder) {
	result = &WindowsSDKBuilder{
		MajorVer:   "10",
		SearchDir:  utils.MakeDirectory("C:/Program Files (x86)/Windows Kits/10/Lib"),
		SearchGlob: `10\..*`,
	}
	bi.NeedFolder(result.SearchDir)
	return result
})
var windowsSDK_8_1 = utils.MakeBuildable(func(bi utils.BuildInit) (result *WindowsSDKBuilder) {
	result = &WindowsSDKBuilder{
		MajorVer:   "8.1",
		SearchDir:  utils.MakeDirectory("C:/Program Files (x86)/Windows Kits/8.1/Lib"),
		SearchGlob: `8\..*`,
	}
	bi.NeedFolder(result.SearchDir)
	return result
})
var windowsSDK_User = utils.MakeBuildable(func(_ utils.BuildInit) (result *WindowsSDKBuilder) {
	result = &WindowsSDKBuilder{
		MajorVer: "User",
	}
	return result
})

var GetWindowsSDKInstall = utils.MemoizeArg(func(overrideDir utils.Directory) *WindowsSDKBuilder {
	bg := utils.CommandEnv.BuildGraph()
	if len(overrideDir) > 0 {
		utils.LogVeryVerbose("using user override '%v' for Windows SDK", overrideDir)
		if winUser, err := windowsSDK_User.Build(bg); err == nil {
			return winUser
		} else {
			panic(err)
		}
	}
	if win10, err := windowsSDK_10.Build(bg); err == nil {
		utils.LogVeryVerbose("using Windows SDK 10")
		return win10
	}
	if win81, err := windowsSDK_8_1.Build(bg); err == nil {
		utils.LogVeryVerbose("using Windows SDK 8.1")
		return win81
	}
	utils.UnreachableCode()
	return nil
})