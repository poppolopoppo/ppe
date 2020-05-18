# frozen_string_literal: true

$Build.ppe_executable!(:UnitTest, :Tools) do
    public_deps!(*namespace[:Runtime]{[
        Core(),
        VFS(),
        RTTI(),
        Serialize(),
        Network(),
        Application() ]})
    runtime_deps!(*namespace[:ContentPipeline]{[
        BuildGraph() ]})
    excluded_files!(*%w{ Private/Test_Pixmap.cpp Private/Test_Lattice.cpp })
end
