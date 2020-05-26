#include "stdafx.h"

#include "WindowMessageHandler.h"

#include "BasicWindow.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void IWindowMessageHandler::RegisterMessageDelegate(FBasicWindow *wnd, EWindowMessage msg, TDelegate member) {
    wnd->RegisterMessageDelegate_(msg, this, member);
}
//----------------------------------------------------------------------------
void IWindowMessageHandler::UnregisterMessageDelegate(FBasicWindow *wnd, EWindowMessage msg, TDelegate member) {
    wnd->UnregisterMessageDelegate_(msg, this, member);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
