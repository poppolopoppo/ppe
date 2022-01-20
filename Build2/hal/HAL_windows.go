// + build windows

package hal

import (
	"build/hal/generic"
	windows "build/hal/windows"
	utils "build/utils"
	"fmt"
	"os"
	"syscall"
)

func osVersion() string {
	v, err := syscall.GetVersion()
	if err != nil {
		return "0.0"
	}
	major := uint8(v)
	minor := uint8(v >> 8)
	build := uint16(v >> 16)
	return fmt.Sprintf("%d.%d build %d", major, minor, build)
}

func setConsoleMode() bool {
	stdout := syscall.Handle(os.Stdout.Fd())

	var originalMode uint32
	syscall.GetConsoleMode(stdout, &originalMode)
	originalMode |= 0x0004

	getConsoleMode := syscall.MustLoadDLL("kernel32").MustFindProc("SetConsoleMode")
	ret, _, err := getConsoleMode.Call(uintptr(stdout), uintptr(originalMode))

	if ret == 1 {
		return true
	}

	utils.LogVerbose("failed to set console mode: %v", err)
	return false
}

func InitHAL() {
	utils.SetCurrentHost(&utils.HostPlatform{
		Id:   utils.HOST_WINDOWS,
		Name: "Windows " + osVersion(),
	})
	utils.FBUILD_BIN = utils.UFS.Build.Folder("hal", "windows", "bin").File("FBuild.exe")
	utils.SetEnableInteractiveShell(setConsoleMode())
	generic.InitGeneric()
	windows.InitWindows()
}
