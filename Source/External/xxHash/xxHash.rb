# frozen_string_literal: true

$Build.ppe_external!(:xxHash) do
    force_includes!(File.join(abs_path, 'Public', 'xxHash-external.h'))
    extra_files!(*%w{
        Public/xxHash-external.h
        xxHash.git/xxhash.h })
    isolated_files!('Private/xxHash-dummy.cpp')
end
