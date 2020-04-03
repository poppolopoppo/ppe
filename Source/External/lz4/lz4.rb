# frozen_string_literal: true

$Build.ppe_external!(:lz4) do
    force_includes!(File.join(abs_path, 'Public', 'lz4-external.h'))
    extra_files!(*%w{
        Public/lz4-external.h
        lz4.git/lib/lz4.h
        lz4.git/lib/lz4hc.h
        lz4.git/lib/lz4opt.h })
    source_files!(*%w{
        lz4.git/lib/lz4.c
        lz4.git/lib/lz4hc.c })
end
