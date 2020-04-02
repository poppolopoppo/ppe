# frozen_string_literal: true

$Build.ppe_module!(:Core) do
    unity_num_files = 5
    isolated_files!(
        'Private/Allocator/MallocBinned.cpp',
        'Private/Allocator/MallocStomp.cpp',
        'Private/Diagnostic/Benchmark.cpp' )
    private_deps!(*namespace[:External]{[
        double_conversion,
        farmhash,
        lz4,
        xxHash,
        vstools ]})
    extra_files!('resource.rc')
end
