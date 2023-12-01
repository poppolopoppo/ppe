// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"
#include "Test_Uniforms.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Compiler_Reflection5_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IRHIService& rhi = app.RHI();

    FComputePipelineDesc ppln;

    ppln.AddShader(EShaderLangFormat::GLSL_450, "main", R"#(
#pragma shader_stage(compute)
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x=1, local_size_y=1, local_size_z=1) in;

layout(binding=0, rgba8) readonly  uniform image2D			un_Image2D_0;
layout(binding=1, r16ui) coherent  uniform uimage2D			un_Image2D_1;
layout(binding=2)        writeonly uniform image2DMS		un_Image2DMS_2;
layout(binding=3)                  uniform sampler3D		un_Image3D_3;
layout(binding=4)                  uniform sampler2DArray	un_Image2DA_4;
layout(binding=5)                  uniform isampler1D		un_Image1D_5;
layout(binding=6)                  uniform sampler2DShadow	un_Image2DS_6;

void main ()
{
}
)#"
	ARGS_IF_RHIDEBUG("Compiler_Reflection5_CS"));

    const SPipelineCompiler compiler = rhi.Compiler(EShaderLangFormat::GLSL_450);
    PPE_LOG_CHECK(WindowTest, !!compiler);

    PPE_LOG_CHECK(WindowTest, compiler->Compile(ppln, EShaderLangFormat::SPIRV_100));

    const FDescriptorSet* ds = ppln.DescriptorSet("0"_descriptorset);
    PPE_LOG_CHECK(WindowTest, !!ds);

    PPE_LOG_CHECK(WindowTest, TestImageUniform(*ds, "un_Image2D_0"_uniform, EImageSampler(EPixelFormat::RGBA8_UNorm) | EImageSampler::Float2D, EShaderAccess::ReadOnly, 0, EShaderStages::Compute));
    PPE_LOG_CHECK(WindowTest, TestImageUniform(*ds, "un_Image2D_1"_uniform, EImageSampler(EPixelFormat::R16u) | EImageSampler::UInt2D, EShaderAccess::ReadWrite, 1, EShaderStages::Compute));
    PPE_LOG_CHECK(WindowTest, TestImageUniform(*ds, "un_Image2DMS_2"_uniform, EImageSampler::Float2DMS, EShaderAccess::WriteOnly, 2, EShaderStages::Compute));

    PPE_LOG_CHECK(WindowTest, TestTextureUniform(*ds, "un_Image3D_3"_uniform, EImageSampler::Float3D, 3, EShaderStages::Compute));
    PPE_LOG_CHECK(WindowTest, TestTextureUniform(*ds, "un_Image2DA_4"_uniform, EImageSampler::Float2DArray, 4, EShaderStages::Compute));
    PPE_LOG_CHECK(WindowTest, TestTextureUniform(*ds, "un_Image1D_5"_uniform, EImageSampler::Int1D, 5, EShaderStages::Compute));
    PPE_LOG_CHECK(WindowTest, TestTextureUniform(*ds, "un_Image2DS_6"_uniform, EImageSampler::Float2D | EImageSampler::_Shadow, 6, EShaderStages::Compute));

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
