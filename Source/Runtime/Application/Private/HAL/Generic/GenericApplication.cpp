#include "stdafx.h"

#include "HAL/Generic/GenericApplication.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformMessageHandler.h"
#include "HAL/PlatformTime.h"

namespace PPE {
namespace Application {
EXTERN_LOG_CATEGORY(PPE_APPLICATION_API, Application)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FGenericApplication::FGenericApplication(FWString&& name)
:   _name(std::move(name)) {
    Assert(not _name.empty());
}
//----------------------------------------------------------------------------
FGenericApplication::~FGenericApplication() = default;
//----------------------------------------------------------------------------
void FGenericApplication::Start() {
    LOG(Application, Emphasis, L"start application <{0}>", _name);

    FPlatformTime::EnterHighResolutionTimer();
}
//----------------------------------------------------------------------------
void FGenericApplication::PumpMessages() {
    FPlatformMessageHandler::PumpMessages(nullptr);
}
//----------------------------------------------------------------------------
void FGenericApplication::Tick(FTimespan ) {

}
//----------------------------------------------------------------------------
void FGenericApplication::Shutdown() {
    LOG(Application, Emphasis, L"shutdown application <{0}>", _name);

    FPlatformTime::LeaveLowResolutionTimer();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
