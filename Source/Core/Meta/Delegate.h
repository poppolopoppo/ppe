
#pragma once

#include "Core/Meta/PointerWFlags.h"

#include <type_traits>

// Original idea :
// http://blog.coldflake.com/posts/C++-delegates-on-steroids/
//
// 4 different usages :
// * Non const member function  :  TDelegate(&Obj::NonConstMember, &obj)
// * Const member function      :  TDelegate(&Obj::ConstMember, &obj)
// * Function with argument     :  TDelegate(&FunctionWArg, arg)
// * Function without argument  :  TDelegate(&FunctionWOArg, nullptr)
//

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TDelegate {
public:
    TDelegate() { static_assert(false, "TDelegate<T> accepts only function pointers"); }
};
//----------------------------------------------------------------------------
class FBaseDelegate {
public:
    FBaseDelegate() : FBaseDelegate(nullptr, nullptr) {}
    FBaseDelegate(void* pcallback, void* pcallee)
    :   _pcallee(pcallee)
    ,   _pcallback(pcallback) {}

    FBaseDelegate(const FBaseDelegate& other) { operator =(other); }
    FBaseDelegate& operator =(const FBaseDelegate& other) {
        _pcallee = other._pcallee;
        _pcallback = other._pcallback;
        return *this;
    }

    bool Valid() const { return (nullptr != _pcallback); }
    operator void* () const { return _pcallback; }

    template <typename _Func>
    const TDelegate<_Func>& Cast() const {
        return *static_cast<const TDelegate<_Func>*>(this);
    }

    friend bool operator ==(const FBaseDelegate& lhs, const FBaseDelegate& rhs) {
        return  lhs._pcallback == rhs._pcallback &&
                lhs._pcallee == rhs._pcallee;
    }

    friend bool operator !=(const FBaseDelegate& lhs, const FBaseDelegate& rhs) {
        return (not operator ==(lhs, rhs));
    }

protected:
    void* _pcallee;
    void* _pcallback;
};
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args >
class TDelegate<_Ret (*)(_Args... )> : public FBaseDelegate {
public:
    typedef _Ret return_type;
    typedef return_type (*func_type)(_Args... );
    typedef return_type (*callback_type)(void*, _Args... );

    TDelegate() : FBaseDelegate(nullptr, nullptr) {}
    TDelegate(callback_type pcallback, void* pcallee = nullptr)
        : FBaseDelegate(reinterpret_cast<void*>(pcallback), pcallee) {}

    TDelegate(const TDelegate& other) : FBaseDelegate(other) {}
    TDelegate& operator =(const TDelegate& other) {
        FBaseDelegate::operator =(other);
        return *this;
    }

    return_type Invoke(_Args... args) const {
        Assert(Valid());
        return reinterpret_cast<callback_type>(_pcallback)(_pcallee, std::forward<_Args>(args)... );
    }

    return_type InvokeIFP(_Args... args) const {
        return (Valid() ? Invoke(std::forward<_Args>(args)...) : return_type());
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
struct TBindDelegate {};
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Delegate-inl.h"

#define _TDelegate(_pMemberOrFunc, _pCalleeOrArg0, _Template) \
    ::Core::details::TBindDelegate< \
            decltype( _pCalleeOrArg0 ) \
        ,   decltype( _pMemberOrFunc ) \
        >::_Template get< _pMemberOrFunc >( _pCalleeOrArg0 )

#define TDelegate(_pMemberOrFunc, _pCalleeOrArg0) _TDelegate(_pMemberOrFunc, _pCalleeOrArg0, )
#define TDelegateTpl(_pMemberOrFunc, _pCalleeOrArg0) _TDelegate(_pMemberOrFunc, _pCalleeOrArg0, template)

#define DelegateType(_pMemberOrFunc, _pCalleeOrArg0) \
    ::Core::details::TBindDelegate< \
            decltype( _pCalleeOrArg0 ) \
        ,   decltype( _pMemberOrFunc ) \
        >::type
