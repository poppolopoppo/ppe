#pragma once

#include "HAL/Generic/GenericWindow.h"

#ifndef PLATFORM_LINUX
#   error "invalid include for current platform"
#endif

#include "HAL/PlatformIncludes.h"
#include "HAL/Linux/LinuxPlatformMessageHandler.h"
#include "Misc/Event.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FLinuxWindow : public FGenericWindow {
public: // must be defined for every platform
    using FNativeHandle = void*;
    using FWindowDefinition = FGenericWindow::FWindowDefinition;
    using EWindowType = EGenericWindowType;

    FLinuxWindow();
    virtual ~FLinuxWindow();

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

    static FLinuxWindow* ActiveWindow();
    static void MainWindowDefinition(FWindowDefinition* def);
    static void HiddenWindowDefinition(FWindowDefinition* def);
    static bool CreateWindow(FLinuxWindow* window, FWString&& title, const FWindowDefinition& def);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
