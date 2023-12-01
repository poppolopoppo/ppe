// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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
FDynamicLibrary::FDynamicLibrary() NOEXCEPT {
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
    AssertRelease_NoAssume(not IsValid());
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
    AssertRelease_NoAssume(not IsValid());

    if (auto* const hModule = FPlatformProcess::AttachToDynamicLibrary(path)) {
        _handle.Reset(hModule, true, true);

        PPE_LOG(DynamicLib, Debug, "attached to module '{0}'", MakeCStringView(path) );

        _OnAttachLibrary.Invoke(*this, path);

        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
bool FDynamicLibrary::Load(const wchar_t* path) {
    AssertRelease_NoAssume(not IsValid());

    if (auto* const hModule = FPlatformProcess::OpenDynamicLibrary(path)) {
        _handle.Reset(hModule, true, false);

        PPE_LOG(DynamicLib, Debug, "loaded module '{0}'", MakeCStringView(path) );

        _OnLoadLibrary.Invoke(*this, path);

        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
void FDynamicLibrary::Unload() {
    AssertRelease_NoAssume(IsValid());

    auto* const hModule = (FPlatformProcess::FDynamicLibraryHandle)_handle.Get();

    _OnUnloadLibrary.Invoke(*this);

    if (IsSharedResource()) {
        PPE_LOG(DynamicLib, Debug, "detaching module '{0}'", FPlatformProcess::DynamicLibraryFilename(hModule));

        FPlatformProcess::DetachFromDynamicLibrary(hModule);
    }
    else {
        PPE_LOG(DynamicLib, Debug, "unloading module '{0}'", FPlatformProcess::DynamicLibraryFilename(hModule));

        FPlatformProcess::CloseDynamicLibrary(hModule);
    }

    _handle.Reset(nullptr, false, false);
}
//----------------------------------------------------------------------------
void* FDynamicLibrary::FunctionAddr(const char* funcname) const {
    Assert_NoAssume(IsValid());

    auto* const hModule = (FPlatformProcess::FDynamicLibraryHandle)_handle.Get();

    return FPlatformProcess::DynamicLibraryFunction(hModule, funcname);
}
//----------------------------------------------------------------------------
FWString FDynamicLibrary::ModuleName() const {
    Assert_NoAssume(IsValid());

    auto* const hModule = (FPlatformProcess::FDynamicLibraryHandle)_handle.Get();

    return FPlatformProcess::DynamicLibraryFilename(hModule);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
