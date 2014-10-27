#include "stdafx.h"

#include "ApplicationBase.h"

#include "Core/Diagnostic/Logger.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ApplicationBase::ApplicationBase(const wchar_t *appname)
:   _appname(appname) {}
//----------------------------------------------------------------------------
ApplicationBase::~ApplicationBase() {}
//----------------------------------------------------------------------------
void ApplicationBase::Start() {
    LOG(Information, L"[Application] Start <{0}>", _appname);

    _services.Start();
}
//----------------------------------------------------------------------------
void ApplicationBase::Shutdown() {
    LOG(Information, L"[Application] Shutdown <{0}>", _appname);

    _services.Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
