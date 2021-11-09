# frozen_string_literal: true

$Build.ppe_module!(:PipelineCompiler) do
    #link!(:dynamic)
    public_deps!(*namespace[:Runtime]{[
        Core(), VFS(), RHI() ]})
    private_deps!(*namespace[:Runtime]{[
        RHIVulkan() ]})
    private_deps!(*namespace[:External]{[
        vulkan(), glslang(), spirv_tools(), glsl_trace() ]})
    custom!() do |env, target|
        case env.platform.os
        when :Windows
            # warning LNK4006: "class TCompiler * __cdecl ConstructCompiler(enum EShLanguage,int)" (?ConstructCompiler@@YAPEAVTCompiler@@W4EShLanguage@@H@Z) already defined in glslang.lib(CodeGen.obj); second definition ignored
            librarianOptions << '/IGNORE:4006'
        end
    end
end
