#include "stdafx.h"

#include "Test_Includes.h"
#include "Test_Uniforms.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Compiler_Optimization1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IRHIService& rhi = app.RHI();

    FComputePipelineDesc ppln1;
    ppln1.AddShader(EShaderLangFormat::VKSL_100, "main", R"#(
#extension GL_ARB_shading_language_420pack : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(binding=0, rgba8) writeonly uniform image2D  un_OutImage;

void main ()
{
	ivec2	coord	= ivec2(gl_GlobalInvocationID.xy);
	float	xsin	= sin(float(coord.x));
	float	ycos	= cos(float(coord.y));
	float	xysin	= sin(float(coord.x + coord.y));

	imageStore( un_OutImage, coord, vec4(xsin, ycos, xysin, 1.0f) );
}
)#"
ARGS_IF_RHIDEBUG("Compiler_Optimization1_CS"));

    FComputePipelineDesc ppln2{ ppln1 };

    const SPipelineCompiler compiler = rhi.Compiler(EShaderLangFormat::GLSL_450);
    LOG_CHECK(WindowTest, !!compiler);

    const EShaderCompilationFlags flags = compiler->CompilationFlags();
    compiler->SetCompilationFlags(EShaderCompilationFlags::Unknown);

    LOG_CHECK(WindowTest, compiler->Compile(ppln1, EShaderLangFormat::SPIRV_100));

    compiler->SetCompilationFlags(
        EShaderCompilationFlags::Optimize +
        EShaderCompilationFlags::OptimizeSize +
        EShaderCompilationFlags::StrongOptimization );

    LOG_CHECK(WindowTest, compiler->Compile(ppln2, EShaderLangFormat::SPIRV_100));

    compiler->SetCompilationFlags(flags);

    const FShaderDataVariant* const it1 = ppln1.Shader.Find(EShaderLangFormat::SPIRV_100);
    LOG_CHECK(WindowTest, !!it1);

    const FShaderDataVariant* const it2 = ppln2.Shader.Find(EShaderLangFormat::SPIRV_100);
    LOG_CHECK(WindowTest, !!it2);

    const PShaderBinaryData* const pBinary1 = std::get_if<PShaderBinaryData>(it1);
    LOG_CHECK(WindowTest, !!pBinary1 && pBinary1->valid());

    const PShaderBinaryData* const pBinary2 = std::get_if<PShaderBinaryData>(it2);
    LOG_CHECK(WindowTest, !!pBinary2 && pBinary2->valid());

    const IShaderData<FRawData>& shaderData1 = (*pBinary1->get());
    const IShaderData<FRawData>& shaderData2 = (*pBinary2->get());

    LOG_CHECK(WindowTest, shaderData1.EntryPoint() == shaderData2.EntryPoint());
    LOG_CHECK(WindowTest, shaderData1.Data()->SizeInBytes() > shaderData2.Data()->SizeInBytes());

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
