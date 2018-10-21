#include "stdafx.h"

#include "Module.h"

#include "ModuleManager.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FModule::FModule(const char* name)
:   _name(name)
,   _status(EModuleStatus::Initialized) {
}
//----------------------------------------------------------------------------
FModule::~FModule() {
    AssertRelease(EModuleStatus::Destroyed == _status);
}
//----------------------------------------------------------------------------
const char* FModule::Name() const { return _name; }
//----------------------------------------------------------------------------
EModuleStatus FModule::Status() const { return _status; }
//----------------------------------------------------------------------------
void FModule::Start(FModuleManager& manager) {
    AssertRelease(EModuleStatus::Initialized == _status);

    UNUSED(manager);

    _status = EModuleStatus::Started;
}
//----------------------------------------------------------------------------
void FModule::Shutdown() {
    AssertRelease(EModuleStatus::Started == _status);

    _status = EModuleStatus::Destroyed;
}
//----------------------------------------------------------------------------
void FModule::ReleaseMemory() {
    AssertRelease(EModuleStatus::Started == _status);

}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBaseModuleStartup::FBaseModuleStartup(PPE::FModuleManager& manager)
    : Manager(manager) {
    Manager.PreInit(*this);
}
//----------------------------------------------------------------------------
FBaseModuleStartup::~FBaseModuleStartup() {
    Manager.PostDestroy(*this);
}
//----------------------------------------------------------------------------
void FBaseModuleStartup::ReleaseMemory() {
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
