#pragma once

#include "Application.h"

#include "Container/FixedSizeHashSet.h"
#include "Memory/MemoryView.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
class TInputState {
public:
    STATIC_ASSERT(std::is_enum<T>::value);

    TInputState() = default;

    TInputState(const TInputState& other) = default;
    TInputState& operator =(const TInputState& other) = default;

    TInputState(TInputState&& rvalue) = default;
    TInputState& operator =(TInputState&& rvalue) = default;

    size_t size() const { return _events.size(); }
    bool empty() const { return _events.empty(); }

    using const_iterator = typename TFixedSizeHashSet<T, _Capacity>::const_iterator;
    const_iterator begin() const { return _events.begin(); }
    const_iterator end() const { return _events.end(); }

    bool Contains(const T value) const { return _events.Contains(value); }
    bool Add_KeepExisting(T value) { return _events.Add_KeepExisting(value); }
    void Add_AssertUnique(T value) { _events.Add_AssertUnique(value); }
    bool Remove_ReturnIfExists(T value) { return _events.Remove_ReturnIfExists(value); }
    void Remove_AssertExists(T value) { _events.Remove_AssertExists(value); }
    void Clear() { _events.clear(); }

    bool Update(
        TInputState* up,
        TInputState* down,
        const TInputState& pressed ) {
        Assert(up);
        Assert(down);

        up->Clear();
        down->Clear();

        for (auto e : pressed._events) {
            if (not Contains(e))
                down->Add_AssertUnique(e);
        }

        for (auto e : _events) {
            if (not pressed.Contains(e))
                up->Add_AssertUnique(e);
        }

        _events = pressed._events;

        return (not (up->empty() && down->empty()));
    }

private:
    TFixedSizeHashSet<T, _Capacity> _events;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
