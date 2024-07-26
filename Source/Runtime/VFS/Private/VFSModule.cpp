// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "VFSModule.h"

#include "Modular/ModularDomain.h"
#include "Modular/ModuleRegistration.h"

#include "VirtualFileSystem.h"
#include "VirtualFileSystemTrie.h"

#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformFile.h"
#include "IO/FileSystem.h"

#include "BuildModules.generated.h"
#include "Diagnostic/BuildVersion.h"

namespace PPE {
LOG_CATEGORY(PPE_VFS_API, VFS)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FModuleInfo FVFSModule::StaticInfo{
    FModuleStaticRegistration::MakeInfo<FVFSModule>(
        STRINGIZE(BUILD_TARGET_NAME),
        EModulePhase::Bare,
        EModuleUsage::Runtime,
        EModuleSource::Core,
        BUILD_TARGET_ORDINAL,
        Generated::DependencyList,
        CurrentBuildVersion() )
};
//----------------------------------------------------------------------------
FVFSModule& FVFSModule::Get(const FModularDomain& domain) {
    return domain.ModuleChecked<FVFSModule>(STRINGIZE(BUILD_TARGET_NAME));
}
//----------------------------------------------------------------------------
FVFSModule::FVFSModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
{}
//----------------------------------------------------------------------------
FMountingPoint FVFSModule::DataDir() const {
    ONE_TIME_INITIALIZE(const FMountingPoint, gMountingPoint, L"Data:"_view);
    return gMountingPoint;
}
//----------------------------------------------------------------------------
FMountingPoint FVFSModule::ProcessDir() const {
    ONE_TIME_INITIALIZE(const FMountingPoint, gMountingPoint, L"Process:"_view);
    return gMountingPoint;
}
//----------------------------------------------------------------------------
FMountingPoint FVFSModule::SavedDir() const {
    ONE_TIME_INITIALIZE(const FMountingPoint, gMountingPoint, L"Saved:"_view);
    return gMountingPoint;
}
//----------------------------------------------------------------------------
FMountingPoint FVFSModule::SystemDir() const {
    ONE_TIME_INITIALIZE(const FMountingPoint, gMountingPoint, L"System:"_view);
    return gMountingPoint;
}
//----------------------------------------------------------------------------
FMountingPoint FVFSModule::TmpDir() const {
    ONE_TIME_INITIALIZE(const FMountingPoint, gMountingPoint, L"Tmp:"_view);
    return gMountingPoint;
}
//----------------------------------------------------------------------------
FMountingPoint FVFSModule::UserDir() const {
    ONE_TIME_INITIALIZE(const FMountingPoint, gMountingPoint, L"User:"_view);
    return gMountingPoint;
}
//----------------------------------------------------------------------------
FMountingPoint FVFSModule::WorkingDir() const {
    ONE_TIME_INITIALIZE(const FMountingPoint, gMountingPoint, L"Working:"_view);
    return gMountingPoint;
}
//----------------------------------------------------------------------------
void FVFSModule::Start(FModularDomain& domain) {
    IModuleInterface::Start(domain);

    FVirtualFileSystem::Create();

    auto& vfs = FVirtualFileSystem::Get();
    auto& process = FCurrentProcess::Get();

    vfs.MountNativePath(FDirpath(DataDir()), process.DataPath());
    vfs.MountNativePath(FDirpath(ProcessDir()), process.Directory());
    vfs.MountNativePath(FDirpath(SavedDir()), process.SavedPath());
    vfs.MountNativePath(FDirpath(SystemDir()), FPlatformFile::SystemDirectory());
    vfs.MountNativePath(FDirpath(TmpDir()), FPlatformFile::TemporaryDirectory());
    vfs.MountNativePath(FDirpath(UserDir()), FPlatformFile::UserDirectory());
    vfs.MountNativePath(FDirpath(WorkingDir()), FPlatformFile::WorkingDirectory());
}
//----------------------------------------------------------------------------
void FVFSModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);

    FVirtualFileSystem::Destroy();
}
//----------------------------------------------------------------------------
void FVFSModule::DutyCycle(FModularDomain& domain) {
    IModuleInterface::DutyCycle(domain);

}
//----------------------------------------------------------------------------
void FVFSModule::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    IModuleInterface::ReleaseMemory(domain);

}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
