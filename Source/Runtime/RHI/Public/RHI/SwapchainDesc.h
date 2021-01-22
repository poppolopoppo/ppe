#pragma once

#include "RHI_fwd.h"

#include "RHI/ResourceEnums.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EPresentMode : u32 {
    Immediate = 0,
    Fifo,
    RelaxedFifo,
    Mailbox,
};
//----------------------------------------------------------------------------
struct FSwapchainDesc {
    FWindowHandle Window{ nullptr };
    EPixelFormat SurfaceFormat{ Default };
    EColorSpace ColorSpace{ Default };
    EPresentMode PresentMode{ EPresentMode::Mailbox };
    u32 ImageCount{ 0 };

    FSwapchainDesc() = default;
    FSwapchainDesc(FWindowHandle handle, u32 count) { SetWindow(handle, count); }

    FSwapchainDesc& SetWindow(FWindowHandle handle, u32 count) {
        Assert(handle);
        Assert(count > 0);
        Window = handle;
        ImageCount = count;
        return (*this);
    }

    FSwapchainDesc& SetFormat(EPixelFormat fmt) { SurfaceFormat = fmt; return (*this); }
    FSwapchainDesc& SetColorSpace(EColorSpace space) { ColorSpace = space; return (*this); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
