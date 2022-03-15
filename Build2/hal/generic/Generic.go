package generic

import "encoding/gob"

func InitGeneric() {
	gob.Register(GlslangGeneratedHeader{})
	gob.Register(SpirvToolsGeneratedHeader{})
	gob.Register(&VulkanHeadersT{})
	gob.Register(&VulkanBindingsT{})
	gob.Register(&VulkanInterfaceT{})
	gob.Register(VulkanGeneratedHeader{})
	gob.Register(VulkanGeneratedSource{})
}
