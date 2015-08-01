#pragma once

#include "Core/Meta/PointerWFlags.h"

#include <type_traits>

// Original idea :
// http://blog.coldflake.com/posts/C++-delegates-on-steroids/
//
// 4 different usages :
// * Non const member function  :  Delegate(&Obj::NonConstMember, &obj)
// * Const member function      :  Delegate(&Obj::ConstMember, &obj)
// * Function with argument     :  Delegate(&FunctionWArg, arg)
// * Function without argument  :  Delegate(&FunctionWOArg, nullptr)
//

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class Delegate {
public:
    Delegate() { static_assert(false, "Delegate<T> accepts only function pointers"); }
};
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args >
class Delegate<_Ret (*)(_Args... )> {
public:
    typedef _Ret (*func_type)(_Args... );
    typedef _Ret (*callback_type)(void *, _Args... );
    typedef _Ret return_type;

    Delegate() : Delegate(nullptr, nullptr) {}
    Delegate(callback_type pcallback, void *pcallee = nullptr)
    :   _pcallee(pcallee) {
        _pcallback.Set(pcallback);
    }

    Delegate(const Delegate& other) { operator =(other); }
    Delegate& operator =(const Delegate& other) {
        _pcallee = other._pcallee;
        _pcallback = other._pcallback;
        return *this; 
    }

    bool Valid() const { return (nullptr != _pcallback.Get()); }
    operator void *() const { return _pcallback.Get(); }

    size_t Flag01() const { return _pcallback.Flag01(); }
    void SetFlag01(size_t value) { _pcallback.SetFlag01(value); }

    bool Flag0() const { return _pcallback.Flag0(); }
    void SetFlag0(bool value) { _pcallback.SetFlag0(value); }

    bool Flag1() const { return _pcallback.Flag1(); }
    void SetFlag1(bool value) { _pcallback.SetFlag01(); }

    _Ret Invoke(_Args... args) const {
        return (*_pcallback.Get())(_pcallee, std::forward<_Args>(args)... );
    }

    FORCE_INLINE _Ret operator ()(_Args... args) const { 
        return Invoke(std::forward<_Args>(args)... ); 
    }

    friend bool operator ==(const Delegate<_Ret (*)(_Args... )>& lhs,
                            const Delegate<_Ret (*)(_Args... )>& rhs ) { 
        return  lhs._pcallback == rhs._pcallback && 
                lhs._pcallee == rhs._pcallee; 
    }

    friend bool operator !=(const Delegate<_Ret (*)(_Args... )>& lhs,
                            const Delegate<_Ret (*)(_Args... )>& rhs ) { 
        return ! operator ==(lhs, rhs);
    }

protected:
    void *_pcallee;
    Meta::PointerWFlags<callback_type> _pcallback;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace DelegateHelper {
//----------------------------------------------------------------------------
template <typename _Callee, typename _Bind >
struct Bind {};
//----------------------------------------------------------------------------
} //!namespace DelegateHelper
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Delegate-inl.h"

#define Delegate(_pMemberOrFunc, _pCalleeOrArg0) \
    ::Core::DelegateHelper::Bind< \
            decltype( _pCalleeOrArg0 ) \
        ,   decltype( _pMemberOrFunc ) \
        >::get< _pMemberOrFunc >( _pCalleeOrArg0 )

#define DelegateType(_pMemberOrFunc, _pCalleeOrArg0) \
    ::Core::DelegateHelper::Bind< \
            decltype( _pCalleeOrArg0 ) \
        ,   decltype( _pMemberOrFunc ) \
        >::type
