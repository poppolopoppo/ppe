#pragma once

#include "Delegate.h"

/************************************************************************/
/* Bind Delegate                                                        */
/************************************************************************/
namespace Core {
namespace details {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Non const member function  :
//----------------------------------------------------------------------------
template <typename T, typename _Ret, typename... _Args >
struct BindDelegate<T *, _Ret (T::*)(_Args... )> {
    typedef Delegate<_Ret (*)(_Args... )> type;

    template <_Ret (T::*_Member)(_Args... )>
    static type get(T *pcallee);

    template <_Ret (T::*_Member)(_Args... )>
    static _Ret Wrapper_(void *pcallee, _Args... args );
};
//----------------------------------------------------------------------------
template <typename T, typename _Ret, typename... _Args >
template <_Ret (T::*_Member)(_Args... )>
auto BindDelegate<T *, _Ret (T::*)(_Args... )>::get(T *pcallee) -> type {
    return type(&Wrapper_<_Member>, static_cast<void *>(pcallee));
}
//----------------------------------------------------------------------------
template <typename T, typename _Ret, typename... _Args >
template <_Ret (T::*_Member)(_Args... )>
_Ret BindDelegate<T *, _Ret (T::*)(_Args... )>::Wrapper_(void *pcallee, _Args... args ) {
    Likely(pcallee);
    T *const p = static_cast<T *>(pcallee);
    return (p->*_Member)(std::forward<_Args>(args)... );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Const member function  :
//----------------------------------------------------------------------------
template <typename T, typename _Ret, typename... _Args >
struct BindDelegate<T *, _Ret (T::*)(_Args... ) const> {
    typedef Delegate<_Ret (*)(_Args... )> type;

    template <_Ret (T::*_Member)(_Args... ) const>
    static type get(T *pcallee);

    template <_Ret (T::*_Member)(_Args... ) const>
    static _Ret Wrapper_(void *pcallee, _Args... args );
};
//----------------------------------------------------------------------------
template <typename T, typename _Ret, typename... _Args >
template <_Ret (T::*_Member)(_Args... ) const>
auto BindDelegate<T *, _Ret (T::*)(_Args... ) const>::get(T *pcallee) -> type {
    return type(&Wrapper_<_Member>, static_cast<void *>(pcallee));
}
//----------------------------------------------------------------------------
template <typename T, typename _Ret, typename... _Args >
template <_Ret (T::*_Member)(_Args... ) const>
_Ret BindDelegate<T *, _Ret (T::*)(_Args... ) const>::Wrapper_(void *pcallee, _Args... args ) {
    Likely(pcallee);
    const T *p = static_cast<const T *>(pcallee);
    return (p->*_Member)(std::forward<_Args>(args)... );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Free function with one leading deferred argument :
//----------------------------------------------------------------------------
template <typename _Arg0, typename _Ret, typename... _Args >
struct BindDelegate<_Arg0, _Ret (*)(_Arg0, _Args... )> {
    typedef Delegate<_Ret (*)(_Args... )> type;

    STATIC_ASSERT(std::is_pod<_Arg0>::value);
    static_assert(  sizeof(_Arg0) <= sizeof(void *),
                    "_Arg0 is too big to fit in a void *" );
    typedef union { _Arg0 typed; void *raw; } arg0_type;

    template <_Ret (*_Func)(_Arg0, _Args... )>
    static type get(_Arg0&& arg0);

    template <_Ret (*_Func)(_Arg0, _Args... )>
    static _Ret Wrapper_(void *pcallee, _Args... args );
};
//----------------------------------------------------------------------------
template <typename _Arg0, typename _Ret, typename... _Args >
template <_Ret (*_Func)(_Arg0, _Args... )>
auto BindDelegate<_Arg0, _Ret (*)(_Arg0, _Args... )>::get(_Arg0&& arg0) -> type {
    arg0_type callee = {0};
    callee.typed = std::forward<_Arg0>(arg0);
    return type(&Wrapper_<_Func>, callee.raw);
}
//----------------------------------------------------------------------------
template <typename _Arg0, typename _Ret, typename... _Args >
template <_Ret (*_Func)(_Arg0, _Args... )>
_Ret BindDelegate<_Arg0, _Ret (*)(_Arg0, _Args... )>::Wrapper_(void *pcallee, _Args... args ) {
    arg0_type arg0 = {0};
    arg0.raw = pcallee;
    return (*_Func)(arg0.typed, std::forward<_Args>(args)... );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Binary function with one trailing deferred argument :
//----------------------------------------------------------------------------
template <typename _Arg1, typename _Ret, typename _Arg0 >
struct BindDelegate<_Arg1, _Ret (*)(_Arg0, _Arg1)> {
    typedef Delegate<_Ret (*)(_Arg0)> type;

    STATIC_ASSERT(std::is_pod<_Arg1>::value);
    static_assert(  sizeof(_Arg1) <= sizeof(void *),
                    "_Arg1 is too big to fit in a void *" );
    typedef union { _Arg1 typed; void *raw; } arg1_type;

    template <_Ret (*_Func)(_Arg0, _Arg1)>
    static type get(_Arg1&& arg1);

    template <_Ret (*_Func)(_Arg0, _Arg1)>
    static _Ret Wrapper_(void *pcallee, _Arg0 arg0);
};
//----------------------------------------------------------------------------
template <typename _Arg1, typename _Ret, typename _Arg0 >
template <_Ret (*_Func)(_Arg0, _Arg1)>
auto BindDelegate<_Arg1, _Ret (*)(_Arg0, _Arg1)>::get(_Arg1&& arg1) -> type {
    arg1_type callee = {0};
    callee.typed = std::forward<_Arg1>(arg1);
    return type(&Wrapper_<_Func>, callee.raw);
}
//----------------------------------------------------------------------------
template <typename _Arg1, typename _Ret, typename _Arg0 >
template <_Ret (*_Func)(_Arg0, _Arg1)>
_Ret BindDelegate<_Arg1, _Ret (*)(_Arg0, _Arg1)>::Wrapper_(void *pcallee, _Arg0 arg0) {
    arg1_type arg1 = {0};
    arg1.raw = pcallee;
    return (*_Func)(std::forward<_Arg0>(arg0), arg1.typed);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Bare free function :
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args >
struct BindDelegate<decltype(nullptr), _Ret (*)(_Args... )> {
    typedef Delegate<_Ret (*)(_Args... )> type;

    template <_Ret (*_Func)(_Args... )>
    static type get(void *);

    template <_Ret (*_Func)(_Args... )>
    static _Ret Wrapper_(void *pcallee, _Args... args );
};
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args >
template <_Ret (*_Func)(_Args... )>
auto BindDelegate<decltype(nullptr), _Ret (*)(_Args... )>::get(void *) -> type {
    return type(&Wrapper_<_Func>, nullptr);
}
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args >
template <_Ret (*_Func)(_Args... )>
_Ret BindDelegate<decltype(nullptr), _Ret (*)(_Args... )>::Wrapper_(void *, _Args... args ) {
    return (*_Func)(std::forward<_Args>(args)... );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace details
} //!namespace Core
