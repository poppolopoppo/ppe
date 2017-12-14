#pragma once

#include "Delegate.h"

/************************************************************************/
/* Bind TDelegate                                                        */
/************************************************************************/
namespace Core {
namespace details {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Non const member function  :
//----------------------------------------------------------------------------
template <typename T, typename _Ret, typename... _Args >
struct TBindDelegate<T *, _Ret (T::*)(_Args... )> {
    typedef TDelegate<_Ret (*)(_Args... )> type;

    template <_Ret (T::*_Member)(_Args... )>
    static type get(T *pcallee) NOEXCEPT {
        return type(&Wrapper_<_Member>, static_cast<void *>(pcallee));
    }

    template <_Ret (T::*_Member)(_Args... )>
    static _Ret Wrapper_(void *pcallee, _Args... args ) {
        T *const p = static_cast<T *>(pcallee);
        return (p->*_Member)(std::forward<_Args>(args)... );
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Const member function, const callee  :
//----------------------------------------------------------------------------
template <typename T, typename _Ret, typename... _Args >
struct TBindDelegate<const T*, _Ret (T::*)(_Args... ) const> {
    typedef TDelegate<_Ret (*)(_Args... )> type;

    template <_Ret (T::*_Member)(_Args... ) const>
    static type get(const T* pcallee) NOEXCEPT {
        return type(&Wrapper_<_Member>, (void*)pcallee);
    }

    template <_Ret (T::*_Member)(_Args... ) const>
    static _Ret Wrapper_(void *pcallee, _Args... args ) {
        const T* p = static_cast<const T*>(pcallee);
        return (p->*_Member)(std::forward<_Args>(args)... );
    }
};
//----------------------------------------------------------------------------
// Const member function, non const callee  :
//----------------------------------------------------------------------------
template <typename T, typename _Ret, typename... _Args >
struct TBindDelegate<T*, _Ret (T::*)(_Args... ) const>
    : public TBindDelegate<const T*, _Ret (T::*)(_Args... ) const> {
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Free function with one leading deferred argument :
//----------------------------------------------------------------------------
template <typename _Arg0, typename _Ret, typename... _Args >
struct TBindDelegate<_Arg0, _Ret (*)(_Arg0, _Args... )> {
    typedef TDelegate<_Ret (*)(_Args... )> type;

    STATIC_ASSERT(Meta::TIsPod<_Arg0>::value);
    static_assert(  sizeof(_Arg0) <= sizeof(void *),
                    "_Arg0 is too big to fit in a void *" );
    typedef union { _Arg0 typed; void *raw; } arg0_type;

    template <_Ret (*_Func)(_Arg0, _Args... )>
    static type get(_Arg0&& arg0) NOEXCEPT {
        arg0_type callee = {0};
        callee.typed = std::forward<_Arg0>(arg0);
        return type(&Wrapper_<_Func>, callee.raw);
    }

    template <_Ret (*_Func)(_Arg0, _Args... )>
    static _Ret Wrapper_(void *pcallee, _Args... args ) {
        arg0_type arg0 = {0};
        arg0.raw = pcallee;
        return (*_Func)(arg0.typed, std::forward<_Args>(args)... );
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Binary function with one trailing deferred argument :
//----------------------------------------------------------------------------
template <typename _Arg1, typename _Ret, typename _Arg0 >
struct TBindDelegate<_Arg1, _Ret (*)(_Arg0, _Arg1)> {
    typedef TDelegate<_Ret (*)(_Arg0)> type;

    STATIC_ASSERT(Meta::TIsPod<_Arg1>::value);
    static_assert(  sizeof(_Arg1) <= sizeof(void *),
                    "_Arg1 is too big to fit in a void *" );
    typedef union { _Arg1 typed; void *raw; } arg1_type;

    template <_Ret (*_Func)(_Arg0, _Arg1)>
    static type get(_Arg1&& arg1) NOEXCEPT {
        arg1_type callee = {0};
        callee.typed = std::forward<_Arg1>(arg1);
        return type(&Wrapper_<_Func>, callee.raw);
    }

    template <_Ret (*_Func)(_Arg0, _Arg1)>
    static _Ret Wrapper_(void *pcallee, _Arg0 arg0) {
        arg1_type arg1 = {0};
        arg1.raw = pcallee;
        return (*_Func)(std::forward<_Arg0>(arg0), arg1.typed);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Bare free function :
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args >
struct TBindDelegate<decltype(nullptr), _Ret (*)(_Args... )> {
    typedef TDelegate<_Ret (*)(_Args... )> type;

    template <_Ret (*_Func)(_Args... )>
    static type get(void *) NOEXCEPT {
        return type(&Wrapper_<_Func>, nullptr);
    }

    template <_Ret (*_Func)(_Args... )>
    static _Ret Wrapper_(void *pcallee, _Args... args ) {
        return (*_Func)(std::forward<_Args>(args)... );
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace details
} //!namespace Core
