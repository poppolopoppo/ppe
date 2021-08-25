# frozen_string_literal: true

$Build.ppe_module!(:Core) do
    isolated_files!(
        'Private/Allocator/InitSegAllocator.cpp',
        'Private/Diagnostic/Benchmark.cpp' )
    private_deps!(*namespace[:External]{[
        double_conversion,
        farmhash,
        lz4,
        xxHash ]})
    runtime_deps!(*namespace[:External]{[ vstools ]}) if Build.os_windows?
    custom!() do |env, target|
        case env.platform.os
        when :Linux
            compilationFlag!('-ldl', '-lncurses', '-lrt')
        end
    end
end
