#pragma once

#include "Core.h"

#include "Container/SparseArray.h"
#include "Misc/Function.h"

#define PUBLIC_EVENT(_NAME, _DELEGATE) \
    private: \
        ::PPE::TEvent<_DELEGATE> CONCAT(_, _NAME); \
    public: \
        ::PPE::TPublicEvent<_DELEGATE>& _NAME() { \
            return CONCAT(_, _NAME).Public(); \
        }

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TEvent<> is equivalent to C# events
// FEventHandle handles registration lifetime
// TPublicEvent<> is the public interface, while TEvent<> allows full control
//----------------------------------------------------------------------------
template <typename _Delegate>
class TPublicEvent;
//----------------------------------------------------------------------------
class FEventHandle {
    template <typename _Delegate>
    friend class TPublicEvent;

public:
    FEventHandle() NOEXCEPT : _id(0) {}
    explicit FEventHandle(FSparseDataId id)
        : _id(id) {
        Assert(_id);
    }

    FEventHandle(const FEventHandle&) = delete;
    FEventHandle& operator =(const FEventHandle&) = delete;

    FEventHandle(FEventHandle&& rvalue)
        : FEventHandle() {
        Swap(rvalue);
    }

    FEventHandle& operator =(FEventHandle&& rvalue) {
        Assert(0 == _id); // don't support assigning to initialized handle !
        Swap(rvalue);
        return (*this);
    }

    ~FEventHandle() {
        Assert_NoAssume(0 == _id);
    }

    PPE_FAKEBOOL_OPERATOR_DECL() { return (_id ? this : nullptr); }

    void Forget() {
        _id = 0; // won't asset on destruction, use wisely ;O
    }

    void Swap(FEventHandle& other) {
        std::swap(_id, other._id);
    }

    inline friend void swap(FEventHandle& lhs, FEventHandle& rhs) {
        lhs.Swap(rhs);
    }

private:
    FSparseDataId _id;
};
//----------------------------------------------------------------------------
template <typename _Delegate>
class TPublicEvent {
public:
    using FDelegate = _Delegate;
    using FHandle = FEventHandle;
    using FInvocationList = SPARSEARRAY_INSITU(Event, FDelegate, 8);

    TPublicEvent() NOEXCEPT {}

    TPublicEvent(const TPublicEvent&) = delete;
    TPublicEvent& operator =(const TPublicEvent&) = delete;

    TPublicEvent(TPublicEvent&&) = delete;
    TPublicEvent& operator =(TPublicEvent&&) = delete;

    FHandle Add(FDelegate&& func) {
        Assert(func);
        return FHandle(_delegates.Emplace(std::move(func)));
    }

    void Emplace(FDelegate&& func) {
        Assert(func);
        _delegates.Emplace(std::move(func));
    }

    void Remove(FHandle& handle) {
        Assert(handle);
        VerifyRelease(_delegates.Remove(handle._id));
        handle._id = 0;
    }

protected:
    FInvocationList _delegates;
};
//----------------------------------------------------------------------------
template <typename T>
class TEvent;
template <typename _Ret, typename... _Args>
class TEvent< TFunction<_Ret(_Args...)> > : public TPublicEvent< TFunction<_Ret(_Args...)> > {
public:
    using parent_type = TPublicEvent< TFunction<_Ret(_Args...)> >;

    using typename parent_type::FDelegate;
    using typename parent_type::FHandle;
    using typename parent_type::FInvocationList;

    TEvent() NOEXCEPT : parent_type() {}

    using parent_type::Add;
    using parent_type::Remove;

    parent_type& Public() { return *this; }

    PPE_FAKEBOOL_OPERATOR_DECL() {
        return (_delegates.empty() ? nullptr : this);
    }

    void operator ()(_Args... args) const {
        Invoke(std::forward<_Args>(args)...);
    }

    void Invoke(_Args... args) const {
        for (auto& it : _delegates) {
            it(std::forward<_Args>(args)...);
        }
    }

    void Clear() {
        _delegates.Clear();
    }

private:
    using parent_type::_delegates;
};
//----------------------------------------------------------------------------
template <typename _Delegate, typename T, class = Meta::TEnableIf<_Delegate::template is_callable_v<T>> >
TPublicEvent<_Delegate>& operator <<(TPublicEvent<_Delegate>& publicEvent, T&& callback) {
    publicEvent.Emplace(std::move(callback));
    return publicEvent;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
