#pragma once

#include "Core/Allocator/Allocation.h"

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if 0
template <typename T>
class TFunction;
template <typename _Ret, typename... _Args>
class TFunction<_Ret(_Args...)> : FFunctionAllocator {
public:
    TFunction() = default;

    template <typename _Fx>
    TFunction(const _Fx& other) { assign(other); }

    template <typename _Fx>
    TFunction& operator =(const _Fx& other) { assign(other); return (*this); }

    bool valid() const { return (_storage != storage_type{}); }
    operator bool() const { return valid(); }

    _Ret operator()(Args&& ...args) const {
        if (!valid())
            throw std::bad_function_call();

        return pimpl_()->Invoke(std::forward<Args>(args)...);
    }

    void assign(const TFunction& other) {
        reset();
        if (other.valid())
            other.pimpl_()->Clone(_storage.data());
    }

    void assign(TFunction&& rvalue) {
        reset();
        if (other.valid())
            other.pimpl_()->Clone(_storage.data());
    }

    void assign(_Ret(*f)(_Args...)) {
        make_pimpl_(f);
    }

    void reset() {
        if (not valid()) return;
        pimpl_()->~IPimpl_();
        _storage = storage_type{};
        Assert(not valid());
    }

    void swap(TFunction& other) {
        std::swap(_storage, other._storage);
    }

private:
    typedef std::array<intptr_t, 4> storage_type;
    alignas(intptr_t) storage_type _storage;

    struct IPimpl_ {
        virtual ~IPimpl_() = default;
        virtual _Ret Invoke(Args&& ...args) const = 0;
        virtual void Copy(void* dst) const = 0;
    };

    template <typename _Fx>
    struct TPimpl_ : IPimpl_<_Fx> {
        _Fx _fx;
        TPimpl_(const _Fx& value) : _fx(value) {}
        TPimpl_(_Fx&& rvalue) : _fx(std::move(rvalue)) {}
        virtual _Ret Invoke(Args&& ...args) const override { return _fx(std::forward<_Args>(args)...); }
        virtual void Copy(void* dst) const override { new (dst) TPimpl_<_Fx>(_fx); }
    };

    template <typename _Fx>
    TPimpl_<_Fx>* make_pimpl_(const _Fx& fx) {
        if (valid())
            pimpl_()->~IPimpl_();
        return new (_storage.data()) TPimpl_<_Fx>(fx);
    }

    IPimpl_* pimpl_() { return reinterpret_cast<IPimpl_*>(_storage.data()); }
    const IPimpl_* pimpl_() const { return reinterpret_cast<const IPimpl_*>(_storage.data()); }
};
#else
template <typename T, typename _Allocator = ALLOCATOR(Functional, char)>
class TFunction : _Allocator, public std::function<T> {
public:
    typedef _Allocator allocator_type;
    typedef std::function<T> parent_type;

    using parent_type::target;
    using parent_type::target_type;
    using parent_type::operator bool;

    TFunction() = default;

    TFunction(const parent_type& other)
        : parent_type(std::allocator_arg, allocator_(), other)
    {}

    TFunction& operator =(const parent_type& other) {
        parent_type::assign(other, allocator_());
        return (*this);
    }

    TFunction(parent_type&& rvalue)
        : parent_type(std::allocator_arg, allocator_(), std::move(rvalue))
    {}

    TFunction& operator =(parent_type&& rvalue) {
        parent_type::assign(std::move(rvalue), allocator_())
        return (*this);
    }

    template <typename _Fx, class = decltype(parent_type(std::declval<_Fx>())) >
    TFunction(_Fx fx)
        : parent_type(std::allocator_arg, allocator_(), std::forward<_Fx>(fx))
    {}

    template <typename _Fx, class = decltype(std::declval<parent_type>().assign(std::declval<_Fx>())) >
    TFunction& operator =(_Fx fx) {
        parent_type::assign(std::forward<_Fx>(fx), allocator_());
        return (*this);
    }

    template<class _Fx>
    void assign(_Fx&& fx) {
        parent_type::assign(std::forward<_Fx>(fx), allocator_());
    }

    void swap(TFunction& other) {
        swap(allocator_(), other.allocator_());
        parent_type::swap(other);
    }

private:
    allocator_type& allocator_() { return static_cast<allocator_type&>(*this); }
};
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
