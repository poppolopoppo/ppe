# frozen_string_literal: true

$Build.namespace(:External) do
    include!('double-conversion/double-conversion.rb')
    include!('farmhash/farmhash.rb')
    include!('glslang/glslang.rb')
    include!('glsl_trace/glsl_trace.rb')
    include!('iaca/iaca.rb')
    include!('lz4/lz4.rb')
    include!('renderdoc/renderdoc.rb')
    include!('spirv-cross/spirv-cross.rb')
    include!('spirv-reflect/spirv-reflect.rb')
    include!('stb/stb.rb')
    include!('vma/vma.rb')
    include!('vstools/vstools.rb')
    include!('vulkan/vulkan.rb')
    include!('xxHash/xxHash.rb')
end
