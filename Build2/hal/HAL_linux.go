// + build linux

package hal

import (
	"build/hal/generic"
	"build/hal/linux"
	utils "build/utils"
	"os"
	"strings"
	"syscall"
)

var FBUILD_BIN = utils.UFS.Build.Folder("hal", "linux", "bin").File("fbuild")

func InitHAL() {
	var uname syscall.Utsname
	if err := syscall.Uname(&uname); err != nil {
		utils.LogFatal("Uname: %s", err)
	}
	utils.SetCurrentHost(&utils.HostPlatform{
		Id:   utils.HOST_LINUX,
		Name: arrayToString(uname.Version),
	})
	utils.SetEnableInteractiveShell(isInteractiveShell())
	generic.InitGeneric()
	linux.InitLinux()
}

func isInteractiveShell() bool {
	switch os.Getenv("TERM") {
	case "xterm", "xterm-256", "xterm-256color", "alacritty":
		return true
	default:
		return false
	}
}

func arrayToString(x [65]int8) string {
	var buf [65]byte
	for i, b := range x {
		buf[i] = byte(b)
	}
	str := string(buf[:])
	if i := strings.Index(str, "\x00"); i != -1 {
		str = str[:i]
	}
	return str
}
