#pragma once

#include "HAL/Generic/GenericWindow.h"

#ifndef PLATFORM_GLFW
#   error "invalid include for current platform"
#endif

#include "HAL/PlatformIncludes.h"
#include "HAL/GLFW/GLFWPlatformMessageHandler.h"
#include "Misc/Event.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FGLFWWindow : public FGenericWindow {
public: // must be defined for every platform
    using FNativeHandle = void*;
    using FWindowDefinition = FGenericWindow::FWindowDefinition;
    using EWindowType = EGenericWindowType;

    FGLFWWindow();
    virtual ~FGLFWWindow();

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

    static FGLFWWindow* ActiveWindow();
    static void MainWindowDefinition(FWindowDefinition* def);
    static void HiddenWindowDefinition(FWindowDefinition* def);
    static bool CreateWindow(FGLFWWindow* window, FWString&& title, const FWindowDefinition& def);

public: // platform specific
    PUBLIC_EVENT(OnKeyboardEvent, FGLFWMessageKeyboardEvent);
    PUBLIC_EVENT(OnMouseEvent, FGLFWMessageMouseEvent);

    static FNativeHandle ActiveNativeHandle();

    struct FKeyboardCallbacks;
    struct FMouseCallbacks;
    struct FWindowCallbacks;

protected:
    void OnFocusSet() override;
    void OnFocusLose() override;

    void SendEvent(const FGLFWMessageKeyboard& keyboard);
    void SendEvent(const FGLFWMessageMouse& mouse);

    virtual void UpdateClientRect() override final;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
