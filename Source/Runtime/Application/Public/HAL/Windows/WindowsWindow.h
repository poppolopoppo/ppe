#pragma once

#include "HAL/Generic/GenericWindow.h"

#ifndef PLATFORM_WINDOWS
#   error "invalid include for current platform"
#endif

#include "HAL/PlatformIncludes.h"
#include "HAL/Windows/WindowsPlatformMessageHandler.h"
#include "Misc/Event.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FWindowsWindow : public FGenericWindow {
public: // must be defined for every platform
    using FNativeHandle = ::HWND;
    using FWindowDefinition = FGenericWindow::FWindowDefinition;
    using EWindowType = EGenericWindowType;

    FWindowsWindow();
    virtual ~FWindowsWindow();

    virtual bool Show() override final;
    virtual bool Close() override final;

    virtual bool PumpMessages() override final;

    virtual bool Center() override final;
    virtual bool Maximize() override final;
    virtual bool Minimize() override final;
    virtual bool Move(int x, int y) override final;
    virtual bool Resize(size_t w, size_t h) override final;
    virtual bool SetFocus() override final;
    virtual bool SetFullscreen(bool value) override final;
    virtual bool SetTitle(FWString&& title) override final;

    virtual void ScreenToClient(int* screenX, int* screenY) const override final;
    virtual void ClientToScreen(int* clientX, int* clientY) const override final;

    virtual void SetCursorCapture(bool enabled) const override final;
    virtual void SetCursorOnWindowCenter() const override final;

    static FWindowsWindow* ActiveWindow();
    static void MainWindowDefinition(FWindowDefinition* def);
    static bool CreateWindow(FWindowsWindow* window, FWString&& title, const FWindowDefinition& def);

public: // platform specific
    FNativeHandle HandleWin32() const {
        return reinterpret_cast<FNativeHandle>(FGenericWindow::Handle());
    }

    void SetHandleWin32(::HWND hWnd) { SetNativeHandle(hWnd); }
    bool WindowProcWin32(::UINT msg, ::WPARAM wParam, ::LPARAM lParam);

    static void Start();
    static void Shutdown();

    friend struct FWindowsPlatformMessageHandler;
    PUBLIC_EVENT(OnMessageWin32, FWindowsPlatformMessageHandler::FMessageHandler)

private:
    using parent_type = FGenericWindow;

    virtual void UpdateClientRect() override final;

    bool DispatchEventWin32(::UINT msg, ::WPARAM wParam, ::LPARAM lParam);
    void DragDropProcWin32(::WPARAM wParam);
    void PaintProcWin32();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
