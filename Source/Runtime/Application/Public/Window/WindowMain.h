#pragma once

#include "Application.h"

#include "Window/WindowBase.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(WindowMain);
class PPE_APPLICATION_API FWindowMain : public FWindowBase {
public:
    explicit FWindowMain(FWString&& title);
    FWindowMain(FWString&& title, size_t width, size_t height);
    FWindowMain(FWString&& title, int left, int top, size_t width, size_t height);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
