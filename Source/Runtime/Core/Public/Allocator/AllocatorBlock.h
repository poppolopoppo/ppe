#pragma once

#include "Core_fwd.h"

#include "Memory/MemoryView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct NODISCARD FAllocatorBlock {
    void* Data{ nullptr };
    size_t SizeInBytes{ 0 };

    PPE_FAKEBOOL_OPERATOR_DECL() { return (!!Data); }

    CONSTEXPR FAllocatorBlock Reset() NOEXCEPT {
        const FAllocatorBlock cpy{ *this };
        *this = Default;
        return cpy;
    }

    CONSTEXPR FRawMemory MakeView() const { return { static_cast<u8*>(Data), SizeInBytes }; }

    friend bool operator ==(const FAllocatorBlock& lhs, const FAllocatorBlock& rhs) NOEXCEPT {
        return (lhs.Data == rhs.Data && lhs.SizeInBytes == rhs.SizeInBytes);
    }
    friend bool operator !=(const FAllocatorBlock& lhs, const FAllocatorBlock& rhs) NOEXCEPT {
        return not operator ==(lhs, rhs);
    }

    template <typename T>
    static CONSTEXPR FAllocatorBlock From(TMemoryView<T> v) NOEXCEPT {
        return { v.data(), v.SizeInBytes() };
    }
};
PPE_ASSUME_TYPE_AS_POD(FAllocatorBlock)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
