// + build darwin

package hal

import (
	"build/hal/generic"
	utils "build/utils"
)

func InitHAL(env *utils.CommandEnv) {
	utils.SetCurrentHost(&utils.HostPlatform{
		Id:   HOST_DARWIN,
		Name: "TODO",
	})
	utils.FBUILD_BIN = utils.UFS.Build.Folder("hal", "darwin", "bin").File("fbuild")
	generic.InitGeneric()
}
