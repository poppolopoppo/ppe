#pragma once

#include "Core.h"

#include "IO/StringView.h"
#include "Meta/PointerWFlags.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FDynamicLibrary {
public:
    FDynamicLibrary();
    ~FDynamicLibrary();

    FDynamicLibrary(const FDynamicLibrary&) = delete;
    FDynamicLibrary& operator =(const FDynamicLibrary&) = delete;

    FDynamicLibrary(FDynamicLibrary&& rvalue);
    FDynamicLibrary& operator =(FDynamicLibrary&& rvalue);

    void* Handle() const {
        Assert(IsValid());
        return _handle.Get();
    }

    bool IsValid() const { return _handle.Flag0(); }
    bool IsSharedResource() const { return _handle.Flag1(); }

    PPE_FAKEBOOL_OPERATOR_DECL() { return _handle.Get(); }

    bool Attach(const FWStringView& path);
    bool Load(const FWStringView& path);
    bool AttachOrLoad(const FWStringView& path);

    void Unload();
    bool UnloadIFP();

    FWString ModuleName() const;
    void* FunctionAddr(const FStringView& funcname) const;

    friend inline void swap(FDynamicLibrary& lhs, FDynamicLibrary& rhs) {
        swap(lhs._handle, rhs._handle);
    }

private:
    Meta::TPointerWFlags<void> _handle;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
