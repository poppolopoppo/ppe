#pragma once

#include "Core/Core.h"

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct CallbackImpl {
    enum : bool { enabled = false };
};
//----------------------------------------------------------------------------
template <typename _Return, typename... _Args>
struct CallbackImpl<_Return (*)(_Args...)> {
    enum : bool { enabled = true };

    typedef _Return (*helper_func)(void *powner, _Args... args);

    void *_powner;
    helper_func _fhelper;

    operator bool () const { return _fhelper != nullptr; }
    _Return operator ()(_Args... args) const { return _fhelper(_powner, args...); }

    template <typename T, _Return (T::*_Member)(_Args...)>
    void bind(T *powner) {
        assert(powner);
        _powner = powner;
        _fhelper = &call_<T, _Member>;
    }

    template <typename T, _Return (T::*_Member)(_Args...)>
    static _Return call_(void *powner, _Args... args) {
        assert(nullptr != powner);
        return (reinterpret_cast<T *>(powner)->*_Member)(args...);
    }

    template <_Return (*_Function)(_Args...)>
    void bind() {
        _powner = nullptr;
        _fhelper = &call_<_Function>;
    }

    template <_Return (*_Function)(_Args...)>
    static _Return call_(void *powner, _Args... args) {
        assert(nullptr == powner);
        return (*_Function)(args...);
    }
};
//----------------------------------------------------------------------------
template <typename _Function>
struct Callback : CallbackImpl<_Function> {
    typedef CallbackImpl<_Function> impl_type;
    static_assert(impl_type::enabled, "unknown function type");
    using impl_type::operator bool;
    using impl_type::operator ();
    using impl_type::bind;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
