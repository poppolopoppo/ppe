#include "stdafx.h"

#include "RTTI/Service.h"

#include "MetaModule.h"

#include "Container/AssociativeVector.h"
#include "Diagnostic/Logger.h"
#include "Modular/ModuleInterface.h"
#include "Thread/ReadWriteLock.h"

namespace PPE {
namespace RTTI {
EXTERN_LOG_CATEGORY(PPE_RTTI_API, RTTI)
} //!namespace RTTI
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FDefaultRTTIService_ final : public IRTTIService {
public:
    FDefaultRTTIService_() = default;

#if USE_PPE_ASSERT
    ~FDefaultRTTIService_() {
        const FReadWriteLock::FScopeLockRead readLock(_barrierRW);
        Assert_NoAssume(_modules.empty());
    }
#endif

    virtual TPtrRef<const RTTI::FMetaModule> Module(TPtrRef<const IModuleInterface> modular) const NOEXCEPT override;

    virtual void RegisterModule(TPtrRef<const IModuleInterface> modular, TPtrRef<const RTTI::FMetaModule> rtti) override;
    virtual void UnregisterModule(TPtrRef<const IModuleInterface> modular, TPtrRef<const RTTI::FMetaModule> rtti) override;

private:
    FReadWriteLock _barrierRW;
    ASSOCIATIVE_VECTOR(Modular, const IModuleInterface*, const RTTI::FMetaModule*) _modules;
};
//----------------------------------------------------------------------------
TPtrRef<const RTTI::FMetaModule> FDefaultRTTIService_::Module(TPtrRef<const IModuleInterface> modular) const NOEXCEPT {
    Assert(modular);

    const FReadWriteLock::FScopeLockRead readLock(_barrierRW);
    return _modules.GetIFP(modular);
}
//----------------------------------------------------------------------------
void FDefaultRTTIService_::RegisterModule(TPtrRef<const IModuleInterface> modular, TPtrRef<const RTTI::FMetaModule> rtti) {
    Assert(modular);
    Assert(rtti);

    using namespace RTTI;

    LOG(RTTI, Info, L"register RTTI module <{0}> for application module <{1}>",
        modular->Name(), rtti->Name() );

    const FReadWriteLock::FScopeLockWrite writeLock(_barrierRW);
    _modules.Insert_AssertUnique(modular, rtti);
}
//----------------------------------------------------------------------------
void FDefaultRTTIService_::UnregisterModule(TPtrRef<const IModuleInterface> modular, TPtrRef<const RTTI::FMetaModule> rtti) {
    Assert(modular);
    Assert(rtti);

    using namespace RTTI;

    LOG(RTTI, Info, L"unregister RTTI module <{0}> for application module <{1}>",
        modular->Name(), rtti->Name() );

    const FReadWriteLock::FScopeLockWrite writeLock(_barrierRW);
    _modules.Remove_AssertExists(modular, rtti);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void IRTTIService::MakeDefault(TUniquePtr<IRTTIService>* service) {
    Assert(service);
    service->reset<FDefaultRTTIService_>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
