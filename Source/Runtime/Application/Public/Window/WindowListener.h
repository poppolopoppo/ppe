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
    virtual void OnWindowFocus(bool enabled) = 0;
    virtual void OnWindowPaint() = 0;
    virtual void OnWindowResize(const uint2& size) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
