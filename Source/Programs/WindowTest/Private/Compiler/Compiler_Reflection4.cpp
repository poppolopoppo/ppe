// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"
#include "Test_Uniforms.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Compiler_Reflection4_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IRHIService& rhi = app.RHI();

    FComputePipelineDesc ppln;

    ppln.AddShader(EShaderLangFormat::GLSL_450, "main", R"#(
#pragma shader_stage(compute)

layout (local_size_x=16, local_size_y=8, local_size_z=1) in;

layout(binding=0, rgba8) writeonly uniform image2D  un_OutImage;

layout(binding=1, std140) readonly buffer un_SSBO
{
	vec4 ssb_data;
};

void main ()
{
	vec2 uv = vec2(gl_GlobalInvocationID.xy) / vec2((gl_WorkGroupSize * gl_NumWorkGroups).xy);

	vec4 fragColor = vec4(sin(uv.x), cos(uv.y), 1.0, ssb_data.r);

	imageStore( un_OutImage, ivec2(gl_GlobalInvocationID.xy), fragColor );
}
)#"
	ARGS_IF_RHIDEBUG("Compiler_Reflection4_CS"));

    const SPipelineCompiler compiler = rhi.Compiler(EShaderLangFormat::GLSL_450);
    PPE_LOG_CHECK(WindowTest, !!compiler);

    PPE_LOG_CHECK(WindowTest, compiler->Compile(ppln, EShaderLangFormat::SPIRV_100));

    const FDescriptorSet* ds = ppln.DescriptorSet("0"_descriptorset);
    PPE_LOG_CHECK(WindowTest, !!ds);

    PPE_LOG_CHECK(WindowTest, TestImageUniform(*ds, "un_OutImage"_uniform, EImageSampler::Float2D | EImageSampler_FromPixelFormat(EPixelFormat::RGBA8_UNorm), EShaderAccess::WriteOnly, 0, EShaderStages::Compute));
    PPE_LOG_CHECK(WindowTest, TestStorageBuffer(*ds, "un_SSBO"_uniform, 16_b, 0_b, EShaderAccess::ReadOnly, 1, EShaderStages::Compute));

    PPE_LOG_CHECK(WindowTest, ppln.DefaultLocalGroupSize == uint3(16, 8, 1));

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
