# frozen_string_literal: true

$Build.ppe_module!(:BuildGraph) do
    public_deps!(*namespace[:Runtime]{[
        Core(),
        VFS(),
        Application() ]})
    private_deps!(*namespace[:Runtime]{[
        Serialize() ]})
end
