// + build linux

package hal

import (
	"build/hal/generic"
	"build/hal/linux"
	utils "build/utils"
	"os"
	"strings"
	"syscall"

	"golang.org/x/sys/unix"
)

func InitHAL() {
	var uname syscall.Utsname
	if err := syscall.Uname(&uname); err != nil {
		utils.LogFatal("Uname: %s", err)
	}
	utils.SetCurrentHost(&utils.HostPlatform{
		Id:   utils.HOST_LINUX,
		Name: arrayToString(uname.Version),
	})
	utils.FBUILD_BIN = utils.UFS.Build.Folder("hal", "linux", "bin").File("fbuild")
	utils.SetEnableInteractiveShell(isInteractiveShell())
	generic.InitGeneric()
	linux.InitLinux()
}

func isTty() bool {
	// https://stackoverflow.com/questions/68889637/is-it-possible-to-detect-if-a-writer-is-tty-or-not
	_, err := unix.IoctlGetWinsize(int(os.Stdout.Fd()), unix.TIOCGWINSZ)
	return err != nil
}

func isInteractiveShell() bool {
	// if !isTty() {
	// 	return false
	// }
	term := os.Getenv("TERM")
	switch term {
	case "xterm", "alacritty":
		return true
	default:
		return strings.HasPrefix(term, "xterm-")
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
