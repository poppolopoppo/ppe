#pragma once

#include "Application.h"

#include "Container/FlatSet.h"
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

    bool Contains(const T value) const {
        return (_events.end() != _events.find(value));
    }

    bool Add_KeepExisting(T value) {
        bool added;
        _events.FindOrAdd(value, &added);
        return added;
    }

    void Add_AssertUnique(T value) {
        _events.Insert_AssertUnique(value);
    }

    bool Remove_ReturnIfExists(T value) {
        return _events.Erase(value);
    }

    void Remove_AssertExists(T value) {
        _events.Remove_AssertExists(value);
    }

    void Clear() {
        _events.clear();
    }

    TMemoryView<const T> MakeView() const {
        _events.MakeView();
    }

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
    FLATSET_INSITU(Input, T, _Capacity) _events;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
