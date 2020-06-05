# frozen_string_literal: true

$Build.ppe_module!(:RHI) do
    inherits!(Build.VulkanSDK)
    public_deps!(namespace.Core)
    private_deps!(*namespace[:External]{[vulkan]})
end
