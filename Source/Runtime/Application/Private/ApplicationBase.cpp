#include "stdafx.h"

#include "ApplicationBase.h"

#include "Diagnostic/Logger.h"
#include "Memory/MemoryDomain.h"

namespace PPE {
namespace Application {
EXTERN_LOG_CATEGORY(PPE_APPLICATION_API, Application)
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
} //!namespace PPE
