#pragma once

#include "Serialize.h"

#include "Allocator/SlabAllocator.h"
#include "Container/HashSet.h"
#include "HAL/PlatformMemory.h"
#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"
#include "Memory/PtrRef.h"

#define USE_PPE_SERIALIZE_TEXTHEAP_SSO (not USE_PPE_MEMORY_DEBUGGING)

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <bool _Padded/* padding adds more insitu storage capacity */, typename _Allocator>
class TTextHeap : Meta::FNonCopyable {
public:
    STATIC_CONST_INTEGRAL(size_t, GMaxSizeForMerge, 100);

    using heap_type = TSlabHeap<_Allocator>;

    class FText {
    public:
        CONSTEXPR FText() : _small{} {}

        CONSTEXPR FText(const FText& other) { operator =(other); }
        CONSTEXPR FText& operator =(const FText& other) {
            STATIC_ASSERT(sizeof(FSmallText_) == sizeof(FLargeText_));
            _large = other._large;
            Assert(_small.IsLarge == other._small.IsLarge);
            return (*this);
        }

        CONSTEXPR bool empty() const { return (0 == size()); }
        CONSTEXPR size_t size() const { return (_small.IsLarge ? _large.Size : _small.Size); }

        FStringView MakeView() const { return (_small.IsLarge ? _large.MakeView() : _small.MakeView()); }
        operator FStringView () const { return MakeView(); }

        inline friend hash_t hash_value(const FText& txt) { return hash_string(txt.MakeView()); }

        inline friend bool operator ==(const FText& lhs, const FText& rhs) { return Equals(lhs.MakeView(), rhs.MakeView()); }
        inline friend bool operator !=(const FText& lhs, const FText& rhs) { return (not operator ==(lhs, rhs)); }

        inline friend bool operator < (const FText& lhs, const FText& rhs) { return Compare(lhs.MakeView(), rhs.MakeView()) < 0; }
        inline friend bool operator >=(const FText& lhs, const FText& rhs) { return (not operator < (lhs, rhs)); }

    private:
        friend class TTextHeap;

        // FText use "Small String Optimization" (SSO)
        // - small strings will be stored inlined in the structure (we got 2*sizeof(size_t) storage occupied by FStringView)
        // - large strings will be stored as a FStringView pointing to memory allocated by the heap (or to a string literal)
        // SSO is lessening the pressure on the hash map and the linear heap by using memory already available.
        // we might somewhat lose some perf when comparing small strings since it won't benefit from pointer equality.

        struct FLargeTextNotPadded_ {
            size_t IsLarge : 1;
            size_t Size : sizeof(size_t) * 8 - 1;
            const char* Data;
            FStringView MakeView() const { return FStringView(Data, Size); }
        };

        struct FLargeTextPadded_ : FLargeTextNotPadded_ {
            u64 _Padding_Unused; // padding added to get larger storage in small text (x86/x64 : 11/15 -> 15/23 chars)
        };

        using FLargeText_ = std::conditional_t<_Padded, FLargeTextPadded_, FLargeTextNotPadded_>;

        struct FSmallText_ {
            STATIC_CONST_INTEGRAL(size_t, GCapacity, (sizeof(FLargeText_) - sizeof(u8)) / sizeof(char));
            u8 IsLarge  : 1;
            u8 Size     : 7;
            char Data[GCapacity];
            FStringView MakeView() const { return FStringView(Data, Size); }
        };

        union {
            FSmallText_ _small;
            FLargeText_ _large;
        };

        static FText MakeSmall_(const FStringView& str) {
            Assert(str.size() <= FSmallText_::GCapacity);
            FText txt;
            txt._small.IsLarge = false;
            txt._small.Size = checked_cast<u8>(str.size());
            FPlatformMemory::Memcpy(txt._small.Data, str.data(), str.SizeInBytes());
            Assert_NoAssume(not txt._small.IsLarge);
            Assert_NoAssume(not txt._large.IsLarge);
            Assert_NoAssume(txt._small.Size == str.size());
            return txt;
        }

