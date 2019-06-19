#include "stdafx.h"

#include "Misc/DynamicLibrary.h"

#include "Diagnostic/Logger.h"
#include "IO/String.h"
#include "IO/TextWriter.h"
#include "HAL/PlatformProcess.h"

namespace PPE {
LOG_CATEGORY(, DynamicLib)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Cross platform code
//----------------------------------------------------------------------------
FDynamicLibrary::FDynamicLibrary() {
    _handle.Reset(nullptr, false, false);
}
//----------------------------------------------------------------------------
FDynamicLibrary::~FDynamicLibrary() {
    UnloadIFP();
}
//----------------------------------------------------------------------------
FDynamicLibrary::FDynamicLibrary(FDynamicLibrary&& rvalue) NOEXCEPT {
    _handle.Reset(nullptr, false, false);
    swap(_handle, rvalue._handle);
}
//----------------------------------------------------------------------------
FDynamicLibrary& FDynamicLibrary::operator =(FDynamicLibrary&& rvalue) NOEXCEPT {
    AssertRelease(not IsValid());
    swap(_handle, rvalue._handle);

    return (*this);
}
//----------------------------------------------------------------------------
bool FDynamicLibrary::AttachOrLoad(const wchar_t* path) {
    return (Attach(path) || Load(path));
}
//----------------------------------------------------------------------------
bool FDynamicLibrary::UnloadIFP() {
    if (IsValid()) {
        Unload();
        return true;
    }
    else {
        Assert(nullptr == _handle.Get());
        return false;
    }
}
//----------------------------------------------------------------------------
bool FDynamicLibrary::Attach(const wchar_t* path) {
    AssertRelease(not IsValid());

    if (auto* const hModule = FPlatformProcess::AttachToDynamicLibrary(path)) {
        _handle.Reset(hModule, true, true);

        LOG(DynamicLib, Debug, L"attached to module '{0}'", MakeCStringView(path) );

        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
bool FDynamicLibrary::Load(const wchar_t* path) {
    AssertRelease(not IsValid());

    if (auto* const hModule = FPlatformProcess::OpenDynamicLibrary(path)) {
        _handle.Reset(hModule, true, false);

        LOG(DynamicLib, Debug, L"loaded module '{0}'", MakeCStringView(path) );

        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
void FDynamicLibrary::Unload() {
    AssertRelease(IsValid());

    auto* const hModule = (FPlatformProcess::FDynamicLibraryHandle)_handle.Get();

    if (IsSharedResource()) {
        LOG(DynamicLib, Debug, L"detaching module '{0}'", FPlatformProcess::DynamicLibraryFilename(hModule));

        FPlatformProcess::DetachFromDynamicLibrary(hModule);
    }
    else {
        LOG(DynamicLib, Debug, L"unloading module '{0}'", FPlatformProcess::DynamicLibraryFilename(hModule));

        FPlatformProcess::CloseDynamicLibrary(hModule);
    }

    _handle.Reset(nullptr, false, false);
}
//----------------------------------------------------------------------------
void* FDynamicLibrary::FunctionAddr(const char* funcname) const {
    Assert(IsValid());

    auto* const hModule = (FPlatformProcess::FDynamicLibraryHandle)_handle.Get();

    return FPlatformProcess::DynamicLibraryFunction(hModule, funcname);
}
//----------------------------------------------------------------------------
FWString FDynamicLibrary::ModuleName() const {
    Assert(IsValid());

    auto* const hModule = (FPlatformProcess::FDynamicLibraryHandle)_handle.Get();

    return FPlatformProcess::DynamicLibraryFilename(hModule);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
