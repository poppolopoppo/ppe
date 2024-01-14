#pragma once

#include "Application_fwd.h"

#include "Maths/ScalarVector_fwd.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API IWindowListener {
public:
    virtual void OnWindowShow(bool visible) { Unused(visible); }
    virtual void OnWindowDPI(u32 dpi) NOEXCEPT { Unused(dpi); }
    virtual void OnWindowFocus(bool enabled) NOEXCEPT { Unused(enabled); }
    virtual void OnWindowMove(const int2& pos) NOEXCEPT { Unused(pos); }
    virtual void OnWindowPaint() NOEXCEPT {}
    virtual void OnWindowResize(const uint2& size) NOEXCEPT { Unused(size); }
    virtual void OnWindowClose() {}
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
