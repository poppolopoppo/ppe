# frozen_string_literal: true

$Build.ppe_module!(:Network) do
    public_deps!(namespace.Core)
    isolated_files!('Private/Network_extern.cpp')
end
