#pragma once

#include "Core.h"

#include "IO/StringView.h"
#include "Meta/PointerWFlags.h"
#include "Misc/Function.h"
#include "Misc/Event.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FDynamicLibrary {
public:
    FDynamicLibrary() NOEXCEPT;
    ~FDynamicLibrary();

    FDynamicLibrary(const FDynamicLibrary&) = delete;
    FDynamicLibrary& operator =(const FDynamicLibrary&) = delete;

    FDynamicLibrary(FDynamicLibrary&& rvalue) NOEXCEPT;
    FDynamicLibrary& operator =(FDynamicLibrary&& rvalue) NOEXCEPT;

    void* Handle() const {
        Assert_NoAssume(IsValid());
        return _handle.Get();
    }

    bool IsValid() const { return _handle.Flag0(); }
    bool IsSharedResource() const { return _handle.Flag1(); }

    PPE_FAKEBOOL_OPERATOR_DECL() { return (!!_handle.Get()); }

    bool Attach(const wchar_t* path);
    bool Load(const wchar_t* path);
    bool AttachOrLoad(const wchar_t* path);

    void Unload();
    bool UnloadIFP();

    FWString ModuleName() const;
    void* FunctionAddr(const char* funcname) const;

    friend inline void swap(FDynamicLibrary& lhs, FDynamicLibrary& rhs) NOEXCEPT {
        swap(lhs._handle, rhs._handle);
    }

public: // events
    using FLibraryLoadDelegate = TFunction<void(const FDynamicLibrary&, const wchar_t* path)>;
    using FLibraryUnloadDelegate = TFunction<void(const FDynamicLibrary&)>;

    PUBLIC_EVENT(OnAttachLibrary,   FLibraryLoadDelegate)
    PUBLIC_EVENT(OnLoadLibrary,     FLibraryLoadDelegate)
    PUBLIC_EVENT(OnUnloadLibrary,   FLibraryUnloadDelegate)

private:
    Meta::FPointerWFlags _handle;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
