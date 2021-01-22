# frozen_string_literal: true

$Build.ppe_module!(:RHIVulkan) do
    public_deps!(namespace.Core, namespace.RHI)
    private_deps!(*namespace[:External]{[vulkan]})
end
