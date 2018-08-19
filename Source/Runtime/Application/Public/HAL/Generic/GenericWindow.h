#pragma once

#include "Application_fwd.h"

#include "HAL/PlatformMessageHandler.h"

#include "IO/String.h"
#include "Memory/RefPtr.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(GenericWindow);
//----------------------------------------------------------------------------
enum class EGenericWindowType {
    Default,
    Modal,
    Tool,
    BorderLess,
};
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FGenericWindowDefinition {
    FWStringView Title;
    EGenericWindowType Type;
    int Left;
    int Top;
    size_t Width;
    size_t Height;
    bool AllowDragDrop : 1;
    bool Centered : 1;
    bool Fullscreen : 1;
    bool HasCloseButton : 1;
    bool HasResizeButton : 1;
};
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FGenericWindow : public FRefCountable {
public: // must be defined for every platform
    using FNativeHandle = void*;
    using FWindowDefinition = FGenericWindowDefinition;
    using EWindowType = EGenericWindowType;

    virtual ~FGenericWindow() {}

    FGenericWindow(const FGenericWindow&) = delete;
    FGenericWindow& operator =(const FGenericWindow&) = delete;

    const FWString& Title() const { return _title; }
    FNativeHandle Handle() const { return _handle; }
    int Left() const { return _left; }
    int Top() const { return _top; }
    size_t Width() const { return _width; }
    size_t Height() const { return _height; }
    bool AllowDragDrop() const { return _allowDragDrop; }
    bool Fullscreen() const { return _fullscreen; }
    bool HasFocus() const { return _hasFocus; }
    bool HasCloseButton() const { return _hasCloseButton; }
    bool HasResizeButton() const { return _hasResizeButton; }
    bool Visible() const { return _visible; }
    EWindowType Type() const { return _type; }

    virtual void Show();
    virtual void Close();

    virtual void PumpMessages(FTimespan dt);

    virtual bool Move(int x, int y);
    virtual bool Resize(int w, int h, bool fullscreen = false);
    virtual bool SetFullscreen(bool value);

    virtual void ScreenToClient(int* screenX, int* screenY) const;
    virtual void ClientToScreen(int* clientX, int* clientY) const;

    virtual void SetCursorCapture(bool enabled) const;
    virtual void SetCursorPositionOnScreenCenter() const;

    virtual void SetTitle(const FWStringView& title) const;

    static bool CreateWindow(PGenericWindow* pwin, const FWindowDefinition& def) = delete;

protected:
    explicit FGenericWindow(const FGenericWindowDefinition& def);

    virtual void OnPaint() {}

    virtual void OnDragEnter() {}
    virtual void OnDragDrop() {}
    virtual void OnDragLeave() {}

    virtual void OnFocusSet() {}
    virtual void OnFocusLose() {}

    virtual void OnMouseEnter() {}
    virtual void OnMouseLeave() {}

    virtual void OnMouseMove(int x, int y) {}
    virtual void OnMouseClick(int x, int y, EMouseButton btn) {}
    virtual void OnMouseDoubleClick(int x, int y, EMouseButton btn) {}
    virtual void OnMouseWheel(int x, int y, int delta) {}

    virtual void OnWindowShow() {}
    virtual void OnWindowHide() {}

    virtual void OnWindowMove(int x, int y) {}
    virtual void OnWindowResize(int w, int h) {}

private:
    using FDeferredMessages = FPlatformMessageHandler::FDeferredMessages;

    FWString _title;
    FNativeHandle _handle;

    int _left, _top;
    size_t _width, _height;

    bool _allowDragDrop : 1;
    bool _fullscreen : 1;
    bool _hasCloseButton : 1;
    bool _hasFocus : 1;
    bool _hasResizeButton : 1;
    bool _visible : 1;

    FDeferredMessages _messages;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
