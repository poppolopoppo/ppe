#pragma once

#include "RHI_fwd.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FDeviceProperties {
    bool	GeometryShader						: 1;	// for EShader::Geometry.
    bool	TessellationShader					: 1;	// for EShader::TessControl and EShader::TessEvaluation.
    bool	VertexPipelineStoresAndAtomics		: 1;	// for EBufferUsage::VertexPplnStore.
    bool	FragmentStoresAndAtomics			: 1;	// for EBufferUsage::FragmentPplnStore.
    bool	DedicatedAllocation					: 1;	// for EMemoryType::Dedicated.
    bool	DispatchBase						: 1;	// FDispatchCompute::FComputeCommand::BaseGroup can be used.
    bool	ImageCubeArray						: 1;	// for EImage::CubeArray.
    bool	Array2DCompatible					: 1;	// for EImageFlags::Array2DCompatible.
    bool	BlockTexelView						: 1;	// for EImageFlags::BlockTexelViewCompatible.
    bool	SamplerMirrorClamp					: 1;	// for EAddressMode::MirrorClampToEdge.
    bool	DescriptorIndexing					: 1;	// FPipelineResources::***::ElementCount can be set in runtime.
    bool	DrawIndirectCount					: 1;	// FDrawVerticesIndirectCount/FDrawIndexedIndirectCount can be used.
    bool	Swapchain							: 1;	// CreateSwapchain() can be used.
    bool	MeshShaderNV						: 1;	// CreatePipeline(FMeshPipelineDesc), FDrawMeshes*** can be used.
    bool	RayTracingNV						: 1;	// CreatePipeline(FRayTracingPipelineDesc), CreateRayTracingGeometry(), CreateRayTracingScene(), CreateRayTracingShaderTable(),
                                                        // FBuildRayTracingGeometry, FBuildRayTracingScene, FUpdateRayTracingShaderTable, FTraceRays can be used.
    bool	ShadingRateImageNV					: 1;	// FRenderPassDesc::SetShadingRateImage(), EImageUsage::ShadingRate can be used.

    size_t	MinStorageBufferOffsetAlignment;			// alignment of 'offset' argument in FPipelineResources::BindBuffer().
    size_t	MinUniformBufferOffsetAlignment;			// alignment of 'offset' argument in FPipelineResources::BindBuffer().
    u32		MaxDrawIndirectCount;						// max value of 'FDrawCommand::DrawCount' in FDrawVerticesIndirect/FDrawIndexedIndirect
                                                        // and max value of 'FDrawCommand::MaxDrawCount' in FDrawVerticesIndirectCount/FDrawIndexedIndirectCount.
    u32		MaxDrawIndexedIndexValue;					// max value of 'FDrawCommand::IndexCount' in draw commands.
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
