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
class BaseDelegate {
public:
    BaseDelegate() : BaseDelegate(nullptr, nullptr) {}
    BaseDelegate(void* pcallback, void*pcallee)
    :   _pcallee(pcallee)
    ,   _pcallback(pcallback) {}

    BaseDelegate(const BaseDelegate& other) { operator =(other); }
    BaseDelegate& operator =(const BaseDelegate& other) {
        _pcallee = other._pcallee;
        _pcallback = other._pcallback;
        return *this;
    }

    bool Valid() const { return (nullptr != _pcallback); }
    operator void* () const { return _pcallback; }

    template <typename _Func>
    const Delegate<_Func>& Cast() const {
        return *static_cast<const Delegate<_Func>*>(this);
    }

    friend bool operator ==(const BaseDelegate& lhs, const BaseDelegate& rhs) {
        return  lhs._pcallback == rhs._pcallback &&
                lhs._pcallee == rhs._pcallee;
    }

    friend bool operator !=(const BaseDelegate& lhs, const BaseDelegate& rhs) {
        return (not operator ==(lhs, rhs));
    }

protected:
    void* _pcallee;
    void* _pcallback;
};
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args >
class Delegate<_Ret (*)(_Args... )> : public BaseDelegate {
public:
    typedef _Ret return_type;
    typedef return_type (*func_type)(_Args... );
    typedef return_type (*callback_type)(void*, _Args... );

    Delegate() : BaseDelegate(nullptr, nullptr) {}
    Delegate(callback_type pcallback, void* pcallee = nullptr)
        : BaseDelegate(reinterpret_cast<void*>(pcallback), pcallee) {}

    Delegate(const Delegate& other) : BaseDelegate(other) {}
    Delegate& operator =(const Delegate& other) {
        BaseDelegate::operator =(other);
        return *this;
    }

    return_type Invoke(_Args... args) const {
        Assert(Valid());
        return reinterpret_cast<callback_type>(_pcallback)(_pcallee, std::forward<_Args>(args)... );
    }

    FORCE_INLINE return_type operator ()(_Args... args) const {
        return Invoke(std::forward<_Args>(args)... );
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename _Callee, typename _Bind >
struct BindDelegate {};
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Delegate-inl.h"

#define Delegate(_pMemberOrFunc, _pCalleeOrArg0) \
    ::Core::details::BindDelegate< \
            decltype( _pCalleeOrArg0 ) \
        ,   decltype( _pMemberOrFunc ) \
        >::get< _pMemberOrFunc >( _pCalleeOrArg0 )

#define DelegateType(_pMemberOrFunc, _pCalleeOrArg0) \
    ::Core::details::BindDelegate< \
            decltype( _pCalleeOrArg0 ) \
        ,   decltype( _pMemberOrFunc ) \
        >::type
