#include "stdafx.h"

#include "VFSModule.h"

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
FVFSModule::FVFSModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
{}
//----------------------------------------------------------------------------
void FVFSModule::Start(FModularDomain& domain) {
    IModuleInterface::Start(domain);

    FVirtualFileSystem::Create();

    auto& vfs = FVirtualFileSystem::Get();
    auto& process = FCurrentProcess::Get();

    // current process executable directory
    vfs.MountNativePath(L"Process:/", process.Directory());

    // data directory
    vfs.MountNativePath(MakeStringView(L"Data:/"), process.DataPath());

    // saved directory
    vfs.MountNativePath(MakeStringView(L"Saved:/"), process.SavedPath());

    // current process working directory
    vfs.MountNativePath(L"Working:/", FPlatformFile::WorkingDirectory());

    // system temporary path
    vfs.MountNativePath(L"Tmp:/", FPlatformFile::TemporaryDirectory());

    // user profile path
    vfs.MountNativePath(L"User:/", FPlatformFile::UserDirectory());

    // system path
    vfs.MountNativePath(L"System:/", FPlatformFile::SystemDirectory());
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
