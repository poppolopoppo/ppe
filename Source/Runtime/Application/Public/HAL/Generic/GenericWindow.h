#pragma once

#include "Application_fwd.h"

#include "IO/String.h"
#include "Time/Timepoint.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGenericWindowType {
    Main,
    Modal,
    Tool,
    Child,
    BorderLess,
};
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FGenericWindowDefinition {
    EGenericWindowType Type;

    int Left;
    int Top;

    size_t Width;
    size_t Height;

    bool AllowDragDrop      : 1;
    bool Centered           : 1;
    bool Maximized          : 1;
    bool Fullscreen         : 1;
    bool HasCloseButton     : 1;
    bool HasResizeButton    : 1;
    bool HasSystemMenu      : 1;

    FGenericWindow* Parent;
};
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FGenericWindow {
public: // must be defined for every platform
    using FNativeHandle = void*;
    using FWindowDefinition = FGenericWindowDefinition;
    using EWindowType = EGenericWindowType;

    virtual ~FGenericWindow();

    FGenericWindow(const FGenericWindow&) = delete;
    FGenericWindow& operator =(const FGenericWindow&) = delete;

    const FWString& Title() const { return _title; }
    FNativeHandle Handle() const { return _handle; }

    int Left() const { return _left; }
    int Top() const { return _top; }

    size_t Width() const { return _width; }
    size_t Height() const { return _height; }

    bool AllowDragDrop() const { return _allowDragDrop; }
    bool HasCloseButton() const { return _hasCloseButton; }
    bool HasResizeButton() const { return _hasResizeButton; }
    bool HasSystemMenu() const { return _hasSystemMenu; }

    bool Fullscreen() const { return _fullscreen; }
    bool HasFocus() const { return _hasFocus; }
    bool Visible() const { return _visible; }
    EWindowType Type() const { return _type; }

    virtual bool Show();
    virtual bool Close();

    virtual bool PumpMessages();

    virtual bool Center();
    virtual bool Maximize();
    virtual bool Minimize();
    virtual bool Move(int x, int y);
    virtual bool Resize(size_t w, size_t h);
    virtual bool SetFocus();
    virtual bool SetFullscreen(bool value);
    virtual bool SetTitle(FWString&& title);

    virtual void ScreenToClient(int* screenX, int* screenY) const = 0;
    virtual void ClientToScreen(int* clientX, int* clientY) const = 0;

    virtual void SetCursorCapture(bool enabled) const = 0;
    virtual void SetCursorOnWindowCenter() const = 0;

    static FGenericWindow* ActiveWindow() = delete;
    static void MainWindowDefinition(FWindowDefinition* def);
    static bool CreateWindow(FGenericWindow* window, FWString&& title, const FWindowDefinition& def);

protected:
    FGenericWindow();

    virtual void OnDragDropFile(const FWStringView& filename);

    virtual void OnFocusSet();
    virtual void OnFocusLose();

    virtual void OnMouseEnter();
    virtual void OnMouseLeave();

    virtual void OnMouseMove(int x, int y);
    virtual void OnMouseClick(int x, int y, EMouseButton btn);
    virtual void OnMouseDoubleClick(int x, int y, EMouseButton btn);
    virtual void OnMouseWheel(int x, int y, int delta);

    virtual void OnWindowShow();
    virtual void OnWindowClose();

    virtual void OnWindowMove(int x, int y);
    virtual void OnWindowResize(size_t w, size_t h);

    virtual void OnWindowPaint();

    virtual void UpdateClientRect() = 0;

protected:
    void SetNativeHandle(FNativeHandle handle) {
        _handle = handle;
    }

private:
    FWString _title;
    FNativeHandle _handle;

    int _left, _top;
    size_t _width, _height;

    bool _allowDragDrop     : 1;
    bool _hasCloseButton    : 1;
    bool _hasResizeButton   : 1;
    bool _hasSystemMenu     : 1;

    bool _fullscreen        : 1;
    bool _hasFocus          : 1;
    bool _visible           : 1;

    EWindowType _type;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
