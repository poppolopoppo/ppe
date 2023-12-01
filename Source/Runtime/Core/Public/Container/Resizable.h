#pragma once

#include "Core_fwd.h"

#include "Memory/MemoryView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T /*, #TODO: use C++20 requires requires
    decltype(SizeOf(std::declval<const T&>()))* = nullptr,
    decltype(Resize_DiscardData(std::declval<T&>(), std::declval<size_t>()))* = nullptr,
    decltype(MakeView(std::declval<T&>()))* = nullptr*/ >
struct TResizable {
    T* Data{ nullptr };

    using view_type = decltype(MakeView(std::declval<T&>()));
    using const_view_type = decltype(MakeView(std::declval<const T&>()));
    using value_type = typename view_type::value_type;

    NODISCARD view_type View() NOEXCEPT { Assert(Data); return MakeView(*Data); }
    NODISCARD const_view_type View() const NOEXCEPT { Assert(Data); return MakeView(*Data); }
    NODISCARD size_t Size() const NOEXCEPT { Assert(Data); return SizeOf(*Data); }
    void Resize(size_t size) { Assert(Data); Resize_DiscardData(*Data, size); }

    NODISCARD FRawMemory RawView() NOEXCEPT { return MakeRawView(View()); }
    NODISCARD FRawMemoryConst RawView() const NOEXCEPT { return MakeRawConstView(View()); }
};
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TResizable<T> MakeResizable(T* data) NOEXCEPT {
    return { data };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
