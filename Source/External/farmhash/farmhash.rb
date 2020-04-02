# frozen_string_literal: true

$Build.ppe_external!(:farmhash) do
    force_includes!(File.join(abs_path, 'Public', 'farmhash-external.h'))
    extra_files!(*%w{
        Public/farmhash-external.h
        farmhash.git/src/farmhash.h })
    isolated_files!('farmhash.git/src/farmhash.cc')
end
