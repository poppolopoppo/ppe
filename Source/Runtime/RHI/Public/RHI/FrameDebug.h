#pragma once

#include "RHI_fwd.h"

#if USE_PPE_RHIDEBUG || USE_PPE_RHITASKNAME
#   include "Color/Color.h"
#endif

#if USE_PPE_RHITASKNAME
#   include "IO/StaticString.h"
#endif

#if USE_PPE_RHIDEBUG
#   include "Diagnostic/Logger.h"
#   include "IO/FormatHelpers.h"
#   include "Misc/Opaque.h"

#   define RHI_LOG(_LEVEL, ...)     PPE_LOG(RHI, _LEVEL, __VA_ARGS__)
#   define RHI_SLOG(_LEVEL, ...)    PPE_SLOG(RHI, _LEVEL, __VA_ARGS__)

#   if USE_PPE_RHITRACE
#       define RHI_TRACE(_TEXT, ...) PPE_SLOG(RHITrace, Verbose, _TEXT, {{"uid", ::PPE::RHI::FFrameStatistics::NextTraceUID()}, __VA_ARGS__ })
#   else
#       define RHI_TRACE(_TEXT, ...) NOOP()
#   endif

#else
#   define RHI_LOG(_LEVEL, ...) NOOP()
#   define RHI_SLOG(_LEVEL, ...) NOOP()
#   define RHI_TRACE(_TEXT, ...) NOOP()
#endif

#include "IO/String.h"
#include "Maths/Units.h"
#include "Misc/Function.h"
#include "Time/TimedScope.h"

#if USE_PPE_RHIDEBUG
#   define RHI_PROFILINGSCOPE(_NAME, _STAT) \
        ::PPE::RHI::FFrameStatistics::FProfilingScope ANONYMIZE(rhiProfilingScope){ _STAT }
#else
#   define RHI_PROFILINGSCOPE(_NAME, _STAT) NOOP()
#endif

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
#if USE_PPE_RHITRACE
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHITrace)
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
        u32 NumIndirectDrawCalls            = 0;
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

        PPE_RHI_API FRendering& operator +=(const FRendering& other) NOEXCEPT;
    };

    struct FResources {
        u32 NumNewGraphicsPipeline          = 0;
        u32 NumNewMeshPipeline              = 0;
        u32 NumNewComputePipeline           = 0;
        u32 NumNewRayTracingPipeline        = 0;

        PPE_RHI_API FResources& operator +=(const FResources& other) NOEXCEPT;
    };

    FRendering Renderer;
    FResources Resources;

    void Merge(const FFrameStatistics& other) NOEXCEPT {
        Renderer += other.Renderer;
        Resources += other.Resources;
    }

    void Reset() {
        Renderer = FRendering{};
        Resources = FResources{};
    }

    struct FProfilingScope : PPE::FTimedScope {
        FNanoseconds& Counter;
        explicit FProfilingScope(FNanoseconds* pCounter) NOEXCEPT : Counter(*pCounter) {}
        ~FProfilingScope() NOEXCEPT { Counter += Elapsed(); }
    };

#if USE_PPE_RHITRACE
    PPE_RHI_API static u64 NextTraceUID();
    PPE_RHI_API static void SetBreakOnTraceUID(u64 uid);
#endif

};
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG || USE_PPE_RHITASKNAME
struct FDebugColorScheme {
    FLinearColor
        RenderPass,
        Compute,
        DeviceLocalTransfer,
        HostToDeviceTransfer,
        DeviceToHostTransfer,
        Present,
        RayTracing,
        BuildRayTracingStruct,
        Draw,
        DrawMeshes,
        CustomDraw,
        CmdSubBatchBackground,
        CmdSubBatchLabel,
        CmbBatchBackground,
        CmdBatchLabel,
        TaskLabel,
        ResourceBackground,
        BarrierGroupBorder,
        GroupBorder,
        TaskDependency,
        Debug,
        LayoutBarrier,
        WriteAfterWriteBarrier,
        WriteAfterReadBarrier,
        ReadAfterWriteBarrier;

    PPE_RHI_API FDebugColorScheme() NOEXCEPT;
    PPE_RHI_API static const FDebugColorScheme& Get() NOEXCEPT;
};
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
