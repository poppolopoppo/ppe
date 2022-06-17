#include "stdafx.h"

#include "Test_Includes.h"
#include "Test_Uniforms.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Compiler_ComputeLocalSize1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IRHIService& rhi = app.RHI();

    FComputePipelineDesc ppln;
    ppln.AddShader(EShaderLangFormat::GLSL_450, "main", R"#(
#pragma shader_stage(compute)
#extension GL_ARB_separate_shader_objects : enable

layout (constant_id = 2) const float SCALE = 0.5f;

layout (local_size_x=8, local_size_y=8, local_size_z=1) in;
layout (local_size_x_id = 0, local_size_y_id = 1) in;

layout(binding=0, rgba8) writeonly uniform image2D  un_OutImage;

layout(binding=1, std140) readonly buffer un_SSBO
{
    vec4 ssb_data;
};

const float uv_scale = 0.1f;

void main ()
{
    vec2 uv = vec2(gl_GlobalInvocationID.xy) / vec2((gl_WorkGroupSize * gl_NumWorkGroups).xy) * (uv_scale + SCALE);

    vec4 fragColor = vec4(sin(uv.x), cos(uv.y), 1.0, ssb_data.r);

    imageStore( un_OutImage, ivec2(gl_GlobalInvocationID.xy), fragColor );
}
)#"
ARGS_IF_RHIDEBUG("Compiler_ComputeLocalSize1_CS"));

    const SPipelineCompiler compiler = rhi.Compiler(EShaderLangFormat::GLSL_450);
    LOG_CHECK(WindowTest, !!compiler);

    LOG_CHECK(WindowTest, compiler->Compile(ppln, EShaderLangFormat::SPIRV_100));

    FDescriptorSet* const ds = ppln.DescriptorSet("0"_descriptorset);
    LOG_CHECK(WindowTest, !!ds);

    LOG_CHECK(WindowTest, TestImageUniform(*ds, "un_OutImage"_uniform, EImageSampler::Float2D | EImageSampler(EPixelFormat::RGBA8_UNorm), EShaderAccess::WriteOnly, 0, EShaderStages::Compute));
    LOG_CHECK(WindowTest, TestStorageBuffer(*ds, "un_SSBO"_uniform, 16_b, 0_b, EShaderAccess::ReadOnly, 1, EShaderStages::Compute));

    LOG_CHECK(WindowTest, ppln.DefaultLocalGroupSize == uint3{ 8,8,1 });
    LOG_CHECK(WindowTest, ppln.LocalSizeSpecialization == uint3{ 0,1,UMax });

    LOG_CHECK(WindowTest, ppln.Shader.Specializations.size() == 1);
    LOG_CHECK(WindowTest, TestSpecializationConstant(ppln.Shader, "SCALE"_specialization, 2));

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
