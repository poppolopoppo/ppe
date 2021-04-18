#pragma once

#include "RHI_fwd.h"

#include "Meta/Iterator.h"
#include "Memory/MemoryView.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FBufferView {
public:
    using value_type = u8;
    using subpart_type = TMemoryView<value_type>;

    class FIterator : public Meta::TIterator<value_type> {
    public:
        FIterator(const FIterator&) = default;
        FIterator& operator =(const FIterator&) = default;

        FIterator(FIterator&&) = default;
        FIterator& operator =(FIterator&&) = default;

        value_type operator *() const { return (_part->at(_index)); }
        const value_type* operator->() const { return (&_part->at(_index)); }

        FIterator& operator ++() {
            if (++_index >= _part->size()) {
                ++_part;
                _index = 0;
            }
            return (*this);
        }
        FIterator operator ++(int) {
            const FIterator result{ *this };
            ++(*this);
            return result;
        }

        FIterator& operator --() {
            if (--_index >= _part->size()) {
                --_part;
                _index = (_part->size() - 1);
            }
            return (*this);
        }
        FIterator operator --(int) {
            const FIterator result{ *this };
            --(*this);
            return result;
        }

        bool operator ==(const FIterator& other) const { return (_part == other._part && _index == other._index); }
        bool operator !=(const FIterator& other) const { return (not operator ==(other)); }

    private:
        friend struct FBufferView;
        friend struct FImageView;
        FIterator(TMemoryView<subpart_type>::iterator part, size_t index) NOEXCEPT : _part(part), _index(index) {}

        TMemoryView<subpart_type>::iterator _part;
        size_t _index;
    };

    FBufferView() = default;
    explicit FBufferView(TMemoryView<subpart_type> parts) NOEXCEPT : _parts(parts) {}

    bool empty() const { return (size() == 0); }
    size_t size() const {
        size_t total = 0;
        for (const subpart_type& sub : _parts)
            total += sub.size();
        return total;
    }

    const value_type* data() const {
        Assert(not _parts.empty());
        return _parts.front().data();
    }

    FIterator begin() const { return FIterator{ _parts.begin(), 0 }; }
    FIterator end() const { return FIterator{ _parts.end(), 0 }; }

    value_type operator [](size_t i) const {
        for (const subpart_type& sub : _parts) {
            if (i < sub.size())
                return sub[i];
            i -= sub.size();
        }
        AssertNotReached(); // out-of-bounds
    }

    TMemoryView<subpart_type> Parts() const { return _parts; }

    subpart_type SubRange(size_t offset, size_t count) const {
        for (const subpart_type& sub : _parts) {
            if (offset < sub.size())
                return sub.SubRange(offset, count);
            offset -= sub.size();
        }
        AssertNotReached(); // out-of-bounds
    }

private:
    TMemoryView<subpart_type> _parts;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
