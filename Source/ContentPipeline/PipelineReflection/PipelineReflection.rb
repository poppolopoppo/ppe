# frozen_string_literal: true

$Build.ppe_module!(:PipelineReflection) do
    public_deps!(*namespace[:Runtime]{[
        Core(), VFS(), RHI() ]})
    private_deps!(*namespace[:Runtime]{[
        RHIVulkan() ]})
    private_deps!(*namespace[:External]{[
        spirv_cross, spirv_reflect ]})
end
