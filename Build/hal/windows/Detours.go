package windows

import (
	//lint:ignore ST1001 ignore dot imports warning
	. "build/utils"
	"crypto/rand"
	"encoding/hex"
)

var LogDetours = NewLogCategory("Detours")

const DETOURS_IGNORED_APPLICATIONS = `cpptools.exe,cpptools-srv.exe,mspdbsrv.exe,vcpkgsrv.exe,vctip.exe`
const DETOURS_IOWRAPPER_EXE = `Tools-IOWrapper-Win64-Devel.exe`

var getDetoursIOWrapperExecutable = Memoize(func() Filename {
	return UFS.Build.AbsoluteFile(`hal`, `windows`, `bin`, DETOURS_IOWRAPPER_EXE)
})

func SetupWin32IODetouring() {
	if exe := getDetoursIOWrapperExecutable(); exe.Exists() {
		OnRunCommandWithDetours = RunProcessWithDetoursWin32 // Comment to disable detouring %_NOCOMMIT%
	} else {
		LogWarning(LogDetours, "disable Win32 IO detouring: could not find %q", exe)
	}
}

func getDetoursUniqueDependencyOutputFile() Filename {
	randBytes := [16]byte{}
	rand.Read(randBytes[:])

	dependencyOutputFile := UFS.Transient.Folder("Dependencies").File(hex.EncodeToString(randBytes[:]))
	UFS.Mkdir(dependencyOutputFile.Dirname)
	return dependencyOutputFile
}

func RunProcessWithDetoursWin32(executable Filename, arguments StringSet, options ProcessOptions) error {
	randBytes := [16]byte{}
	rand.Read(randBytes[:])

	dependencyOutputFile := getDetoursUniqueDependencyOutputFile()

	if err := RunProcess_Vanilla(
		getDetoursIOWrapperExecutable(),
		append([]string{dependencyOutputFile.String(), DETOURS_IGNORED_APPLICATIONS, executable.String()}, arguments...),
		options); err != nil {
		return err
	}

	defer UFS.Remove(dependencyOutputFile)

	return UFS.ReadLines(dependencyOutputFile, func(line string) error {
		var far FileAccessRecord
		far.Path = MakeFilename(line[1:])
		far.Access = FILEACCESS_NONE

		mode := line[0] - '0'
		if (mode & 1) == 1 {
			far.Access.Append(FILEACCESS_READ)
		}
		if (mode & 2) == 2 {
			far.Access.Append(FILEACCESS_WRITE)
		}
		if (mode & 4) == 4 {
			far.Access.Append(FILEACCESS_EXECUTE)
		}

		return options.OnFileAccess.Invoke(far)
	})
}
