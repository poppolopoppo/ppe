#pragma once

#include "Core/Core.h"

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Return, typename... _Args>
class Callback {
public:
    typedef _Return (*helper_type)(void *, const _Args&... );

    template <typename T, _Return (T::*_Member)(const _Args&&... )>
    struct Bind {
        static helper_type helper() { return &call; }
        static _Return call(void *powner, const _Args& args... ) {
            return reinterpret_cast<T *>(powner)->*_Member(args...);
        }
    };

    Callback() : _powner(nullptr), _call(nullptr) {}
    Callback(void *powner, helper_type call) 
    :   _powner(powner), _call(call) {}

    operator helper_type () const { return _call; }

    _Return operator ()(const _Args& args...) const {
        Assert(_powner);
        Assert(_call);
        return _call(_powner, args...);
    }

private:
    void *_powner;
    helper_type _call;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
