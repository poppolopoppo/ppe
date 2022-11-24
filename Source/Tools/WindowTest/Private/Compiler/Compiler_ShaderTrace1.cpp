// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"
#include "Test_Uniforms.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Compiler_ShaderTrace1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IRHIService& rhi = app.RHI();

    FComputePipelineDesc ppln;

    ppln.AddShader(EShaderLangFormat::VKSL_100 | EShaderLangFormat::EnableDebugTrace | EShaderLangFormat::EnableTimeMap, "main", R"#(
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
	ARGS_IF_RHIDEBUG("Compiler_ShaderTrace1_CS"));

    const SPipelineCompiler compiler = rhi.Compiler(EShaderLangFormat::VKSL_100);
    LOG_CHECK(WindowTest, !!compiler);

    LOG_CHECK(WindowTest, compiler->Compile(ppln, EShaderLangFormat::SPIRV_100));

    auto it1 = ppln.Shader.Data.Find(EShaderLangFormat::SPIRV_100);
    LOG_CHECK(WindowTest, ppln.Shader.Data.end() != it1);

    auto it2 = ppln.Shader.Data.Find(EShaderLangFormat::SPIRV_100 | EShaderLangFormat::EnableDebugTrace);
    LOG_CHECK(WindowTest, ppln.Shader.Data.end() != it2);

    auto it3 = ppln.Shader.Data.Find(EShaderLangFormat::SPIRV_100 | EShaderLangFormat::EnableTimeMap);
    LOG_CHECK(WindowTest, ppln.Shader.Data.end() != it3);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
