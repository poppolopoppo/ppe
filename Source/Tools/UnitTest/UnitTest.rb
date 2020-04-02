# frozen_string_literal: true

$Build.ppe_executable!(:UnitTest) do
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
    public_deps!(*namespace[:ContentPipeline]{[ 
        BuildGraph() ]})
    extra_files!('resource.rc')
end
