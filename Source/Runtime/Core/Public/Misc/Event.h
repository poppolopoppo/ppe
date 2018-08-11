#pragma once

#include "Core.h"

#include "Container/SparseArray.h"
#include "Misc/Function.h"

#define PUBLIC_EVENT(_NAME, ...) \
    private: \
        ::PPE::TEvent<__VA_ARGS__> CONCAT(_, _NAME); \
    public: \
        ::PPE::TPublicEvent<__VA_ARGS__>& _NAME() { \
            return CONCAT(_, _NAME).Public(); \
        }

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TEvent<> is equivalent to C# events
// TEventHandle<> handles registration lifetime
// TPublicEvent<> is the public interface, while TEvent<> allows full control
//----------------------------------------------------------------------------
template <typename T>
class TPublicEvent;
//----------------------------------------------------------------------------
template <typename T>
class TEventHandle {
    template <>
    friend class TPublicEvent<T>;

public:
    TEventHandle() NOEXCEPT : _id(0) {}
    explicit TEventHandle(FSparseDataId id) NOEXCEPT
        : _id(id) {
        Assert(_id);
    }

    TEventHandle(const TEventHandle&) = delete;
    TEventHandle& operator =(const TEventHandle&) = delete;

    TEventHandle(TEventHandle&& rvalue)
        : TEventHandle() {
        Swap(rvalue);
    }

    TEventHandle& operator =(const TEventHandle&) {
        Assert(0 == _id); // don't support assigning to initialized handle !
        Swap(rvalue);
        return (*this);
    }

    ~TEventHandle() {
        Assert_NoAssume(0 == _id);
    }

    PPE_FAKEBOOL_OPERATOR_DECL() { return (_id ? this : nullptr); }

    void Swap(TEventHandle& other) {
        std::swap(_id, other._id);
    }

    inline friend bool swap(TEventHandle& lhs, TEventHandle& rhs) {
        lhs.Swap(rhs);
    }

private:
    FSparseDataId _id;
};
//----------------------------------------------------------------------------
template <typename T>
class TPublicEvent {
    using FDelegate = TFunction<T>;
    using FHandle = TEventHandle<T>;
    using FInvocationList = SPARSEARRAY(Event, FDelegate, 8);

    TPublicEvent() NOEXCEPT {}

    TPublicEvent(const TPublicEvent&) = delete;
    TPublicEvent& operator =(const TPublicEvent&) = delete;

    TPublicEvent(TPublicEvent&&) = delete;
    TPublicEvent& operator =(TPublicEvent&&) = delete;

    FHandle Add(FDelegate&& func) {
        Assert(func);
        return FHandle{ this, _delegates.Emplace(std::move(func)) };
    }

    void Remove(FHandle& handle) {
        Assert(handle);
        VerifyRelease(_delegates.Remove(handle));
        handle._id = 0;
    }

protected:
    FInvocationList _delegates;
};
//----------------------------------------------------------------------------
template <typename T>
class TEvent : public TPublicEvent<T> {
public:
    using parent_type = TPublicEvent<T>;

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

    template <typename... _Args>
    void operator ()(_Args... args) const {
        return Invoke(std::forward<_Args>(args)...);
    }

    template <typename... _Args>
    void Invoke(_Args&&... args) {
        for (FDelegate& it : _delegates) {
            it(std::forward<_Args>(args)...);
        }
    }

    void Clear() {
        _delegates.Clear();
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
