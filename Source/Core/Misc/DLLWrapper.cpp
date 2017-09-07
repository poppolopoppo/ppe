#include "stdafx.h"

#include "DLLWrapper.h"

#include "Diagnostic/Logger.h"

#ifdef PLATFORM_WINDOWS

#include "Platform_Windows.h"

namespace Core {
namespace {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static void* DLLWrapper_Attach_(const FStringView& path) {
    Assert('\0' == path.data()[path.size()]);
    return ::GetModuleHandleA(path.data());
}
//----------------------------------------------------------------------------
static void* DLLWrapper_Load_(const FStringView& path) {
    Assert('\0' == path.data()[path.size()]);
    return ::LoadLibraryA(path.data());
}
//----------------------------------------------------------------------------
static void DLLWrapper_Unload_(void* handle) {
    ::FreeLibrary((::HMODULE)handle);
}
//----------------------------------------------------------------------------
static void* DLLWrapper_FunctionAddr_(void* handle, const FStringView& funcname) {
    Assert('\0' == funcname.data()[funcname.size()]);
    return ::GetProcAddress((::HMODULE)handle, funcname.data());
}
//----------------------------------------------------------------------------
static size_t DLLWrapper_LibaryFilename_(void* handle, const TMemoryView<char>& buffer) {
    return checked_cast<size_t>(::GetModuleFileNameA((::HMODULE)handle, buffer.data(), checked_cast<::DWORD>(buffer.size())) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace
} //!namespace Core

#else
#   error "unsupported platform !"
#endif

#ifdef USE_DEBUG_LOGGER
namespace Core {
namespace {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FDLLWrapperFilename_ {
    char Filename[512];
    size_t Length = 0;
    FDLLWrapperFilename_(const FDLLWrapper& wrapper) {
        Length = DLLWrapper_LibaryFilename_(wrapper.Handle(), Filename);
    }
    template <typename _Char, typename _Traits>
    inline friend std::basic_ostream<_Char, _Traits>& operator <<(
        std::basic_ostream<_Char, _Traits>& oss,
        const FDLLWrapperFilename_& wrapperFilename) {
        return (oss << FStringView(wrapperFilename.Filename, wrapperFilename.Length));
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace
} //!namespace Core
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Cross platform code
//----------------------------------------------------------------------------
FDLLWrapper::FDLLWrapper() {
    _handle.Reset(nullptr, false, false);
}
//----------------------------------------------------------------------------
FDLLWrapper::~FDLLWrapper() {
    UnloadIFP();
}
//----------------------------------------------------------------------------
FDLLWrapper::FDLLWrapper(FDLLWrapper&& rvalue) {
    _handle.Reset(nullptr, false, false);
    swap(_handle, rvalue._handle);
}
//----------------------------------------------------------------------------
FDLLWrapper& FDLLWrapper::operator =(FDLLWrapper&& rvalue) {
    AssertRelease(not IsValid());
    swap(_handle, rvalue._handle);

    return (*this);
}
//----------------------------------------------------------------------------
bool FDLLWrapper::AttachOrLoad(const FStringView& path) {
    return (Attach(path) || Load(path));
}
//----------------------------------------------------------------------------
bool FDLLWrapper::UnloadIFP() {
    if (IsValid() && not IsSharedResource()) {
        Unload();
        return true;
    }
    else {
        _handle.Reset(nullptr, false, false);
        return false;
    }
}
//----------------------------------------------------------------------------
bool FDLLWrapper::Attach(const FStringView& path) {
    AssertRelease(not IsValid());

    if (void* const handle = DLLWrapper_Attach_(path)) {
        _handle.Reset(handle, true, true);

        LOG(Debug, L"[DLL] Attached to module '{0}'", FDLLWrapperFilename_(*this));

        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
bool FDLLWrapper::Load(const FStringView& path) {
    AssertRelease(not IsValid());

    if (void* const handle = DLLWrapper_Load_(path)) {
        _handle.Reset(handle, true, false);

        LOG(Debug, L"[DLL] Loaded module '{0}'", FDLLWrapperFilename_(*this));

        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
void FDLLWrapper::Unload() {
    AssertRelease(IsValid());
    AssertRelease(not IsSharedResource());

    LOG(Debug, L"[DLL] Unloading module '{0}'", FDLLWrapperFilename_(*this));

    DLLWrapper_Unload_(_handle.Get());

    _handle.Reset(nullptr, false, false);
}
//----------------------------------------------------------------------------
void* FDLLWrapper::FunctionAddr(const FStringView& funcname) {
    Assert(IsValid());

    return DLLWrapper_FunctionAddr_(_handle.Get(), funcname);
}
//----------------------------------------------------------------------------
bool FDLLWrapper::LibaryFilename(const TMemoryView<char>& buffer, FStringView* filename) {
    Assert(IsValid());
    Assert(filename);
    Assert(not buffer.empty());

    if (const size_t len = DLLWrapper_LibaryFilename_(_handle.Get(), buffer)) {
        *filename = buffer.CutBeforeConst(len);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
