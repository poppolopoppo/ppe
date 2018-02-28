#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core/Allocator/LinearHeap.h"
#include "Core/Container/HashSet.h"
#include "Core/IO/StringView.h"

#define USE_CORE_SERIALIZE_TEXTHEAP_SSO (not USE_CORE_MEMORY_DEBUGGING)

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTextHeap {
public:
    class FText {
    public:
        FText() {
            _small.IsSmall = true;
            _small.Size = 0;
        }
        ~FText() {}

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
        friend class FTextHeap;

        // FText use "Small String Optimization" (SSO)
        // - small strings will be stored inlined in the structure (we got 2*sizeof(size_t) storage occupied by FStringView)
        // - large strings will be stored as a FStringView pointing to memory allocated by the heap (or to a string literal)
        // SSO is lessening the pressure of the hash map and the linear heap by using memory already available.
        // we might somewhat lose some perf when comparing small strings since it won't benefit from pointer equality quick reject.

        struct FLargeText_ {
            size_t IsSmall : 1;
            size_t Size : sizeof(size_t) * 8 - 1;
            const char* Data;
            u64 _Padding_Unused; // padding added to get larger storage in small text (x86/x64 : 11/15 -> 15/23 chars)
            FStringView MakeView() const { return FStringView(Data, Size); }
        };

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
            ::memcpy(txt._small.Data, str.data(), str.SizeInBytes());
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

    FTextHeap(FLinearHeap& heap)
        : _texts(TLinearHeapAllocator<FText>(heap)) {
        STATIC_ASSERT(sizeof(FText::FLargeText_) == sizeof(FText::FSmallText_));
        STATIC_ASSERT(sizeof(FText::FLargeText_) == 2 * sizeof(size_t) + sizeof(u64));
    }

    bool empty() const { return _texts.empty(); }
    size_t size() const { return _texts.size(); }
    void reserve(size_t capacity) { _texts.reserve(capacity); }

    FText MakeText(const FStringView& str, bool mergeable = true) {
        if (str.empty()) {
            return FText();
        }
#if USE_CORE_SERIALIZE_TEXTHEAP_SSO
        else if (str.size() <= FText::FSmallText_::GCapacity) {
            return FText::MakeSmall_(str);
        }
#endif
        else if (Likely(mergeable)) {
            // always reserve a minimum size of 32 (avoid wasting relocate)
            if (_texts.capacity() == 0)
                _texts.reserve(32);

            FText txt = FText::MakeLarge_(str);
            // tries to pool short to medium strings :
            const auto it = _texts.insert(txt);
            if (it.second) {
                // hack for replacing registered string view with one pointing to linear heap storage
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
    HASHSET_LINEARHEAP(FText) _texts;

    FStringView AllocateString_(const FStringView& str) {
        Assert(str.size() > FText::FSmallText_::GCapacity);
        FLinearHeap* heap = _texts.get_allocator().Heap();
        Assert(heap);
        void* const storage = heap->Allocate(str.SizeInBytes(), std::alignment_of_v<char>);
        ::memcpy(storage, str.data(), str.SizeInBytes());
        return FStringView((const char*)storage, str.size());
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
