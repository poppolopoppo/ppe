# frozen_string_literal: true

$Build.ppe_external!(:vulkan) do
    extra_files!(*%w{
        Public/vulkan-external.h })
    source_files!('Private/vulkan.cpp')
    inherits!(Build.VulkanSDK)
    # header-only library
end
