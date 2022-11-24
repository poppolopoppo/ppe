// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"
#include "Test_Uniforms.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Compiler_MRT1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IRHIService& rhi = app.RHI();

    FGraphicsPipelineDesc ppln;
    ppln.AddShader(EShaderType::Vertex, EShaderLangFormat::GLSL_450, "main", R"#(
#version 450 core
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

layout(location=0) in  vec3	in_Position;
layout(location=2) in  vec2	in_Texcoord;

layout(location=0) out vec2	out_Texcoord;

void main() {
	gl_Position	 = vec4( in_Position, 1.0 );
	out_Texcoord = in_Texcoord;
}
)#"
ARGS_IF_RHIDEBUG("Compiler_MRT1_MT"));

    ppln.AddShader(EShaderType::Fragment, EShaderLangFormat::GLSL_450, "main", R"#(
#version 450 core
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable

layout(binding=0) uniform sampler2D  un_Texture1;
layout(binding=1) uniform usampler2D un_Texture2;

layout(location=0) in  vec2	in_Texcoord;

layout(location=0) out vec4	 out_Color0;
layout(location=2) out uvec4 out_Color1;

void main() {
	out_Color0 = texture(un_Texture1, in_Texcoord);
	out_Color1 = texture(un_Texture2, in_Texcoord);
}
)#"
ARGS_IF_RHIDEBUG("Compiler_MRT1_MS"));

    const SPipelineCompiler compiler = rhi.Compiler(EShaderLangFormat::GLSL_450);
    LOG_CHECK(WindowTest, !!compiler);

    const EShaderCompilationFlags flags = compiler->CompilationFlags();
    compiler->SetCompilationFlags(EShaderCompilationFlags::Unknown);

    LOG_CHECK(WindowTest, compiler->Compile(ppln, EShaderLangFormat::SPIRV_100));

    compiler->SetCompilationFlags(flags);

    LOG_CHECK(WindowTest, TestVertexInput(ppln, "in_Position"_vertex, EVertexFormat::Float3, 0));
    LOG_CHECK(WindowTest, TestVertexInput(ppln, "in_Texcoord"_vertex, EVertexFormat::Float2, 2));

    LOG_CHECK(WindowTest, TestFragmentOutput(ppln, EFragmentOutput::Float4, 0));
    LOG_CHECK(WindowTest, TestFragmentOutput(ppln, EFragmentOutput::UInt4, 2));

    const FDescriptorSet* ds = ppln.DescriptorSet("0"_descriptorset);
    LOG_CHECK(WindowTest, !!ds);

    LOG_CHECK(WindowTest, TestTextureUniform(*ds, "un_Texture1"_uniform, EImageSampler::Float2D, /*binding*/0, EShaderStages::Fragment, /*arraySize*/1 ));
    LOG_CHECK(WindowTest, TestTextureUniform(*ds, "un_Texture2"_uniform, EImageSampler::UInt2D, /*binding*/1, EShaderStages::Fragment, /*arraySize*/1 ));

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
