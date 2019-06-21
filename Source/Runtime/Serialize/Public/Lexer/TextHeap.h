#pragma once

#include "Serialize.h"

#include "Allocator/LinearAllocator.h"
#include "Container/HashSet.h"
#include "HAL/PlatformMemory.h"
#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"

#define USE_PPE_SERIALIZE_TEXTHEAP_SSO (not USE_PPE_MEMORY_DEBUGGING)

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <bool _Padded/* padding adds more insitu storage capacity */>
class TTextHeap {
public:
    STATIC_CONST_INTEGRAL(size_t, GMaxSizeForMerge, 100);

    class FText {
    public:
        FText() {
            _small.IsSmall = true;
            _small.Size = 0;
            ONLY_IF_ASSERT(FPlatformMemory::Memzero(_small.Data, sizeof(_small.Data)));
        }
        ~FText() = default;

        FText(const FText& other) { operator =(other); }
        FText& operator =(const FText& other) {
            _large = other._large;
            Assert(_small.IsSmall == other._small.IsSmall);
            return (*this);
        }

        bool empty() const { return (0 == size()); }
        size_t size() const { return (_small.IsSmall ? _small.Size : _large.Size); }

        FStringView MakeView() const { return (_small.IsSmall ? _small.MakeView() : _large.MakeView()); }
        operator FStringView () const { return MakeView(); }

        inline friend hash_t hash_value(const FText& txt) { return hash_string(txt.MakeView()); }

        inline friend bool operator ==(const FText& lhs, const FText& rhs) { return Equals(lhs.MakeView(), rhs.MakeView()); }
        inline friend bool operator !=(const FText& lhs, const FText& rhs) { return (not operator ==(lhs, rhs)); }

    private:
        friend class TTextHeap;

        // FText use "Small String Optimization" (SSO)
        // - small strings will be stored inlined in the structure (we got 2*sizeof(size_t) storage occupied by FStringView)
        // - large strings will be stored as a FStringView pointing to memory allocated by the heap (or to a string literal)
        // SSO is lessening the pressure on the hash map and the linear heap by using memory already available.
        // we might somewhat lose some perf when comparing small strings since it won't benefit from pointer equality.

        struct FLargeTextNotPadded_ {
            size_t IsSmall : 1;
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
            u8 IsSmall  : 1;
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
            txt._small.IsSmall = true;
            txt._small.Size = checked_cast<u8>(str.size());
            FPlatformMemory::Memcpy(txt._small.Data, str.data(), str.SizeInBytes());
            Assert(txt._small.IsSmall);
            Assert(txt._large.IsSmall);
            Assert(txt._small.Size == str.size());
            return txt;
        }

        static FText MakeLarge_(const FStringView& str) {
            FText txt;
            txt._large.IsSmall = false;
            txt._large.Size = str.size();
            txt._large.Data = str.data();
            Assert(not txt._small.IsSmall);
            Assert(not txt._large.IsSmall);
            Assert(txt._large.Size == str.size());
            return txt;
        }
    };

    TTextHeap(FPooledLinearHeap& heap)
        : _heap(heap) {
        STATIC_ASSERT(sizeof(typename FText::FLargeText_) == sizeof(typename FText::FSmallText_));
        STATIC_ASSERT(_Padded
            ? sizeof(typename FText::FLargeText_) == 2 * sizeof(size_t) + sizeof(u64)
            : sizeof(typename FText::FLargeText_) == 2 * sizeof(size_t) );
    }

    bool empty() const { return _texts.empty(); }
    size_t size() const { return _texts.size(); }
    void reserve(size_t capacity) { _texts.reserve(capacity); }

    void Clear() {
        _texts.clear_ReleaseMemory();
    }

    FText MakeText(const FStringView& str, bool mergeable = true) {
        if (str.empty()) {
            return FText();
        }
#if USE_PPE_SERIALIZE_TEXTHEAP_SSO
        else if (str.size() <= FText::FSmallText_::GCapacity) {
            return FText::MakeSmall_(str);
        }
#endif
        else if (Likely(mergeable && str.size() <= GMaxSizeForMerge)) {
            // always reserve a minimum size of 64 (avoid wasting too much on relocate)
            if (_texts.capacity() == 0)
                _texts.reserve(64);

            // tries to pool short to medium strings :
            FText txt = FText::MakeLarge_(str);
            const auto it = _texts.insert(txt);

            if (it.second) {
                // hack the map by replacing registered string view with one pointing to linear heap storage
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

    static FText MakeStaticText(const FStringView& str) {
        // the user is responsible for lifetime of str :
        // this class will consider str as a static literal string
        return FText::MakeLarge_(str);
    }

private:
    FPooledLinearHeap& _heap;
    HASHSET(Transient, FText) _texts;

    FStringView AllocateString_(const FStringView& str) {
#if USE_PPE_SERIALIZE_TEXTHEAP_SSO
        Assert(str.size() > FText::FSmallText_::GCapacity);
#endif

        void* const storage = _heap.Allocate(str.SizeInBytes());
        FPlatformMemory::Memcpy(storage, str.data(), str.SizeInBytes());
        return FStringView((const char*)storage, str.size());
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, bool _Padded>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const typename Serialize::TTextHeap<_Padded>::FText& text) {
    return oss << text.MakeView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
