// + build linux

package hal

import (
	utils "build/utils"
	"strings"
	"syscall"
)

var currentPlatform *PlatformType

var FBUILD_BIN = utils.UFS.Build.Folder("hal", "linux", "bin").File("fbuild")

func InitHAL(env *utils.CommandEnv) {
	var uname syscall.Utsname
	if err := syscall.Uname(&uname); err != nil {
		utis.LogFatal("Uname: %s", err)
	}
	utils.SetCurrentHost(&utils.HostPlatform{
		Id:   HOST_LINUX,
		Name: arrayToString(uname.Version),
	})
	env.HAL = currentPlatform.Id
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
