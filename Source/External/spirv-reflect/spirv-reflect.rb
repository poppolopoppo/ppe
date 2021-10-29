# frozen_string_literal: true

$Build.ppe_external!('spirv-reflect') do
    force_includes!(File.join(abs_path, 'Public', 'spirv-reflect-external.h'))
    extra_files!(*%w{
        Public/spirv-reflect-external.h
        SPIRV-Reflect.git/spirv_reflect.h
        })
    source_files!(*%w{
        SPIRV-Reflect.git/spirv_reflect.c
        })
end
