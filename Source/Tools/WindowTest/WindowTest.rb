# frozen_string_literal: true

$Build.ppe_executable!(:WindowTest, :Tools) do
    public_deps!(*namespace[:Runtime]{[
        Core(),
        VFS(),
        RTTI(),
        Serialize(),
        Network(),
        Application() ]})
    private_deps!(*namespace[:External]{[
        vulkan ]})
    runtime_deps!(*namespace[:ContentPipeline]{[
        BuildGraph() ]})
end
