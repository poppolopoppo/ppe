// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"
#include "Test_Uniforms.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Compiler_PushConst1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IRHIService& rhi = app.RHI();

    FGraphicsPipelineDesc ppln;
    ppln.AddShader(EShaderType::Vertex, EShaderLangFormat::VKSL_100, "main", R"#(
#version 450 core
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

layout(location=0) in  vec2	at_Position;
layout(location=1) in  vec2	at_Texcoord;

layout(location=0) out vec2	v_Texcoord;

layout (push_constant, std140) uniform VSPushConst {
	vec3	f1;
	ivec2	f2;
} pc;

void main() {
	gl_Position	= vec4( at_Position, 0.0, 1.0 );
	v_Texcoord	= at_Texcoord;
}
)#"
ARGS_IF_RHIDEBUG("Compiler_PushConst1_VS"));

    ppln.AddShader(EShaderType::Fragment, EShaderLangFormat::VKSL_100, "main", R"#(
#version 450 core
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable

layout (binding=1, std140) uniform UB {
	vec4	color;
} ub;

layout(binding=0) uniform sampler2D un_ColorTexture;

layout (push_constant, std140) uniform FSPushConst {
	layout(offset = 32)	float	f3;
						ivec2	f4;
	layout(offset = 64) vec4	f5;
} pc;

layout(location=0) in  vec2	v_Texcoord;

layout(location=0) out vec4	out_Color;

void main() {
	out_Color = texture(un_ColorTexture, v_Texcoord) * ub.color;
}
)#"
ARGS_IF_RHIDEBUG("Compiler_PushConst1_PS"));

    const SPipelineCompiler compiler = rhi.Compiler(EShaderLangFormat::VKSL_100);
    LOG_CHECK(WindowTest, !!compiler);

    LOG_CHECK(WindowTest, compiler->Compile(ppln, EShaderLangFormat::SPIRV_100));

    LOG_CHECK(WindowTest, TestPushConstant(ppln, "VSPushConst"_pushconstant, EShaderStages::Vertex, 0_b, 24_b));
    LOG_CHECK(WindowTest, TestPushConstant(ppln, "FSPushConst"_pushconstant, EShaderStages::Fragment, 32_b, 48_b));

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
