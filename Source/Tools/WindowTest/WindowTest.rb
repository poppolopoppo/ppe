# frozen_string_literal: true

$Build.ppe_executable!(:WindowTest, :Tools) do
    private_deps!(*namespace[:External]{[
        double_conversion,
        farmhash,
        iaca,
        lz4,
        xxHash ]})
    public_deps!(*namespace[:Runtime]{[
        Core(),
        VFS(),
        RTTI(),
        Serialize(),
        Network(),
        Application() ]})
    runtime_deps!(*namespace[:ContentPipeline]{[
        BuildGraph() ]})
end
