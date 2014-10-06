#include "stdafx.h"

#include "WindowMessageHandler.h"

#include "BasicWindow.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void IWindowMessageHandler::RegisterMessageDelegate(BasicWindow *wnd, WindowMessage msg, Delegate member) {
    wnd->RegisterMessageDelegate_(msg, this, member);
}
//----------------------------------------------------------------------------
void IWindowMessageHandler::UnregisterMessageDelegate(BasicWindow *wnd, WindowMessage msg, Delegate member) {
    wnd->UnregisterMessageDelegate_(msg, this, member);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
