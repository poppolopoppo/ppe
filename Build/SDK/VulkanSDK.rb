# frozen_string_literal: true

require_once '../Core/Facet.rb'
require_once '../Utils/Prerequisite.rb'

module Build

    import_envvar(:VK_SDK_PATH).validate_DirExist!
    import_envvar(:VULKAN_SDK).validate_DirExist!

    const_memoize(self, :VulkanSDK_Path) do
        sdkPath = Build.VULKAN_SDK || Build.VK_SDK_PATH
        sdkVer = File.basename(sdkPath)
        Log.log 'Vulkan: found SDK v%s in "%s"', sdkVer, sdkPath
        File.expand_path(sdkPath)
    end

    class VulkanDecorator < Decorator
        def initialize()
            super()
            facet!() do
                externPaths << File.join(Build.VulkanSDK_Path, 'include')
            end
            facet!(platform: :Windows) do
                defines << 'VK_USE_PLATFORM_WIN32_KHR'
            end
            facet!(platform: :Win32) do
                path = File.join(Build.VulkanSDK_Path, 'Lib32')
                libraryPaths << path
                libraries << File.join(path, 'vulkan-1.lib')
            end
            facet!(platform: :Win64) do
                path = File.join(Build.VulkanSDK_Path, 'Lib')
                libraryPaths << path
                libraries << File.join(path, 'vulkan-1.lib')
            end
            facet!(platform: :Linux) do
                defines << 'VK_USE_PLATFORM_WAYLAND_KHR'
                libraryPaths << File.join(Build.VulkanSDK_Path, 'lib')
                libraries << 'vulkan-1.a'
            end
        end
    end #~ VulkanDecorator

    const_memoize(self, :VulkanSDK) do
        VulkanDecorator.new
    end

end #~ Build