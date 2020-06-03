# frozen_string_literal: true

$Build.ppe_executable!(:WindowTest, :Tools) do
    public_deps!(*namespace[:Runtime]{[
        Core(),
        VFS(),
        RHI(),
        RTTI(),
        Serialize(),
        Network(),
        Application() ]})
    runtime_deps!(*namespace[:ContentPipeline]{[
        BuildGraph() ]})
end
