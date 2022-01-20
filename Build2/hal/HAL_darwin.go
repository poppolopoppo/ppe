// + build darwin

package hal

import (
	utils "build/utils"
)

func InitHAL(env *utils.CommandEnv) {
	utils.SetCurrentHost(&utils.HostPlatform{
		Id:   HOST_DARWIN,
		Name: "TODO",
	})
	utils.FBUILD_BIN = utils.UFS.Build.Folder("hal", "darwin", "bin").File("fbuild")
}
