#pragma once

#include "RHI_fwd.h"

#if USE_PPE_RHIDEBUG
#   include "Color/Color.h"
#endif

#if USE_PPE_RHITASKNAME
#   include "IO/StaticString.h"
#endif

#include "IO/String.h"
#include "Maths/Units.h"
#include "Misc/Function.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
using FTaskName = TStaticString<64>;
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
using FShaderDebugCallback = TFunction<void(
    FStringView taskName,
    FStringView shaderName,
    EShaderStages stages,
    TMemoryView<const FString> output )
>;
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
struct FFrameStatistics {
    struct FRendering {
        u32 NumDescriptorBinds              = 0;
        u32 NumPushConstants                = 0;
        u32 NumPipelineBarriers             = 0;
        u32 NumTransferOps                  = 0;

        u32 NumIndexBufferBindings          = 0;
        u32 NumVertexBufferBindings         = 0;
        u32 NumDrawCalls                    = 0;
        u64 NumVertexCount                  = 0;
        u64 NumPrimitiveCount               = 0;
        u32 NumGraphicsPipelineBindings     = 0;
        u32 NumDynamicStateChanges          = 0;

        u32 NumDispatchCalls                = 0;
        u32 NumComputePipelineBindings      = 0;

        u32 NumRayTracingPipelineBindings   = 0;
        u32 NumTraceRaysCalls               = 0;
        u32 NumBuildASCalls                 = 0;

        FNanoseconds GpuTime{ 0 };
        FNanoseconds CpuTime{ 0 };

        FNanoseconds SubmittingTime{ 0 };
        FNanoseconds WaitingTime{ 0 };
    };

    struct FResources {
        u32 NumNewGraphicsPipeline          = 0;
        u32 NumNewComputePipeline           = 0;
        u32 NumNewRayTracingPipeline        = 0;
    };

    FRendering Renderer;
    FResources Resources;

    void Merge(const FFrameStatistics& other);
};
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
struct FDebugColorScheme {
    FRgba8u
        RenderPass{ FColor::OrangeRed() },
        Compute{ FColor::MediumBlue() },
        DeviceLocalTransfer{ FColor::Green() },
        HostToDeviceTransfer{ FColor::BlueViolet() },
        DeviceToHostTransfer{ FColor::BlueViolet() },
        Present{ FColor::Red() },
        RayTracing{ FColor::Lime() },
        BuildRayTracingStruct{ FColor::Lime() },

        Draw{ FColor::Bisque() },
        DrawMeshes{ FColor::Bisque() },
        CustomDraw{ FColor::Bisque() },

        CmdSubBatchBackground{ FColor::SlateBlue() },
        CmdSubBatchLabel{ FColor::LightGray() },
        CmbBatchBackground{ FColor::Aquamarine() },
        CmdBatchLabel{ FColor::LightGray() },
        TaskLabel{ FColor::White() },
        ResourceBackground{ FColor::Silver() },
        BarrierGroupBorder{ FColor::Olive() },
        GroupBorder{ FColor::DarkGray() },
        TaskDependency{ FColor::GhostWhite() },

        Debug{ FColor::Pink() },

        LayoutBarrier{ FColor::Yellow() },
        WriteAfterWriteBarrier{ FColor::DodgerBlue() },
        WriteAfterReadBarrier{ FColor::LimeGreen() },
        ReadAfterWriteBarrier{ FColor::Red() };

    static FDebugColorScheme Get() NOEXCEPT {
        // #TODO
        return FDebugColorScheme{};
    }
};
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
