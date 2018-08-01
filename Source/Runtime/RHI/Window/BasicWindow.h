#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Window/WindowMessage.h"
#include "Core.Graphics/Window/WindowMessageHandler.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/IO/String.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Graphics {
FWD_REFPTR(BasicWindow);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FBasicWindow : public FRefCountable {
public:
    FBasicWindow(   const FWStringView& title,
                    int left, int top,
                    size_t width, size_t height,
                    FBasicWindow *parent = nullptr   );
    virtual ~FBasicWindow();

    FBasicWindow(const FBasicWindow&) = delete;
    FBasicWindow& operator =(const FBasicWindow&) = delete;

    void *Handle() const { return _handle; }
    const FWString& Title() const { return _title; }
    size_t Width() const { return _width; }
    size_t Height() const { return _height; }
    FBasicWindow *Parent() const { return _parent; }
    bool HasFocus() const { return _hasFocus; }

    void RegisterMessageHandler(IWindowMessageHandler *handler);
    void UnregisterMessageHandler(IWindowMessageHandler *handler);

    virtual bool DispatchMessageIFP(
        EWindowMessage msg,
        MessageLParam lparam, MessageWParam wparam,
        MessageResult *result);

    virtual void Update_BeforeDispatch();
    virtual void Update_AfterDispatch();

    void Show();
    void Close();

    bool PumpMessage(EWindowMessage& msg, MessageLParam& lparam, MessageWParam& wparam);
    bool PumpAllMessages_ReturnIfQuit();

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
    typedef TPair<IWindowMessageHandler *, IWindowMessageHandler::TDelegate> WindowMessageHandlerDelegate;

    friend struct FBasicWindowHelper;
    friend class IWindowMessageHandler;

    void RegisterMessageDelegate_(EWindowMessage msg, IWindowMessageHandler *handler, IWindowMessageHandler::TDelegate member);
    void UnregisterMessageDelegate_(EWindowMessage msg, IWindowMessageHandler *handler, IWindowMessageHandler::TDelegate member);

    void UpdateMessageHandlers_BeforeDispatch_();
    void UpdateMessageHandlers_AfterDispatch_();

    void *_handle;

    FWString _title;

    size_t _width;
    size_t _height;

    FBasicWindow *_parent;

    bool _hasFocus;
    bool _wantFocus;

    size_t _wantedWidth;
    size_t _wantedHeight;

    ASSOCIATIVE_VECTOR(Window, EWindowMessage, WindowMessageHandlerDelegate) _dispatch;
    VECTOR(Window, IWindowMessageHandler *) _handlers;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
