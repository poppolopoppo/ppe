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
void FModule::Start() {
    AssertRelease(EModuleStatus::Initialized == _status);

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
void FBaseModuleStartup::Start(FModuleManager& manager) {
    manager.PreInit(*this);
}
//----------------------------------------------------------------------------
void FBaseModuleStartup::Shutdown(FModuleManager& manager) {
    manager.PostDestroy(*this);
}
//----------------------------------------------------------------------------
void FBaseModuleStartup::ReleaseMemory(FModuleManager&) {
}
//----------------------------------------------------------------------------
void FBaseModuleStartup::StartModule(FModuleManager& manager, FModule& m) {
    manager.Start(m);
}
//----------------------------------------------------------------------------
void FBaseModuleStartup::ShutdownModule(FModuleManager& manager, FModule& m) {
    manager.Shutdown(m);
}
//----------------------------------------------------------------------------
void FBaseModuleStartup::ReleaseMemoryInModule(FModuleManager& manager, FModule& m) {
    manager.ReleaseMemory(m);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
