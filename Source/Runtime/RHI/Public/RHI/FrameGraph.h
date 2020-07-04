#pragma once

#include "RHI_fwd.h"

#include "HAL/RHI_fwd.h"
#include "HAL/RHIFixedFunctionState.h"
#include "HAL/RHIFormat.h"

#include "Container/AssociativeVector.h"
#include "Diagnostic/Logger_fwd.h"
#include "Maths/ScalarVector.h"
#include "Meta/Enum.h"
#include "Meta/StronglyTyped.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, FrameGraph);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EFrameResourceFlags : u32 {
	None        = 0,
	Read        = 1 << 0,
	Write       = 1 << 1,
    Concurrent  = 1 << 2,
};
ENUM_FLAGS(EFrameResourceFlags);
//----------------------------------------------------------------------------
PPE_STRONGLYTYPED_NUMERIC_DEFAULTVALUE_DEF(u32, FFrameResourceID, u32(-1));
//----------------------------------------------------------------------------
struct FFrameResource {
    u32 RefCount{ 0 };
    EFrameResourceFlags Flags{ EFrameResourceFlags::None };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EBufferWriteMode : u32 {
    Reset,
    Append,
};
//----------------------------------------------------------------------------
struct FBufferDescription {
    u32 Capacity{ 0 };
    EVertexFormat Format{ EVertexFormat::R8G8B8A8_UNORM };
    EBufferWriteMode  WriteMode{ EBufferWriteMode::Reset };
};
//----------------------------------------------------------------------------
PPE_STRONGLYTYPED_NUMERIC_DEFAULTVALUE_DEF(u32, FBufferID, u32(-1));
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ERenderTargetClearMode : u32 {
	DontCare,
	ExplicitlyClear,
};
//----------------------------------------------------------------------------
enum class ERenderTargetSizeMode : u32 {
	Absolute,
	InputRelative,
	ViewportRelative,
};
//----------------------------------------------------------------------------
struct FRenderTargetDescription {
    u32 NumSlices{ 1 };
    float2 Extent{ float2::One };
    float4 ClearValue{ float4::Zero };
    EPixelFormat Format{ EPixelFormat::R8G8B8A8_UNORM };
    ERenderTargetClearMode ClearMode{ ERenderTargetClearMode::DontCare };
    ERenderTargetSizeMode SizeMode{ ERenderTargetSizeMode::InputRelative };
};
//----------------------------------------------------------------------------
PPE_STRONGLYTYPED_NUMERIC_DEFAULTVALUE_DEF(u32, FRenderTargetID, u32(-1));
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FFramePassBuilder {
    FFramePassBuilder() NOEXCEPT;

    FBufferID CreateBuffer(const FBufferDescription& desc);
    void ReadBuffer(FBufferID buffer);
    void SetOutputBuffer(FBufferID buffer);
    void DestroyBuffer(FBufferID buffer);

	FRenderTargetID CreateRenderTarget(const FBufferDescription& desc);
    void ResolveRenderTarget(FRenderTargetID renderTarget);
    void SetRenderTarget(std::initializer_list<FRenderTargetID> renderTargets);
	void DestroyRenderTarget(FRenderTargetID renderTarget);
};
//----------------------------------------------------------------------------
struct FFramePass {
    FFixedFunctionState State;
    VECTORINSITU(FrameGraph, FBufferDescription, 1) Buffers;
    VECTORINSITU(FrameGraph, FRenderTargetDescription, 1) RenderTargets;
};
//----------------------------------------------------------------------------
class PPE_RHI_API FFrameGraph : public Meta::FNonCopyable {
public:
    explicit FFrameGraph(FDevice& device) NOEXCEPT;



private:
    FDevice* _device;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
