#pragma once

#include "Application_fwd.h"

#include "Container/Vector.h"
#include "HAL/PlatformMouse.h"
#include "IO/String.h"
#include "Maths/ScalarVector_fwd.h"
#include "Misc/Event.h"
#include "Thread/DataRaceCheck.h"

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

    bool AutoSize           : 1;
    bool AllowDragDrop      : 1;
    bool Centered           : 1;
    bool Maximized          : 1;
    bool Fullscreen         : 1;
    bool Invisible          : 1;
    bool HasCloseButton     : 1;
    bool HasResizeButton    : 1;
    bool HasSystemMenu      : 1;

    FGenericWindow* Parent;
};
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FGenericWindow : public FRWDataRaceCheckResource {
public: // must be defined for every platform
    using FNativeHandle = void*;
    using FWindowDefinition = FGenericWindowDefinition;
    using EWindowType = EGenericWindowType;

    virtual ~FGenericWindow();

    FGenericWindow(const FGenericWindow&) = delete;
    FGenericWindow& operator =(const FGenericWindow&) = delete;

    const FWString& Title() const { PPE_DATARACE_SHARED_SCOPE(this); return _title; }
    FNativeHandle NativeHandle() const { PPE_DATARACE_SHARED_SCOPE(this); return _handle; }

    int Left() const { PPE_DATARACE_SHARED_SCOPE(this); return _left; }
    int Top() const { PPE_DATARACE_SHARED_SCOPE(this); return _top; }

    size_t Width() const { PPE_DATARACE_SHARED_SCOPE(this); return _width; }
    size_t Height() const { PPE_DATARACE_SHARED_SCOPE(this); return _height; }

    int2 Position() const NOEXCEPT;
    uint2 Dimensions() const NOEXCEPT;

    u32 DPI() const { return _dpi; }

    bool AllowDragDrop() const { PPE_DATARACE_SHARED_SCOPE(this); return _allowDragDrop; }
    bool HasCloseButton() const { PPE_DATARACE_SHARED_SCOPE(this); return _hasCloseButton; }
    bool HasResizeButton() const { PPE_DATARACE_SHARED_SCOPE(this); return _hasResizeButton; }
    bool HasSystemMenu() const { PPE_DATARACE_SHARED_SCOPE(this); return _hasSystemMenu; }

    bool Fullscreen() const { PPE_DATARACE_SHARED_SCOPE(this); return _fullscreen; }
    bool HasFocus() const { PPE_DATARACE_SHARED_SCOPE(this); return _hasFocus; }
    bool Visible() const { PPE_DATARACE_SHARED_SCOPE(this); return _visible; }
    EWindowType Type() const { PPE_DATARACE_SHARED_SCOPE(this); return _type; }

    void AddListener(TPtrRef<IWindowListener>&& listener);
    void RemoveListener(const TPtrRef<IWindowListener>& listener);

    virtual bool Show();
    virtual bool Close();

    virtual bool PumpMessages();

    virtual bool BringToFront();
    virtual bool Center();
    virtual bool Maximize();
    virtual bool Minimize();
    virtual bool Move(int x, int y);
    virtual bool Resize(size_t w, size_t h);
    virtual bool SetFocus();
    virtual bool SetFullscreen(bool value);
    virtual bool SetTitle(FWString&& title);

    virtual void ScreenToClient(int* screenX, int* screenY) const;
    virtual void ClientToScreen(int* clientX, int* clientY) const;

    ECursorType CursorType() const NOEXCEPT { return _cursor; }
    ECursorType SetCursorType(ECursorType value);

    virtual void SetCursorCapture(bool enabled);
    virtual void SetCursorPosition(int clientX, int clientY);
    virtual void SetCursorOnWindowCenter();

    static FGenericWindow* ActiveWindow() = delete;
    static void MainWindowDefinition(FWindowDefinition* def);
    static void HiddenWindowDefinition(FWindowDefinition* def);
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

    virtual void OnWindowShow(bool visible);
    virtual void OnWindowClose();

    virtual void OnWindowDPI(u32 dpi);
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
    FNativeHandle _handle{ nullptr };

    int _left, _top;
    u32 _width, _height;
    u32 _dpi;

    bool _allowDragDrop     : 1;
    bool _hasCloseButton    : 1;
    bool _hasResizeButton   : 1;
    bool _hasSystemMenu     : 1;

    bool _fullscreen        : 1;
    bool _hasFocus          : 1;
    bool _mouseCapture      : 1;
    bool _visible           : 1;

    int _mouseClientX = -1;
    int _mouseClientY = -1;

    ECursorType _cursor{ Default };
    EWindowType _type;

    TThreadSafe<VECTORINSITU(Window, TPtrRef<IWindowListener>, 1), EThreadBarrier::RWDataRaceCheck> _listeners;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
