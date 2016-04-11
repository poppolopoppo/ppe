#pragma once

#include "Core.Application/Input/InputState.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
InputState<T, _Capacity>::InputState()
:   _size(0) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
InputState<T, _Capacity>::~InputState() {}
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
InputState<T, _Capacity>::InputState(const InputState& other)
:   _size(other._size) {
    for (size_t i = 0; i < _size; ++i)
        _events[i] = other._events[i];
}
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
auto InputState<T, _Capacity>::operator =(const InputState& other) -> InputState& {
    _size = other._size;
    for (size_t i = 0; i < _size; ++i)
        _events[i] = other._events[i];

    return *this;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
bool InputState<T, _Capacity>::Contains(const T value) const {
    for (size_t i = 0; i < _size; ++i)
        if (_events[i] == value)
            return true;

    return false;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
bool InputState<T, _Capacity>::Add_KeepExisting(const T value) {
    for (size_t i = 0; i < _size; ++i)
        if (_events[i] == value)
            return false;

    Assert(_size < _Capacity);
    _events[_size++] = value;

    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
void InputState<T, _Capacity>::Add_AssertUnique(const T value) {
    AssertRelease(_size < _Capacity);
    Assert(!Contains(value));

    _events[_size++] = value;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
bool InputState<T, _Capacity>::Remove_ReturnIfExists(const T value) {
    Assert(_size < _Capacity);

    for (size_t i = 0; i < _size; ++i)
        if (_events[i] == value) {
            if (i + 1 != _size)
                _events[i] = _events[_size - 1];
            --_size;
            return true;
        }

    return false;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
void InputState<T, _Capacity>::Remove_AssertExists(const T value) {
    if (false == Remove_ReturnIfExists(value))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
void InputState<T, _Capacity>::Clear() {
    _size = 0;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
MemoryView<const T> InputState<T, _Capacity>::MakeView() const {
    return MemoryView<const T>(_events, _size);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
