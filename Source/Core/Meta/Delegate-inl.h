#pragma once

#include "Delegate.h"

namespace Core {
namespace DelegateHelper {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Non const member function  :
//----------------------------------------------------------------------------
template <typename T, typename _Ret, typename... _Args >
struct Bind<T *, _Ret (T::*)(_Args... )> {

    template <_Ret (T::*_Member)(_Args... )>
    static Delegate<_Ret (*)(_Args... )> get(T *pcallee);

    template <_Ret (T::*_Member)(_Args... )>
    static _Ret Wrapper_(void *pcallee, _Args... args );
};
//----------------------------------------------------------------------------
template <typename T, typename _Ret, typename... _Args >
template <_Ret (T::*_Member)(_Args... )>
Delegate<_Ret (*)(_Args... )> Bind<T *, _Ret (T::*)(_Args... )>::get(T *pcallee) {
    return Delegate<_Ret (*)(_Args... )>(&Wrapper_<_Member>, static_cast<void *>(pcallee));
}
//----------------------------------------------------------------------------
template <typename T, typename _Ret, typename... _Args >
template <_Ret (T::*_Member)(_Args... )>
_Ret Bind<T *, _Ret (T::*)(_Args... )>::Wrapper_(void *pcallee, _Args... args ) {
    __assume(pcallee);
    T *const p = static_cast<T *>(pcallee);
    return (p->*_Member)(std::forward<_Args>(args)... );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Const member function  :
//----------------------------------------------------------------------------
template <typename T, typename _Ret, typename... _Args >
struct Bind<T *, _Ret (T::*)(_Args... ) const> {

    template <_Ret (T::*_Member)(_Args... ) const>
    static Delegate<_Ret (*)(_Args... )> get(T *pcallee);

    template <_Ret (T::*_Member)(_Args... ) const>
    static _Ret Wrapper_(void *pcallee, _Args... args );
};
//----------------------------------------------------------------------------
template <typename T, typename _Ret, typename... _Args >
template <_Ret (T::*_Member)(_Args... ) const>
Delegate<_Ret (*)(_Args... )> Bind<T *, _Ret (T::*)(_Args... ) const>::get(T *pcallee) {
    return Delegate<_Ret (*)(_Args... )>(&Wrapper_<_Member>, static_cast<void *>(pcallee));
}
//----------------------------------------------------------------------------
template <typename T, typename _Ret, typename... _Args >
template <_Ret (T::*_Member)(_Args... ) const>
_Ret Bind<T *, _Ret (T::*)(_Args... ) const>::Wrapper_(void *pcallee, _Args... args ) {
    __assume(pcallee);
    const T *p = static_cast<const T *>(pcallee);
    return (p->*_Member)(std::forward<_Args>(args)... );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Free function with one leading deferred argument :
//----------------------------------------------------------------------------
template <typename _Arg0, typename _Ret, typename... _Args >
struct Bind<_Arg0, _Ret (*)(_Arg0, _Args... )> {

    static_assert(  sizeof(_Arg0) <= sizeof(void *),
                    "_Arg0 is too big to fit in a void *" );
    typedef union { _Arg0 typed; void *raw; } arg0_type;

    template <_Ret (*_Func)(_Arg0, _Args... )>
    static Delegate<_Ret (*)(_Args... )> get(_Arg0 arg0);

    template <_Ret (*_Func)(_Arg0, _Args... )>
    static _Ret Wrapper_(void *pcallee, _Args... args );
};
//----------------------------------------------------------------------------
template <typename _Arg0, typename _Ret, typename... _Args >
template <_Ret (*_Func)(_Arg0, _Args... )>
Delegate<_Ret (*)(_Args... )> Bind<_Arg0, _Ret (*)(_Arg0, _Args... )>::get(_Arg0 arg0) {
    arg0_type callee = {0};
    callee.typed = arg0;
    return Delegate<_Ret (*)(_Args... )>(&Wrapper_<_Func>, callee.raw);
}
//----------------------------------------------------------------------------
template <typename _Arg0, typename _Ret, typename... _Args >
template <_Ret (*_Func)(_Arg0, _Args... )>
_Ret Bind<_Arg0, _Ret (*)(_Arg0, _Args... )>::Wrapper_(void *pcallee, _Args... args ) {
    arg0_type arg0 = {0};
    arg0.raw = pcallee;
    return (*_Func)(arg0.typed, std::forward<_Args>(args)... );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Bare free function :
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args >
struct Bind<decltype(nullptr), _Ret (*)(_Args... )> {

    template <_Ret (*_Func)(_Args... )>
    static Delegate<_Ret (*)(_Args... )> get(void *);

    template <_Ret (*_Func)(_Args... )>
    static _Ret Wrapper_(void *pcallee, _Args... args );
};
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args >
template <_Ret (*_Func)(_Args... )>
Delegate<_Ret (*)(_Args... )> Bind<decltype(nullptr), _Ret (*)(_Args... )>::get(void *) {
    return Delegate<_Ret (*)(_Args... )>(&Wrapper_<_Func>, nullptr);
}
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args >
template <_Ret (*_Func)(_Args... )>
_Ret Bind<decltype(nullptr), _Ret (*)(_Args... )>::Wrapper_(void *, _Args... args ) {
    return (*_Func)(std::forward<_Args>(args)... );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DelegateHelper
} //!namespace Core
