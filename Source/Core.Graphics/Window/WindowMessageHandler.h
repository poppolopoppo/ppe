#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Window/WindowMessage.h"

namespace Core {
namespace Graphics {
class BasicWindow;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IWindowMessageHandler {
public:
    IWindowMessageHandler() : _window(nullptr) {}
    virtual ~IWindowMessageHandler() {}

    const BasicWindow *Window() const { return _window; }
    void SetWindow(const BasicWindow *wnd) { _window = wnd; }

    virtual void RegisterMessageDelegates(BasicWindow *wnd) = 0;
    virtual void UnregisterMessageDelegates(BasicWindow *wnd) = 0;

    virtual void UpdateBeforeDispatch(BasicWindow *wnd) = 0;
    virtual void UpdateAfterDispatch(BasicWindow *wnd) = 0;

    typedef MessageResult (*Delegate)(
        IWindowMessageHandler *handler,
        BasicWindow *wnd,
        WindowMessage msg,
        MessageLParam lparam,
        MessageWParam wparam );

protected:
    void RegisterMessageDelegate(BasicWindow *wnd, WindowMessage msg, Delegate member);
    void UnregisterMessageDelegate(BasicWindow *wnd, WindowMessage msg, Delegate member);

private:
    const BasicWindow *_window;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
