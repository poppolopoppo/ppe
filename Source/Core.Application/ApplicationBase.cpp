#include "stdafx.h"

#include "ApplicationBase.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/Memory/MemoryDomain.h"

namespace Core {
namespace Application {
EXTERN_LOG_CATEGORY(CORE_APPLICATION_API, Application);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FApplicationBase::FApplicationBase(const wchar_t *appname)
    : _appname(MakeCStringView(appname)) {}
//----------------------------------------------------------------------------
FApplicationBase::~FApplicationBase() {}
//----------------------------------------------------------------------------
void FApplicationBase::Start() {
    LOG(Application, Debug, L"start application <{0}>", _appname);
    ReportAllTrackingData();
}
//----------------------------------------------------------------------------
void FApplicationBase::Shutdown() {
    ReportAllTrackingData();
    LOG(Application, Debug, L"shutdown application <{0}>", _appname);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
