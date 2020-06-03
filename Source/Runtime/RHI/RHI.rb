# frozen_string_literal: true

$Build.ppe_module!(:RHI) do
    define!('RHI_VULKAN', 'TARGET_RHI=Vulkan')
    inherits!(Build.VulkanSDK)
    public_deps!(namespace.Core)
    private_deps!(*namespace[:External]{[vulkan]})
end
