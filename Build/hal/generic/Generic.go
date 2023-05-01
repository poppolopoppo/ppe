package generic

import "build/utils"

func InitGeneric() {
	utils.RegisterSerializable(&GnuSourceDependenciesAction{})

	utils.RegisterSerializable(&GlslangHeaderGenerator{})
	utils.RegisterSerializable(&GlslangGeneratedHeader{})

	utils.RegisterSerializable(&SpirvToolsHeaderGenerator{})
	utils.RegisterSerializable(&SpirvToolsGeneratedHeader{})

	utils.RegisterSerializable(&VulkanHeaderGenerator{})
	utils.RegisterSerializable(&VulkanGeneratedHeader{})

	utils.RegisterSerializable(&VulkanSourceGenerator{})
	utils.RegisterSerializable(&VulkanGeneratedSource{})

	utils.RegisterSerializable(&VulkanHeaders{})
	utils.RegisterSerializable(&VulkanBindings{})
	utils.RegisterSerializable(&VulkanInterface{})

	utils.RegisterSerializable(&VkFunctionPointer{})
}
