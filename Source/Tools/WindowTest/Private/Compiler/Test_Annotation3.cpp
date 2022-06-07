#include "stdafx.h"

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Test_Annotation3_(FWindowTestApp& app) {
	using namespace PPE::RHI;

	IRHIService& rhi = app.RHI();

	FComputePipelineDesc ppln;
	ppln.AddShader(EShaderLangFormat::GLSL_450, "main", R"#(
#extension GL_ARB_shading_language_420pack : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// @set 0 test, @dynamic-offset
layout (std140, binding=1, std140) uniform UB {
	vec4	data[4];
} ub;

// @dynamic-offset, @discard
layout (std430, binding=0, std430) restrict writeonly buffer SSB {
	vec4	data[4];
} ssb;

layout(push_constant, std140) uniform PC {
	vec4	data;
} pc;

void main ()
{
	ssb.data[0] = ub.data[1] + pc.data[3];
	ssb.data[3] = ub.data[2] + pc.data[1];
	ssb.data[1] = ub.data[3] + pc.data[0];
	ssb.data[2] = ub.data[0] + pc.data[2];
}
)#"
ARGS_IF_RHIDEBUG("Test_Annotation3_CS"));

	const SPipelineCompiler compiler = rhi.Compiler(EShaderLangFormat::GLSL_450);
	LOG_CHECK(WindowTest, !!compiler);

	const EShaderCompilationFlags flags = compiler->CompilationFlags();
	compiler->SetCompilationFlags(flags | EShaderCompilationFlags::ParseAnnotations);

	LOG_CHECK(WindowTest, compiler->Compile(ppln, EShaderLangFormat::SPIRV_100));

	compiler->SetCompilationFlags(flags);

	const FDescriptorSet* ds = ppln.DescriptorSet("test"_descriptorset);
	LOG_CHECK(WindowTest, !!ds);

	LOG_CHECK(WindowTest, TestBufferUniform(*ds, "UB"_uniform, 64_b, 1, EShaderStages::Compute, 1, 1));
	LOG_CHECK(WindowTest, TestStorageBuffer(*ds, "SSB"_uniform, 64_b, 0, EShaderAccess::WriteDiscard, 0, EShaderStages::Compute, 1, 0));

	LOG_CHECK(WindowTest, ppln.DefaultLocalGroupSize == uint3(1));

	return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
