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
    LOG(Info, L"[Application] Start <{0}>", _appname);
}
//----------------------------------------------------------------------------
void ApplicationBase::Shutdown() {
    LOG(Info, L"[Application] Shutdown <{0}>", _appname);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
