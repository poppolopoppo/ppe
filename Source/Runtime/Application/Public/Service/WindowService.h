#pragma once

#include "Application.h"

#include "Memory/RefPtr.h"
#include "Memory/UniquePtr.h"
#include "Misc/Function_fwd.h"

namespace PPE {
namespace Application {
FWD_REFPTR(WindowBase);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTEFARCE_UNIQUEPTR(WindowService);
class IWindowService {
public:
    IWindowService() = default;
    virtual ~IWindowService() = default;

    virtual void CreateMainWindow(PWindowBare* window, FWString&& title) = 0;
    virtual void CreateMainWindow(PWindowBare* window, FWString&& title, size_t width, size_t height) = 0;
    virtual void CreateMainWindow(PWindowBare* window, FWString&& title, int left, int top, size_t width, size_t height) = 0;

    virtual void CreateRHIWindow(PWindowRHI* window, FWString&& title) = 0;
    virtual void CreateRHIWindow(PWindowRHI* window, FWString&& title, size_t width, size_t height) = 0;
    virtual void CreateRHIWindow(PWindowRHI* window, FWString&& title, int left, int top, size_t width, size_t height) = 0;

    virtual FWindowBase* MainWindow() const NOEXCEPT = 0;
    virtual void SetMainWindow(FWindowBase* window) = 0;

    virtual void NotifySystrayNone(const FWStringView& title, const FWStringView& text) = 0;
    virtual void NotifySystrayInfo(const FWStringView& title, const FWStringView& text) = 0;
    virtual void NotifySystrayWarning(const FWStringView& title, const FWStringView& text) = 0;
    virtual void NotifySystrayError(const FWStringView& title, const FWStringView& text) = 0;

    using FSystrayDelegate = TFunction<void()>;

    virtual size_t AddSystrayCommand(
        const FWStringView& category,
        const FWStringView& label,
        FSystrayDelegate&& cmd ) = 0;
    virtual bool RemoveSystrayCommand(size_t userCmd) = 0;

    virtual void SetTaskbarStateNormal() = 0;
    virtual void SetTaskbarStatePaused() = 0;
    virtual void SetTaskbarStateError() = 0;
    virtual void SetTaskbarStateIndeterminate() = 0;

    virtual void BeginTaskbarProgress() = 0;
    virtual void SetTaskbarProgress(size_t completed, size_t total) = 0;
    virtual void EndTaskbarProgress() = 0;

public:
    static PPE_APPLICATION_API void MakeDefault(UWindowService* window);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
