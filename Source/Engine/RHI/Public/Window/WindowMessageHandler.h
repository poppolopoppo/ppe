#pragma once

#include "Graphics.h"

#include "Window/WindowMessage.h"

namespace PPE {
namespace Graphics {
class FBasicWindow;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IWindowMessageHandler {
public:
    IWindowMessageHandler() : _window(nullptr) {}
    virtual ~IWindowMessageHandler() {}

    const FBasicWindow *Window() const { return _window; }
    void SetWindow(const FBasicWindow *wnd) { _window = wnd; }

    virtual void RegisterMessageDelegates(FBasicWindow *wnd) = 0;
    virtual void UnregisterMessageDelegates(FBasicWindow *wnd) = 0;

    virtual void UpdateBeforeDispatch(FBasicWindow *wnd) = 0;
    virtual void UpdateAfterDispatch(FBasicWindow *wnd) = 0;

    typedef MessageResult (*TDelegate)(
        IWindowMessageHandler *handler,
        FBasicWindow *wnd,
        EWindowMessage msg,
        MessageLParam lparam,
        MessageWParam wparam );

protected:
    void RegisterMessageDelegate(FBasicWindow *wnd, EWindowMessage msg, TDelegate member);
    void UnregisterMessageDelegate(FBasicWindow *wnd, EWindowMessage msg, TDelegate member);

private:
    const FBasicWindow *_window;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
