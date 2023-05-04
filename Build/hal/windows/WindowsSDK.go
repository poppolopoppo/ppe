package windows

import (
	"build/compile"
	"build/utils"
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

func newWindowsSDK(rootDir utils.Directory, version string) (result WindowsSDK) {
	result = WindowsSDK{
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
func (sdk *WindowsSDK) Serialize(ar utils.Archive) {
	ar.String(&sdk.Name)
	ar.Serializable(&sdk.RootDir)
	ar.String(&sdk.Version)
	ar.Serializable(&sdk.ResourceCompiler)
	ar.Serializable(&sdk.Facet)
}
func (sdk *WindowsSDK) Decorate(compileEnv *compile.CompileEnv, u *compile.Unit) error {
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
	return nil
}

type WindowsSDKBuilder struct {
	MajorVer   string
	SearchDir  utils.Directory
	SearchGlob string
	WindowsSDK
}

func (x *WindowsSDKBuilder) Alias() utils.BuildAlias {
	return utils.MakeBuildAlias("HAL", "Windows", "SDK", x.MajorVer)
}
func (x *WindowsSDKBuilder) Build(bc utils.BuildContext) error {
	var dirs utils.DirSet
	var err error
	if x.MajorVer != "User" {
		err = x.SearchDir.MatchDirectories(func(d utils.Directory) error {
			dirs.Append(d)
			return nil
		}, regexp.MustCompile(x.SearchGlob))
	} else {
		windowsFlags := GetWindowsFlags()
		if _, err = utils.GetBuildableFlags(windowsFlags).Need(bc); err != nil {
			return err
		}

		dirs.Append(windowsFlags.WindowsSDK)
		_, err = windowsFlags.WindowsSDK.Info()
	}
	if err == nil && len(dirs) > 0 {
		sort.Sort(dirs)
		lib := dirs[len(dirs)-1]
		if err = bc.NeedDirectory(lib); err != nil {
			return err
		}

		utils.LogDebug("found WindowsSDK@%v in '%v'", x.MajorVer, lib)

		libParent, ver := lib.Split()
		x.WindowsSDK = newWindowsSDK(libParent.Parent(), ver)
		err = bc.NeedFile(x.WindowsSDK.ResourceCompiler)
	}
	return err
}
func (x *WindowsSDKBuilder) Serialize(ar utils.Archive) {
	ar.String(&x.MajorVer)
	ar.Serializable(&x.SearchDir)
	ar.String(&x.SearchGlob)
	ar.Serializable(&x.WindowsSDK)
}

func getWindowsSDK_10() utils.BuildFactoryTyped[*WindowsSDKBuilder] {
	return func(bi utils.BuildInitializer) (*WindowsSDKBuilder, error) {
		searchDir := utils.MakeDirectory("C:/Program Files (x86)/Windows Kits/10/Lib")
		if err := bi.NeedDirectory(searchDir); err != nil {
			return nil, err
		}
		return &WindowsSDKBuilder{
			MajorVer:   "10",
			SearchDir:  searchDir,
			SearchGlob: `10\..*`,
		}, nil
	}
}

func getWindowsSDK_8_1() utils.BuildFactoryTyped[*WindowsSDKBuilder] {
	return func(bi utils.BuildInitializer) (*WindowsSDKBuilder, error) {
		searchDir := utils.MakeDirectory("C:/Program Files (x86)/Windows Kits/8.1/Lib")
		if err := bi.NeedDirectory(searchDir); err != nil {
			return nil, err
		}
		return &WindowsSDKBuilder{
			MajorVer:   "8.1",
			SearchDir:  searchDir,
			SearchGlob: `8\..*`,
		}, nil
	}
}

func getWindowsSDK_User(overrideDir utils.Directory) utils.BuildFactoryTyped[*WindowsSDKBuilder] {
	return func(bi utils.BuildInitializer) (*WindowsSDKBuilder, error) {
		if err := bi.NeedDirectory(overrideDir); err != nil {
			return nil, err
		}
		return &WindowsSDKBuilder{
			MajorVer:  "User",
			SearchDir: overrideDir,
		}, nil
	}
}

func GetWindowsSDKInstall(bi utils.BuildInitializer, overrideDir utils.Directory) *WindowsSDKBuilder {
	if len(overrideDir.Path) > 0 {
		utils.LogPanicIfFailed(bi.NeedDirectory(overrideDir))

		utils.LogVeryVerbose("using user override '%v' for Windows SDK", overrideDir)
		return getWindowsSDK_User(overrideDir).SafeNeed(bi)
	}

	if win10, err := getWindowsSDK_10().Need(bi); err == nil {
		utils.LogVeryVerbose("using Windows SDK 10")
		return win10
	}

	if win81, err := getWindowsSDK_8_1().Need(bi); err == nil {
		utils.LogVeryVerbose("using Windows SDK 8.1")
		return win81
	}

	utils.UnreachableCode()
	return nil
}
