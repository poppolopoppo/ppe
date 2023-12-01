﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"
#include "Test_Uniforms.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Compiler_Reflection3_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IRHIService& rhi = app.RHI();

    FComputePipelineDesc ppln;

    ppln.AddShader(EShaderLangFormat::GLSL_450, "main", R"#(
#pragma shader_stage(compute)
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x=16, local_size_y=8, local_size_z=1) in;

struct DynamicBuffer_Struct
{
	ivec2	i2;
	bool	b1;
	vec2	f2;
	ivec3	i3;
	bvec2	b2;
};

layout(binding=0, std140) coherent buffer DynamicBuffer_SSBO
{
	// static part
	vec2	f2;
	ivec4	i4;

	// dynamic part
	DynamicBuffer_Struct	arr[];

} ssb;

void main ()
{
	int idx = int(gl_GlobalInvocationID.x);

	ssb.arr[idx].i2 += ivec2(ssb.i4.x, idx);
	ssb.arr[idx].b1 = ((idx & 1) == 0);
	ssb.arr[idx].f2 -= vec2(ssb.f2);
	ssb.arr[idx].i3.xy *= 2;
	ssb.arr[idx].i3.z = ~ssb.arr[idx].i3.z;
	ssb.arr[idx].b2 = not(ssb.arr[idx].b2);
}
)#"
	ARGS_IF_RHIDEBUG("Compiler_Reflection3_CS"));

    const SPipelineCompiler compiler = rhi.Compiler(EShaderLangFormat::GLSL_450);
    PPE_LOG_CHECK(WindowTest, !!compiler);

    PPE_LOG_CHECK(WindowTest, compiler->Compile(ppln, EShaderLangFormat::SPIRV_100));

    const FDescriptorSet* ds = ppln.DescriptorSet("0"_descriptorset);
    PPE_LOG_CHECK(WindowTest, !!ds);

	PPE_LOG_CHECK(WindowTest, TestStorageBuffer(*ds, "DynamicBuffer_SSBO"_uniform, 32_b, 64_b, EShaderAccess::ReadWrite, 0, EShaderStages::Compute));

    PPE_LOG_CHECK(WindowTest, ppln.DefaultLocalGroupSize == uint3(16, 8, 1));

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
