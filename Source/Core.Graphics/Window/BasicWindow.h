#pragma once

#include "Core/Core.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Memory/RefPtr.h"

#include "Core.Graphics/Window/WindowMessage.h"
#include "Core.Graphics/Window/WindowMessageHandler.h"

namespace Core {
namespace Graphics {
FWD_REFPTR(BasicWindow);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class BasicWindow : public RefCountable {
public:
    BasicWindow(
        const wchar_t *title,
        int left, int top,
        size_t width, size_t height,
        BasicWindow *parent = nullptr);
    virtual ~BasicWindow();

    void *Handle() const { return _handle; }
    size_t Width() const { return _width; }
    size_t Height() const { return _height; }
    BasicWindow *Parent() const { return _parent; }
    bool HasFocus() const { return _hasFocus; }

    void RegisterMessageHandler(IWindowMessageHandler *handler);
    void UnregisterMessageHandler(IWindowMessageHandler *handler);

    virtual bool DispatchMessageIFP(
        WindowMessage msg,
        MessageLParam lparam, MessageWParam wparam,
        MessageResult *result);

    virtual void Update_BeforeDispatch();
    virtual void Update_AfterDispatch();

    void Show();
    void Close();

    bool PumpMessage(WindowMessage& msg, MessageLParam& lparam, MessageWParam& wparam);

    void ScreenToClient(int *screenX, int *screenY) const;
    void ClientToScreen(int *clientX, int *clientY) const;

    void SetCursorCapture(bool enabled) const;
    void SetCursorPositionOnScreenCenter() const;

    static void Start();
    static void Shutdown();

protected:
    virtual void OnSetFocus();
    virtual void OnLoseFocus();
    virtual void OnResize(size_t width, size_t height);

private:
    typedef Pair<IWindowMessageHandler *, IWindowMessageHandler::Delegate> WindowMessageHandlerDelegate;

    friend void SetWindowHandle_(BasicWindow *wnd, void *handle);
    friend void SetWindowFocus_(BasicWindow *wnd, void *handle, bool hasFocus);
    friend void SetWindowSize_(BasicWindow *wnd, void *handle, size_t width, size_t height);

    friend class IWindowMessageHandler;

    void RegisterMessageDelegate_(WindowMessage msg, IWindowMessageHandler *handler, IWindowMessageHandler::Delegate member);
    void UnregisterMessageDelegate_(WindowMessage msg, IWindowMessageHandler *handler, IWindowMessageHandler::Delegate member);

    void UpdateMessageHandlers_BeforeDispatch_();
    void UpdateMessageHandlers_AfterDispatch_();

    void *_handle;

    size_t _width;
    size_t _height;

    BasicWindow *_parent;

    bool _hasFocus;
    bool _wantFocus;

    size_t _wantedWidth;
    size_t _wantedHeight;

    ASSOCIATIVE_VECTOR(Window, WindowMessage, WindowMessageHandlerDelegate) _dispatch;
    VECTOR(Window, IWindowMessageHandler *) _handlers;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
