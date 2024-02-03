#pragma once

#include "Core_fwd.h"

#include "IO/Format.h"
#include "IO/Text.h"
#include "IO/StringBuilder.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator, typename _Char, size_t _Len, typename _Arg0, typename... _Args>
NODISCARD auto TextFormat(const _Allocator& allocator, const _Char (&format)[_Len], _Arg0&& arg0, _Args&& ...args) -> TBasicText<_Char, _Allocator> {
    return TextFormat(allocator, TBasicStringLiteral<_Char>(format), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Allocator, typename _Char, typename _Arg0, typename... _Args>
NODISCARD auto TextFormat(const _Allocator& allocator, TBasicStringLiteral<_Char> format, _Arg0&& arg0, _Args&& ...args) -> TBasicText<_Char, _Allocator> {

    using text_type = TBasicText<_Char, _Allocator>;
    using inplace_allocator_type = TSegregateAllocator<
        (text_type::SmallCapacity * sizeof(_Char)),
        TInSituAllocator<text_type::SmallCapacity * sizeof(_Char)>,
        _Allocator>;

    TGenericStringBuilder<_Char, inplace_allocator_type> sb{ inplace_allocator_type(allocator) };
    Format(sb, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);

    TBasicText<_Char, _Allocator> text{ allocator };
    if (Likely(sb.size() > text_type::SmallCapacity)) {

        // steal allocation block for larger texts
        size_t sizeInBytes = 0;
        const FAllocatorBlock b = sb.StealDataUnsafe(&sizeInBytes);
        AssertRelease(b);
        Assert_NoAssume(b.SizeInBytes >= sizeInBytes);

        if (not text.AcquireDataUnsafe(b, sizeInBytes / sizeof(_Char)))
            AssertNotReached();
    }
    else {
        // copy for inplace allocations
        text.AssignSmall(sb.Written());
    }

    return text;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
