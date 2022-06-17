#include "stdafx.h"

#include "Test_Includes.h"
#include "Test_Uniforms.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Compiler_Reflection1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IRHIService& rhi = app.RHI();

    FGraphicsPipelineDesc ppln;

    ppln.AddShader(EShaderType::Vertex, EShaderLangFormat::GLSL_450, "main", R"#(
#version 450 core
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

layout(location=0) in  vec2	at_Position;
layout(location=1) in  vec2	at_Texcoord;

layout(location=0) out vec2	v_Texcoord;

void main() {
	gl_Position	= vec4( at_Position, 0.0, 1.0 );
	v_Texcoord	= at_Texcoord;
}
)#"
	ARGS_IF_RHIDEBUG("Compiler_Reflection1_VS"));

	ppln.AddShader(EShaderType::Fragment, EShaderLangFormat::GLSL_450, "main", R"#(
#version 450 core
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable

layout(binding=0, std140) uniform UB
{
	vec4	color;

} ub;

layout(binding=1) uniform sampler2D un_ColorTexture;

layout(location=0) in  vec2	v_Texcoord;

layout(location=0) out vec4	out_Color;

void main() {
	out_Color = texture(un_ColorTexture, v_Texcoord) * ub.color;
}
)#"
	ARGS_IF_RHIDEBUG("Compiler_Reflection1_FS"));

    const SPipelineCompiler compiler = rhi.Compiler(EShaderLangFormat::GLSL_450);
    LOG_CHECK(WindowTest, !!compiler);

    LOG_CHECK(WindowTest, compiler->Compile(ppln, EShaderLangFormat::SPIRV_100));

	LOG_CHECK(WindowTest, ppln.VertexInput("at_Position"_vertex));
	LOG_CHECK(WindowTest, ppln.VertexInput("at_Texcoord"_vertex));

    LOG_CHECK(WindowTest, TestFragmentOutput(ppln, EFragmentOutput::Float4, 0));

    const FDescriptorSet* ds = ppln.DescriptorSet("0"_descriptorset);
    LOG_CHECK(WindowTest, !!ds);

    LOG_CHECK(WindowTest, ds->Uniform<FPipelineDesc::FTexture>("un_ColorTexture"_uniform).second);
    LOG_CHECK(WindowTest, ds->Uniform<FPipelineDesc::FUniformBuffer>("UB"_uniform).second);

    LOG_CHECK(WindowTest, ppln.EarlyFragmentTests);

    LOG_CHECK(WindowTest, ppln.SupportedTopology & EPrimitiveTopology::Point);
    LOG_CHECK(WindowTest, ppln.SupportedTopology & EPrimitiveTopology::LineList);
    LOG_CHECK(WindowTest, ppln.SupportedTopology & EPrimitiveTopology::LineStrip);
    LOG_CHECK(WindowTest, ppln.SupportedTopology & EPrimitiveTopology::TriangleList);
    LOG_CHECK(WindowTest, ppln.SupportedTopology & EPrimitiveTopology::TriangleStrip);
    LOG_CHECK(WindowTest, ppln.SupportedTopology & EPrimitiveTopology::TriangleFan);

    LOG_CHECK(WindowTest, not (ppln.SupportedTopology & EPrimitiveTopology::LineListAdjacency));
    LOG_CHECK(WindowTest, not (ppln.SupportedTopology & EPrimitiveTopology::LineStripAdjacency));
    LOG_CHECK(WindowTest, not (ppln.SupportedTopology & EPrimitiveTopology::TriangleListAdjacency));
    LOG_CHECK(WindowTest, not (ppln.SupportedTopology & EPrimitiveTopology::TriangleStripAdjacency));
    LOG_CHECK(WindowTest, not (ppln.SupportedTopology & EPrimitiveTopology::Patch));

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
