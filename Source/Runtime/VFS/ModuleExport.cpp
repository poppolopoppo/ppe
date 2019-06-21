#include "stdafx.h"

#include "ModuleExport.h"

#include "VirtualFileSystem.h"

#include "Diagnostic/CurrentProcess.h"
#include "HAL/PlatformFile.h"
#include "IO/FileSystem.h"

#include "Module-impl.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVirtualFileSystemModule::FVirtualFileSystemModule()
:   FModule("Runtime/VFS")
{}
//----------------------------------------------------------------------------
FVirtualFileSystemModule::~FVirtualFileSystemModule() = default;
//----------------------------------------------------------------------------
void FVirtualFileSystemModule::Start(FModuleManager& manager) {
    FModule::Start(manager);

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
void FVirtualFileSystemModule::Shutdown() {
    FModule::Shutdown();

    FVirtualFileSystem::Destroy();
}
//----------------------------------------------------------------------------
void FVirtualFileSystemModule::ReleaseMemory() {
    FModule::ReleaseMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
