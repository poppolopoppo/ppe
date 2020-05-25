# frozen_string_literal: true

require 'pathname'

Build.import_envvar(:VK_SDK_PATH).validate_DirExist!
Build.import_envvar(:VULKAN_SDK).validate_DirExist!

Build.const_memoize(Build, :VulkanSDK_Path) do
    sdkPath = Build.VULKAN_SDK || Build.VK_SDK_PATH
    sdkVer = File.basename(sdkPath)
    Build::Log.log('Vulkan: found SDK v%s in "%s"', sdkVer, sdkPath)
    Pathname.new(sdkPath).expand_path.to_s
end

$Build.ppe_external!(:vulkan) do
    extra_files!(*%w{
        Public/vulkan-external.h })
    source_files!('Private/vulkan.cpp')

    vulkanSDK = Build.VulkanSDK_Path
    vulkanIncludePath = File.join(vulkanSDK, 'include')

    custom! do |env, target|
        case env.platform.os
        when :Windows
            libraryPaths << (Build.os_x64? ?
                File.join(vulkanSDK, 'Lib') :
                File.join(vulkanSDK, 'Lib32'))
            libraries << 'vulkan-1.lib'
        when :Linux
            libraryPaths << File.join(vulkanSDK, 'lib')
            libraries << 'vulkan-1.a'
        else
            Assert.unexpected(env.platform.os)
        end
    end

    externPaths << vulkanIncludePath

    vulkanHeaders = Pathname.new(File.join(vulkanIncludePath, 'vulkan', '*.h')).cleanpath
    extra_files!(*Dir[vulkanHeaders])

    # header-only library
end
