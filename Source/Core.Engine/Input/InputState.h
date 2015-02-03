#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Input/State/KeyboardKey.h"
#include "Core.Engine/Input/State/MouseButton.h"

#include "Core/Memory/MemoryView.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
class InputState {
public:
    typedef T value_type;
    enum : size_t { Capacity = _Capacity };

    InputState();
    ~InputState();

    InputState(const InputState& other);
    InputState& operator =(const InputState& other);

    size_t size() const { return _size; }
    bool empty() const { return (0 == _size); }

    bool Contains(const T& value) const;

    bool Add_KeepExisting(T&& value);
    void Add_AssertUnique(T&& value);

    bool Remove_ReturnIfExists(const T& value);
    void Remove_AssertExists(const T& value);

    void Clear();

    MemoryView<const T> MakeView() const;

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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef InputState<KeyboardKey, 6> KeyboardKeyState;
typedef InputState<MouseButton, 6> MouseButtonState;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core

#include "Core.Engine/Input/InputState-inl.h"
