#pragma once

#include "Core/Core.h"

#include "Core/IO/StringView.h"
#include "Core/Meta/PointerWFlags.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_API FDLLWrapper {
public:
    FDLLWrapper();
    ~FDLLWrapper();

    FDLLWrapper(const FDLLWrapper&) = delete;
    FDLLWrapper& operator =(const FDLLWrapper&) = delete;

    FDLLWrapper(FDLLWrapper&& rvalue);
    FDLLWrapper& operator =(FDLLWrapper&& rvalue);

    void* Handle() const {
        Assert(IsValid());
        return _handle.Get();
    }

    bool IsValid() const { return _handle.Flag0(); }
    bool IsSharedResource() const { return _handle.Flag1(); }

    CORE_FAKEBOOL_OPERATOR_DECL() { return _handle.Get(); }

    bool Attach(const FStringView& path);
    bool Load(const FStringView& path);
    bool AttachOrLoad(const FStringView& path);

    void Unload();
    bool UnloadIFP();

    void* FunctionAddr(const FStringView& funcname);
    bool LibaryFilename(const TMemoryView<char>& buffer, FStringView* filename);

    friend inline void swap(FDLLWrapper& lhs, FDLLWrapper& rhs) {
        swap(lhs._handle, rhs._handle);
    }

private:
    Meta::TPointerWFlags<void> _handle;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
