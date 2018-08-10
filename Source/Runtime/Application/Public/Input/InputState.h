#pragma once

#include "Application.h"

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

    typedef T value_type;
    enum : size_t { Capacity = _Capacity };

    TInputState();
    ~TInputState();

    TInputState(const TInputState& other);
    TInputState& operator =(const TInputState& other);

    size_t size() const { return _size; }
    bool empty() const { return (0 == _size); }

    bool Contains(const T value) const;

    bool Add_KeepExisting(T value);
    void Add_AssertUnique(T value);

    bool Remove_ReturnIfExists(T value);
    void Remove_AssertExists(T value);

    void Clear();

    TMemoryView<const T> MakeView() const;

    /*static void Intersect(
        TInputState& up,
        TInputState& down,
        const TInputState& current,
        const TInputState& previous );*/

private:
    u32 _size;
    T _events[_Capacity];
};
//----------------------------------------------------------------------------
template <typename _State>
class IInputStateProvider {
public:
    virtual ~IInputStateProvider() {}
    virtual const _State& State() const = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef TInputState<EGamepadButton,  8> FGamepadButtonState;
typedef TInputState<EKeyboardKey,    8> FKeyboardKeyState;
typedef TInputState<EMouseButton,    8> FMouseButtonState;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#include "Input/InputState-inl.h"
