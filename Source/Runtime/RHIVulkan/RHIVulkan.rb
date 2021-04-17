# frozen_string_literal: true

$Build.ppe_module!(:RHIVulkan) do
    public_deps!(namespace.Core, namespace.RHI, namespace.Application)
    private_deps!(*namespace[:External]{[vulkan]})
end
