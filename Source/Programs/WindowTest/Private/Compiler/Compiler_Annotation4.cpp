// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"
#include "Test_Uniforms.h"

#include "RHI/PipelineCompiler.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Compiler_Annotation4_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IRHIService& rhi = app.RHI();

    FComputePipelineDesc ppln;
    ppln.AddShader(EShaderLangFormat::GLSL_450, "main", R"#(
#extension GL_ARB_shading_language_420pack : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (std140, binding=1) uniform UB {
    vec4	data[4];
} ub;

// @dynamic-offset
layout (std430, binding=0) writeonly buffer SSB {
    vec4	data[4];
} ssb;

void main ()
{
    ssb.data[0] = ub.data[1];
    ssb.data[3] = ub.data[2];
    ssb.data[1] = ub.data[3];
    ssb.data[2] = ub.data[0];
}
)#"
ARGS_IF_RHIDEBUG("Compiler_Annotation4_CS"));

    const SPipelineCompiler compiler = rhi.Compiler(EShaderLangFormat::GLSL_450);
    PPE_LOG_CHECK(WindowTest, !!compiler);

    const EShaderCompilationFlags flags = compiler->CompilationFlags();
    compiler->SetCompilationFlags(flags | EShaderCompilationFlags::ParseAnnotations);

    PPE_LOG_CHECK(WindowTest, compiler->Compile(ppln, EShaderLangFormat::SPIRV_100));

    compiler->SetCompilationFlags(flags);

    const FDescriptorSet* ds = ppln.DescriptorSet("0"_descriptorset);
    PPE_LOG_CHECK(WindowTest, !!ds);

    PPE_LOG_CHECK(WindowTest, TestBufferUniform(*ds, "UB"_uniform, 64_b, 1, EShaderStages::Compute, 1, UMax));
    PPE_LOG_CHECK(WindowTest, TestStorageBuffer(*ds, "SSB"_uniform, 64_b, 0, EShaderAccess::WriteOnly, 0, EShaderStages::Compute, 1, 0));

    PPE_LOG_CHECK(WindowTest, ppln.DefaultLocalGroupSize == uint3(1));

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
