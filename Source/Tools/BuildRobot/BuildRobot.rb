# frozen_string_literal: true

$Build.ppe_executable!(:BuildRobot, :Tools) do
    public_deps!(*namespace[:Runtime]{[
        Core(),
        VFS(),
        RTTI(),
        Serialize(),
        Network(),
        Application() ]})
    public_deps!(*namespace[:ContentPipeline]{[
        BuildGraph() ]})
end
