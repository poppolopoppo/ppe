// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"
#include "Test_Uniforms.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Compiler_UniformArray1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IRHIService& rhi = app.RHI();

    FComputePipelineDesc ppln;

    ppln.AddShader(EShaderLangFormat::GLSL_450, "main", R"#(
#version 460 core
#extension GL_ARB_shading_language_420pack : enable

layout (local_size_x = 8, local_size_y = 1, local_size_z = 1) in;

layout(binding=0) uniform texture2D  un_Textures[8];
layout(binding=2) uniform sampler    un_Sampler;

layout(binding=1) writeonly uniform image2D  un_OutImage;

void main ()
{
	const int	i		= int(gl_LocalInvocationIndex);
	const vec2	coord	= vec2(gl_GlobalInvocationID.xy) / vec2(gl_WorkGroupSize.xy * gl_NumWorkGroups.xy - 1);
		  vec4	color	= texture( sampler2D(un_Textures[i], un_Sampler), coord );

	imageStore( un_OutImage, ivec2(gl_GlobalInvocationID.xy), color );
}
)#"
	ARGS_IF_RHIDEBUG("Compiler_UniformArray1_CS"));

    const SPipelineCompiler compiler = rhi.Compiler(EShaderLangFormat::GLSL_450);
    PPE_LOG_CHECK(WindowTest, !!compiler);

    PPE_LOG_CHECK(WindowTest, compiler->Compile(ppln, EShaderLangFormat::SPIRV_100));

    const FDescriptorSet* ds = ppln.DescriptorSet("0"_descriptorset);
    PPE_LOG_CHECK(WindowTest, !!ds);

    PPE_LOG_CHECK(WindowTest, TestTextureUniform(*ds, "un_Textures"_uniform, EImageSampler::Float2D, 0, EShaderStages::Compute, 8));
    PPE_LOG_CHECK(WindowTest, TestSamplerUniform(*ds, "un_Sampler"_uniform, 2, EShaderStages::Compute, 1));
    PPE_LOG_CHECK(WindowTest, TestImageUniform(*ds, "un_OutImage"_uniform, EImageSampler::Float2D, EShaderAccess::WriteOnly, 1, EShaderStages::Compute, 1));

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
