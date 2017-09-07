#include "stdafx.h"

#include "ApplicationBase.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/Memory/MemoryDomain.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FApplicationBase::FApplicationBase(const wchar_t *appname)
:   _appname(appname) {}
//----------------------------------------------------------------------------
FApplicationBase::~FApplicationBase() {}
//----------------------------------------------------------------------------
void FApplicationBase::Start() {
    LOG(Info, L"[Application] Start <{0}>", _appname);
    ReportAllTrackingData();
}
//----------------------------------------------------------------------------
void FApplicationBase::Shutdown() {
    ReportAllTrackingData();
    LOG(Info, L"[Application] Shutdown <{0}>", _appname);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
