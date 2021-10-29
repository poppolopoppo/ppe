# frozen_string_literal: true

# TODO: port to internal library

$Build.ppe_external!('glsl_trace') do
    private_deps!(*namespace[:External]{[glslang]})
    force_includes!(File.join(abs_path, 'Public', 'glsl_trace-external.h'))
    extra_files!(*%w{
        Public/glsl_trace-external.h
        glsl_trace.git/shader_trace/include/ShaderTrace.h
        glsl_trace.git/shader_trace/source/Common.h
        })
    source_files!(*%w{
        glsl_trace.git/shader_trace/source/InsertFunctionProfiler.cpp
        glsl_trace.git/shader_trace/source/InsertShaderClockHeatmap.cpp
        glsl_trace.git/shader_trace/source/InsertTraceRecording.cpp
        glsl_trace.git/shader_trace/source/ParseShaderTrace.cpp
        glsl_trace.git/shader_trace/source/ShaderTrace.cpp
        glsl_trace.git/shader_trace/source/Utils.cpp
        })
    custom!() do |env, target|
        case env.platform.os
        when :Windows
            compilerOptions << '/WX-' << '/permissive'
        end
    end
end