        static CONSTEXPR FText MakeLarge_(const FStringView& str) {
            FLargeText_ large{};
            large.IsLarge = true;
            large.Size = str.size();
            large.Data = str.data();
            return FText{ std::move(large) };
        }

        CONSTEXPR explicit FText(FSmallText_&& rsmall) : _small(std::move(rsmall)) {}
        CONSTEXPR explicit FText(FLargeText_&& rlarge) : _large(std::move(rlarge)) {}

    };

    explicit TTextHeap(heap_type& heap) NOEXCEPT
    :   _heap(heap) {
        STATIC_ASSERT(_Padded
            ? sizeof(typename FText::FLargeText_) == 2 * sizeof(size_t) + sizeof(u64)
            : sizeof(typename FText::FLargeText_) == 2 * sizeof(size_t) );
    }

    TTextHeap(TTextHeap&&) = default;
    TTextHeap& operator =(TTextHeap&&) = default;

    bool empty() const { return _texts.empty(); }
    size_t size() const { return _texts.size(); }
    void reserve(size_t capacity) { _texts.reserve(capacity); }

    void Clear_ForgetMemory() {
        _texts.clear_ReleaseMemory();
    }

    // Can't deallocate unmergeable strings since there are not tracked : the cache is a lie
    //void Clear_ReleaseMemory() {
    //    for (const FText& it : _texts) {
    //        if (not it._small.IsSmall)
    //            DeallocateString(it.MakeView());
    //    }
    //    Clear_ForgetMemory();
    //}

    FText MakeText(const FStringView& str, bool mergeable = true) {
        if (str.empty()) {
            return FText();
        }
#if USE_PPE_SERIALIZE_TEXTHEAP_SSO
        else if (str.size() <= FText::FSmallText_::GCapacity) {
            return FText::MakeSmall_(str);
        }
#endif
        else if (Likely(mergeable & (str.size() <= GMaxSizeForMerge))) {
            // always reserve a minimum size of 64 (avoid wasting too much on relocate)
            if (_texts.capacity() == 0)
                _texts.reserve(64);

            // tries to pool short to medium strings :
            FText txt = FText::MakeLarge_(str);
            const auto it = _texts.insert(txt);

            if (it.second) {
                // hack the map by replacing registered string view with one pointing to the heap
                const FStringView allocated = AllocateString_(str);
                auto& registered = const_cast<FText&>(*it.first);
                Assert(hash_value(FText::MakeLarge_(allocated)) == hash_value(registered));
                registered = FText::MakeLarge_(allocated);
            }

            return (*it.first);
        }
        else {
            // some strings have too much entropy and won't benefit from merging :
            return FText::MakeLarge_(AllocateString_(str));
        }
    }

    static CONSTEXPR FText MakeStaticText(const FStringView& str) {
        // the user is responsible for lifetime of str :
        // this class will consider str as a static literal string
        return FText::MakeLarge_(str);
    }

private:
    TPtrRef<heap_type> _heap;
    HASHSET(Transient, FText) _texts;

    FStringView AllocateString_(const FStringView& str) {
#if USE_PPE_SERIALIZE_TEXTHEAP_SSO
        Assert(str.size() > FText::FSmallText_::GCapacity);
#endif
        void* const storage = _heap->Allocate(str.SizeInBytes());
        FPlatformMemory::Memcpy(storage, str.data(), str.SizeInBytes());
        return FStringView(static_cast<const char*>(storage), str.size());
    }

    void DeallocateString(const FStringView& str) {
#if USE_PPE_SERIALIZE_TEXTHEAP_SSO
        Assert(str.size() > FText::FSmallText_::GCapacity);
#endif
        _heap->Deallocate(const_cast<char*>(str.data()), str.SizeInBytes());
    }
};
//----------------------------------------------------------------------------
template <typename _Char, bool _Padded, typename _Allocator>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const typename TTextHeap<_Padded, _Allocator>::FText& text) {
    return oss << text.MakeView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
