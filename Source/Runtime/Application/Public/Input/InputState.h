#pragma once

#include "Application.h"

#include "Input/GamepadButton.h"
#include "Input/KeyboardKey.h"
#include "Input/MouseButton.h"

#include "Memory/MemoryView.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
class InputState {
public:
    STATIC_ASSERT(std::is_enum<T>::value);

    typedef T value_type;
    enum : size_t { Capacity = _Capacity };

    InputState();
    ~InputState();

    InputState(const InputState& other);
    InputState& operator =(const InputState& other);

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
        InputState& up,
        InputState& down,
        const InputState& current,
        const InputState& previous );*/

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
typedef InputState<EGamepadButton,  8> FGamepadButtonState;
typedef InputState<EKeyboardKey,    8> FKeyboardKeyState;
typedef InputState<EMouseButton,    8> FMouseButtonState;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#include "Input/InputState-inl.h"
