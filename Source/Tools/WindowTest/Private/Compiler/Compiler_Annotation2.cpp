#include "stdafx.h"

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Compiler_Annotation2_(FWindowTestApp& app) {
	using namespace PPE::RHI;

	IRHIService& rhi = app.RHI();

	FComputePipelineDesc ppln;
	ppln.AddShader(EShaderLangFormat::GLSL_450, "main", R"#(
#extension GL_ARB_shading_language_420pack : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (std140, binding=1) uniform UB {
	vec4	data[4];
} ub;

// @discard
layout (std430, binding=0) writeonly buffer SSB {
	vec4	data[4];
} ssb;

// @discard
layout(binding=2, rgba8) writeonly uniform image2D  un_Image;

void main ()
{
	ssb.data[0] = ub.data[1];
	ssb.data[3] = ub.data[2];
	ssb.data[1] = ub.data[3];
	ssb.data[2] = ub.data[0];

	imageStore( un_Image, ivec2(gl_GlobalInvocationID.xy), ub.data[gl_GlobalInvocationID.x & 3] );
}
)#"
ARGS_IF_RHIDEBUG("Compiler_Annotation2_CS"));

	const SPipelineCompiler compiler = rhi.Compiler(EShaderLangFormat::GLSL_450);
	LOG_CHECK(WindowTest, !!compiler);

	const EShaderCompilationFlags flags = compiler->CompilationFlags();
	compiler->SetCompilationFlags(flags | EShaderCompilationFlags::ParseAnnotations);

	LOG_CHECK(WindowTest, compiler->Compile(ppln, EShaderLangFormat::SPIRV_100));

	compiler->SetCompilationFlags(flags);

	const FDescriptorSet* ds = ppln.DescriptorSet("0"_descriptorset);
	LOG_CHECK(WindowTest, !!ds);

	LOG_CHECK(WindowTest, TestBufferUniform(*ds, "UB"_uniform, 64_b, 1, EShaderStages::Compute, 1, UMax));
	LOG_CHECK(WindowTest, TestStorageBuffer(*ds, "SSB"_uniform, 64_b, 0, EShaderAccess::WriteDiscard, 0, EShaderStages::Compute, 1, UMax));
	LOG_CHECK(WindowTest, TestImageUniform(*ds, "un_Image"_uniform,
		EImageSampler::Float2D | EImageSampler(EPixelFormat::RGBA8_UNorm),
		EShaderAccess::WriteDiscard, 2, EShaderStages::Compute ));

	LOG_CHECK(WindowTest, ppln.DefaultLocalGroupSize == uint3(1));

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
