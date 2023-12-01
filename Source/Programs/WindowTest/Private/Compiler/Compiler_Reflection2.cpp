// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"
#include "Test_Uniforms.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Compiler_Reflection2_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IRHIService& rhi = app.RHI();

    FGraphicsPipelineDesc ppln;

    ppln.AddShader(EShaderType::Vertex, EShaderLangFormat::GLSL_450, "main", R"#(
#version 450 core
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

layout (location=1) in  vec2	at_Position;
layout (location=0) in  vec2	at_Texcoord;
layout (location=2) in  uint	at_MaterialID;

layout (binding=0) uniform sampler2D un_ColorTexture;

layout(location=0) out vec2	v_Texcoord;
layout(location=1) out uint	v_MaterialID;

void main() {
	gl_Position	 = vec4( at_Position, 0.0, 1.0 );
	v_Texcoord	 = at_Texcoord * texelFetch( un_ColorTexture, ivec2(at_Texcoord), 0 ).xy;
	v_MaterialID = at_MaterialID;
}
)#"
	ARGS_IF_RHIDEBUG("Compiler_Reflection2_VS"));

	ppln.AddShader(EShaderType::Fragment, EShaderLangFormat::GLSL_450, "main", R"#(
#version 450 core
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable

layout (std140, binding=1) uniform UB
{
	vec4	color;

} ub;

layout (binding=0) uniform sampler2D un_ColorTexture;

layout(location=0) in  vec2	v_Texcoord;

layout(location=0) out vec4	out_Color;

void main() {
	out_Color = texture(un_ColorTexture, v_Texcoord) * ub.color;
}
)#"
	ARGS_IF_RHIDEBUG("Compiler_Reflection2_FS"));

    const SPipelineCompiler compiler = rhi.Compiler(EShaderLangFormat::GLSL_450);
    PPE_LOG_CHECK(WindowTest, !!compiler);

    PPE_LOG_CHECK(WindowTest, compiler->Compile(ppln, EShaderLangFormat::SPIRV_100));

	PPE_LOG_CHECK(WindowTest, ppln.VertexInput("at_Position"_vertex));
	PPE_LOG_CHECK(WindowTest, ppln.VertexInput("at_Texcoord"_vertex));
    PPE_LOG_CHECK(WindowTest, ppln.VertexInput("at_MaterialID"_vertex));

    PPE_LOG_CHECK(WindowTest, TestFragmentOutput(ppln, EFragmentOutput::Float4, 0));

    const FDescriptorSet* ds = ppln.DescriptorSet("0"_descriptorset);
    PPE_LOG_CHECK(WindowTest, !!ds);

    PPE_LOG_CHECK(WindowTest, TestTextureUniform(*ds, "un_ColorTexture"_uniform, EImageSampler::Float2D, 0, EShaderStages::Vertex | EShaderStages::Fragment));
    PPE_LOG_CHECK(WindowTest, TestBufferUniform(*ds, "UB"_uniform, 16_b, 1, EShaderStages::Fragment));

    PPE_LOG_CHECK(WindowTest, ppln.EarlyFragmentTests);

    PPE_LOG_CHECK(WindowTest, ppln.SupportedTopology & EPrimitiveTopology::Point);
    PPE_LOG_CHECK(WindowTest, ppln.SupportedTopology & EPrimitiveTopology::LineList);
    PPE_LOG_CHECK(WindowTest, ppln.SupportedTopology & EPrimitiveTopology::LineStrip);
    PPE_LOG_CHECK(WindowTest, ppln.SupportedTopology & EPrimitiveTopology::TriangleList);
    PPE_LOG_CHECK(WindowTest, ppln.SupportedTopology & EPrimitiveTopology::TriangleStrip);
    PPE_LOG_CHECK(WindowTest, ppln.SupportedTopology & EPrimitiveTopology::TriangleFan);

    PPE_LOG_CHECK(WindowTest, not (ppln.SupportedTopology & EPrimitiveTopology::LineListAdjacency));
    PPE_LOG_CHECK(WindowTest, not (ppln.SupportedTopology & EPrimitiveTopology::LineStripAdjacency));
    PPE_LOG_CHECK(WindowTest, not (ppln.SupportedTopology & EPrimitiveTopology::TriangleListAdjacency));
    PPE_LOG_CHECK(WindowTest, not (ppln.SupportedTopology & EPrimitiveTopology::TriangleStripAdjacency));
    PPE_LOG_CHECK(WindowTest, not (ppln.SupportedTopology & EPrimitiveTopology::Patch));

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
