#pragma once

#include "RHI_fwd.h"

#if USE_PPE_RHIDEBUG
#   include "Color/Color.h"
#endif

#if USE_PPE_RHITASKNAME
#   include "IO/StaticString.h"
#endif

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
#if USE_PPE_RHITASKNAME
using FTaskName = TStaticString<64>;
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
