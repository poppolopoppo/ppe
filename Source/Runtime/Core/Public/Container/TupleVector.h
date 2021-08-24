#pragma once

#include "Container/Tuple.h"
#include "Container/Vector.h"
#include "Memory/PtrRef.h"
#include "Memory/RefPtr.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Uses the default allocator
#define TUPLEVECTOR(_DOMAIN, ...) \
    ::PPE::TTupleVector<::PPE::TTupleVectorAllocatorBroadcast<ALLOCATOR(_DOMAIN)>::type, COMMA_PROTECT(__VA_ARGS__)>
//----------------------------------------------------------------------------
// Allocates minimum N elements, useful to avoid small allocations and fragmentation
#define TUPLEVECTOR_MINSIZE(_DOMAIN, N, ...) \
    ::PPE::TTupleVector<::PPE::TTupleVectorMinSizeAllocator<MEMORYDOMAIN_TAG(_DOMAIN), N>::type, COMMA_PROTECT(__VA_ARGS__)>
//----------------------------------------------------------------------------
// Don't allocate for first N elements, use inline storage instead
#define TUPLEVECTOR_INSITU(_DOMAIN, N, ...) \
    ::PPE::TTupleVector<::PPE::TTupleVectorInlineAllocator<MEMORYDOMAIN_TAG(_DOMAIN), N>::type, COMMA_PROTECT(__VA_ARGS__)>
//----------------------------------------------------------------------------
// Uses a linear heap allocator
#define TUPLEVECTOR_LINEARHEAP(...) \
    ::PPE::TTupleVector<::PPE::TTupleVectorAllocatorBroadcast<::PPE::FLinearAllocator>::type, COMMA_PROTECT(__VA_ARGS__)>
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator>
struct TTupleVectorAllocatorBroadcast {
    template <typename T>
    using type = _Allocator;
};
template <typename _Tag, size_t N>
struct TTupleVectorInlineAllocator {
    template <typename T>
    using type = TInlineAllocator<_Tag, T, N>;
};
template <typename _Tag, size_t N>
struct TTupleVectorMinSizeAllocator {
    template <typename T>
    using type = TDefaultAllocatorMinSize<_Tag, T, N>;
};
//----------------------------------------------------------------------------
namespace details {
template <template <class> typename _Allocator, typename _Elt0, typename... _Elts>
struct TTupleVectorAllocator_ {
    template <typename T>
    using elt0 = _Elt0;
    using type = Meta::TConditional<std::is_same_v<
        std::tuple<_Allocator<_Elt0>, _Allocator<_Elts>...>,
        std::tuple<_Allocator<_Elt0>, _Allocator<elt0<_Elts>>...> >,
        _Allocator<_Elt0>,
        std::tuple<_Allocator<_Elt0>, _Allocator<_Elts>...> >;
};
} //!details
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
class TTupleVector {
public:
    STATIC_CONST_INTEGRAL(size_t, Arity, sizeof...(_Elts));
    using allocator_type = typename details::TTupleVectorAllocator_<_Allocator, _Elts...>::type;
    using value_type = TTuple<_Elts...>;
    using vectors_tuple = TTuple<TVector<_Elts, _Allocator<_Elts>>...>;
    using size_type = decltype(std::get<0>(std::declval<vectors_tuple>()).size());
    using pointer = Meta::TAddPointer<value_type>;
    using reference = Meta::TAddReference<value_type>;
    using const_pointer = Meta::TAddPointer<Meta::TAddConst<value_type>>;
    using const_reference = Meta::TAddReference<Meta::TAddConst<value_type>>;

    template <bool _Const, size_t... _Indices>
    using TElements = decltype(std::forward_as_tuple( std::get<_Indices>(
        std::declval<Meta::TAddReference<Meta::TConditional<_Const, Meta::TAddConst<vectors_tuple>, vectors_tuple>>>()).at(0)...));

    template <bool _Const, size_t... _Indices>
    class TBaseIterator : public Meta::TIterator<TElements<_Const, _Indices...>, std::random_access_iterator_tag> {
    public:
        using parent_type = Meta::TIterator<TElements<_Const, _Indices...>, std::random_access_iterator_tag>;

        using typename parent_type::iterator_category;
        using difference_type = std::ptrdiff_t;
        using typename parent_type::value_type;

        TBaseIterator() = default;

        TBaseIterator(const TBaseIterator&) = default;
        TBaseIterator& operator =(const TBaseIterator&) = default;

        template <bool _Const2>
        TBaseIterator(const TBaseIterator<_Const2, _Indices...>& other) { operator =(other); }
        template <bool _Const2>
        TBaseIterator& operator =(const TBaseIterator<_Const2, _Indices...>& other) {
            _owner = other._owner;
            _pos = other._pos;
            return (*this);
        }

        TBaseIterator& operator++() /* prefix */ { ++_pos; return *this; }
        TBaseIterator& operator--() /* prefix */ { --_pos; return *this; }

        TBaseIterator operator++(int) /* postfix */ { const auto r{*this}; ++_pos; return r; }
        TBaseIterator operator--(int) /* postfix */ { const auto r{*this}; --_pos; return r; }

        TBaseIterator& operator+=(difference_type n) { _pos += n; Assert(_pos <= _owner->size()); return *this; }
        TBaseIterator& operator-=(difference_type n) { _pos -= n; Assert(_pos <= _owner->size()); return *this; }

        TBaseIterator operator+(difference_type n) const { return { _owner, _pos + n }; }
        TBaseIterator operator-(difference_type n) const { return { _owner, _pos - n }; }

        TElements<_Const, _Indices...> operator*() const { return _owner->Aggregate_(_pos, std::index_sequence<_Indices...>{}); }
        //pointer operator->() const { return &_owner->Aggregate(_pos); }

        TElements<_Const, _Indices...> operator[](difference_type n) const { return *(*this + n); }

        template <bool _Const2>
        difference_type operator-(const TBaseIterator<_Const2, _Indices...>& other) const { return (checked_cast<difference_type>(_pos) - other._pos); }

        template <bool _Const2>
        bool operator==(const TBaseIterator<_Const2, _Indices...>& other) const { Assert(_owner == other._owner); return (_pos == other._pos); }
        template <bool _Const2>
        bool operator!=(const TBaseIterator<_Const2, _Indices...>& other) const { return not operator ==(other); }

        template <bool _Const2>
        bool operator< (const TBaseIterator<_Const2, _Indices...>& other) const { Assert(_owner == other._owner); return (_pos < other._pos); }
        template <bool _Const2>
        bool operator>=(const TBaseIterator<_Const2, _Indices...>& other) const { return not operator <(other); }

        template <bool _Const2>
        bool operator> (const TBaseIterator<_Const2, _Indices...>& other) const { Assert(_owner == other._owner); return (_pos > other._pos); }
        template <bool _Const2>
        bool operator<=(const TBaseIterator<_Const2, _Indices...>& other) const { return not operator >(other); };

        friend void swap(TBaseIterator& lhs, TBaseIterator& rhs) NOEXCEPT {
            std::swap(lhs._owner, rhs._owner);
            std::swap(lhs._pos, rhs._pos);
        }

    private:
        using owner_pointer = Meta::TAddPointer<Meta::TConditional<_Const, const TTupleVector, TTupleVector>>;
        owner_pointer _owner;
        size_type _pos;

        friend class TTupleVector;
        template <bool _Const2, size_t... _Indices2>
        friend class TBaseIterator;
        TBaseIterator(owner_pointer owner, size_type pos) : _owner(owner), _pos(pos) {}
    };
    template <size_t... _Indices>
    using TIterator = TBaseIterator<false, _Indices...>;
    template <size_t... _Indices>
    using TConstIterator = TBaseIterator<true, _Indices...>;

    using iterator = Meta::expand_indices_for<TIterator, _Elts...>;
    using const_iterator = Meta::expand_indices_for<TConstIterator, _Elts...>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    TTupleVector() = default;

    explicit TTupleVector(Meta::FForceInit init) NOEXCEPT : TTupleVector(MakeBroadcast_(init)) {}

    explicit TTupleVector(const allocator_type& alloc) : TTupleVector(MakeBroadcast_(alloc)) {}

    explicit TTupleVector(size_type count) : TTupleVector(MakeBroadcast_(count)) {}
    TTupleVector(size_type count, const_reference value) : TTupleVector(MakeBroadcast_(count, value)) {}
    TTupleVector(size_type count, const allocator_type& alloc) : TTupleVector(MakeBroadcast_(count, alloc)) {}
    TTupleVector(size_type count, const_reference value, const allocator_type& alloc) : TTupleVector(MakeBroadcast_(count, value, alloc)) {}

    TTupleVector(const TTupleVector& other) : _vectors(other._vectors) {}
    TTupleVector(const TTupleVector& other, const allocator_type& alloc) : TTupleVector(MakeBroadcast_(other._vectors, alloc)) {}
    TTupleVector& operator=(const TTupleVector& other) { _vectors = other._vectors; return (*this); }

    TTupleVector(TTupleVector&& rvalue) NOEXCEPT : _vectors(std::move(rvalue)) {}
    TTupleVector(TTupleVector&& rvalue, const allocator_type& alloc) NOEXCEPT : TTupleVector(MakeBroadcast_(std::move(rvalue._vectors), alloc)) {}
    TTupleVector& operator=(TTupleVector&& rvalue) NOEXCEPT { _vectors = std::move(rvalue._vectors); return (*this); }

    TTupleVector(std::initializer_list<value_type> ilist) : TTupleVector() { assign(ilist.begin(), ilist.end()); }
    TTupleVector(std::initializer_list<value_type> ilist, const allocator_type& alloc) : TTupleVector(alloc) { assign(ilist.begin(), ilist.end()); }
    TTupleVector& operator=(std::initializer_list<value_type> ilist) { assign(ilist.begin(), ilist.end()); return *this; }

    TTupleVector(const TMemoryView<const value_type>& view) : TTupleVector() { assign(view.begin(), view.end()); }
    TTupleVector(const TMemoryView<const value_type>& view, const allocator_type& alloc) : TTupleVector(alloc) { assign(view.begin(), view.end()); }
    TTupleVector& operator=(const TMemoryView<const value_type>& view) { assign(view.begin(), view.end()); return *this; }

    template <typename _It>
    TTupleVector(TIterable<_It> range) : TTupleVector() { assign(range); }
    template <typename _It>
    TTupleVector& operator=(TIterable<_It> range) { assign(range); return *this; }

    template <template <class> typename _OtherAllocator>
    TTupleVector(const TTupleVector<_OtherAllocator, _Elts...>& other) : TTupleVector() { operator =(other); }
    template <template <class> typename _OtherAllocator>
    TTupleVector& operator =(const TTupleVector<_OtherAllocator, _Elts...>& other) { _vectors = other._vectors; return (*this); }

    template <template <class> typename _OtherAllocator>
    TTupleVector(TTupleVector<_OtherAllocator, _Elts...>&& rvalue) : TTupleVector() { operator =(std::move(rvalue)); }
    template <template <class> typename _OtherAllocator>
    TTupleVector& operator =(TTupleVector<_OtherAllocator, _Elts...>&& rvalue) { _vectors = std::move(rvalue._vectors); return (*this); }

    template <typename _It>
    TTupleVector(_It first, _It last) : TTupleVector() { assign(first, last); }
    template <typename _It>
    TTupleVector(_It first, _It last, const allocator_type& alloc) : TTupleVector(alloc) { assign(first, last); }

    TTupleVector(std::initializer_list<_Elts>... ilist) : TTupleVector(MakeBroadcast_(ForwardAsTuple(ilist...))) {}
    TTupleVector(const allocator_type& alloc, std::initializer_list<_Elts>... ilist) : TTupleVector(MakeBroadcast_(ForwardAsTuple(ilist...), alloc)) {}

    TTupleVector(const TMemoryView<const _Elts>&... view) : TTupleVector(MakeBroadcast_(ForwardAsTuple(view...))) {}
    TTupleVector(const allocator_type& alloc, const TMemoryView<const _Elts>&... view) : TTupleVector(MakeBroadcast_(ForwardAsTuple(view...), alloc)) {}

    size_type size() const { return std::get<0>(_vectors).size(); }
    size_type capacity() const { return std::get<0>(_vectors).capacity(); }
    size_type max_size() const { return std::numeric_limits<size_type>::max(); }
    bool empty() const { return std::get<0>(_vectors).empty(); }

    iterator begin() { return { this, 0 }; }
    iterator end() { return { this, size() }; }

    const_iterator begin() const { return { this , 0 }; }
    const_iterator end() const { return { this, size() }; }

    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }

    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

    auto at(size_type pos) { return Aggregate_(pos, std::make_index_sequence<Arity>{}); }
    auto at(size_type pos) const { return Aggregate_(pos, std::make_index_sequence<Arity>{}); }

    auto operator[](size_type pos) { return at(pos); }
    auto operator[](size_type pos) const { return at(pos); }

    auto front() { return at(0); }
    auto front() const { return at(0); }

    auto back() { return at(size() - 1); }
    auto back() const { return at(size() - 1); }

    template <typename _It>
    Meta::TEnableIf<Meta::is_iterator_v<_It>> assign(_It first, _It last);
    void assign(size_type count, const value_type& value);
    void assign(std::initializer_list<value_type> ilist) { assign(ilist.begin(), ilist.end()); }
    void assign(TTupleVector&& rvalue) { _vectors = std::move(rvalue._vectors); }

    template <typename _It>
    auto assign(TIterable<_It> range) { return assign(range.begin(), range.end()); }

    template <typename U>
    void assign(const TMemoryView<U>& view) { assign(view.begin(), view.end()); }
    void assign(const TMemoryView<value_type>& view) { assign(std::make_move_iterator(view.begin()), std::make_move_iterator(view.end())); }
    void assign(const TMemoryView<const value_type>& view) { assign(view.begin(), view.end()); }

    template <class... _Args>
    iterator emplace(const_iterator pos, _Args&&... args);

    template <class... _Args>
    void emplace_back(_Args&&... args);
    void emplace_back(const value_type& value);
    void emplace_back(value_type&& rvalue);

    template <class... _Args>
    void emplace_back_AssumeNoGrow(_Args&&... args);
    void emplace_back_AssumeNoGrow(const value_type& value);
    void emplace_back_AssumeNoGrow(value_type&& rvalue);

    iterator erase(const_iterator pos);
    iterator erase(const_iterator first, const_iterator last);
    void erase_DontPreserveOrder(const_iterator pos);

    template <typename _It>
    Meta::TEnableIf<Meta::is_iterator_v<_It>> insert(const_iterator pos, _It first, _It last);
    iterator insert(const_iterator pos, const value_type& value);
    iterator insert(const_iterator pos, value_type&& rvalue);
    iterator insert(const_iterator pos, size_type count, const value_type& value);
    iterator insert(const_iterator pos, std::initializer_list<value_type> ilist) { return insert(pos, ilist.begin(), ilist.end()); }

    template <typename _It>
    auto insert(const_iterator pos, TIterable<_It> range) { return insert(pos, range.begin(), range.end()); }

    void push_back(const value_type& value) { emplace_back(value); }
    void push_back(value_type&& rvalue) { emplace_back(std::move(rvalue)); }
    reference push_back_Default() { emplace_back(); return back(); }
    void push_back_AssumeNoGrow(const value_type& value);
    void push_back_AssumeNoGrow(value_type&& rvalue);

    void pop_back();
    value_type pop_back_ReturnBack();

    void clear();
    void clear_ReleaseMemory();
    void reserve(size_type count);
    void reserve_AtLeast(size_type count);
    void reserve_Additional(size_type count);
    void reserve_AssumeEmpty(size_type count);
    void reserve_Exactly(size_type count);
    void resize(size_type count);
    void resize(size_type count, const_reference value);
    void resize_Uninitialized(size_type count);
    void resize_AssumeEmpty(size_type count);
    void resize_AssumeEmpty(size_type count, const_reference value);
    void shrink_to_fit();

    void swap(TTupleVector& other) NOEXCEPT { std::swap(_vectors, other._vectors); }
    friend void swap(TTupleVector& lhs, TTupleVector& rhs) NOEXCEPT { lhs.swap(rhs); }

    template <size_t _Idx>
    const auto& get() const NOEXCEPT { return std::get<_Idx>(_vectors); }

    template <size_t... _Indices>
    auto Aggregate(size_type pos) NOEXCEPT { return Aggregate_(pos, std::index_sequence<_Indices...>{}); }
    template <size_t... _Indices>
    auto Aggregate(size_type pos) const NOEXCEPT { return Aggregate_(pos, std::index_sequence<_Indices...>{}); }

    template <size_t _Idx>
    TMemoryView<std::tuple_element_t<_Idx, value_type>> MakeView() const { return std::get<_Idx>(_vectors).MakeView(); }
    template <size_t _Idx>
    TMemoryView<Meta::TAddConst<std::tuple_element_t<_Idx, value_type>>> MakeConstView() const { return std::get<_Idx>(_vectors).MakeConstView(); }

    template <size_t... _Indices>
    auto MakeIterable() NOEXCEPT {
        return PPE::MakeIterable( TIterator<_Indices...>{ this, 0 }, TIterator<_Indices...>{ this, size() } );
    }
    template <size_t... _Indices>
    auto MakeIterable() const NOEXCEPT {
        return PPE::MakeIterable( TConstIterator<_Indices...>{ this, 0 }, TConstIterator<_Indices...>{ this, size() } );
    }

    bool CheckInvariants() const;

    bool AliasesToContainer(const iterator& it) const { return (it >= begin() && it < end()); }
    bool AliasesToContainer(const const_iterator& it) const { return (it >= begin() && it < end()); }

    friend hash_t hash_value(const TTupleVector& v) NOEXCEPT {
        return Meta::static_for<Arity>([&v](auto... idx) NOEXCEPT {
            return hash_tuple(std::get<idx>(v._vectors)...);
        });
    }

private:
    vectors_tuple _vectors;

    // heavy boiler plate ahead for forwarding constructor args

    template <template <class> typename _OtherAllocator, typename... OtherElts>
    friend class TTupleVector;

    template <size_t... _Indices>
    TElements<false, _Indices...> Aggregate_(size_type pos, std::index_sequence<_Indices...>) NOEXCEPT {
        return std::forward_as_tuple(std::get<_Indices>(_vectors).at(pos)...);
    }
    template <size_t... _Indices>
    TElements<true, _Indices...> Aggregate_(size_type pos, std::index_sequence<_Indices...>) const NOEXCEPT {
        return std::forward_as_tuple(std::get<_Indices>(_vectors).at(pos)...);
    }
    template <size_t _Idx, typename T>
    static auto BroadcastElt_(T value) NOEXCEPT {
        return std::forward<T>(value);
    }
    template <size_t _Idx, typename T>
    static auto BroadcastElt_(TTupleVector&& rvalue) NOEXCEPT {
        return std::get<_Idx>(rvalue._vectors);
    }
    template <size_t _Idx, typename T>
    static auto BroadcastElt_(const TTupleVector& other) NOEXCEPT {
        return std::get<_Idx>(other._vectors);
    }
    template <size_t _Idx, typename... _Args>
    static auto BroadcastElt_(TTuple<_Args...>& tuple, Meta::TEnableIf<sizeof...(_Args) == sizeof...(_Elts)>* = nullptr) NOEXCEPT {
        STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Elts));
        return std::get<_Idx>(tuple);
    }
    template <size_t _Idx, typename... _Args>
    static auto BroadcastElt_(const TTuple<_Args...>& tuple, Meta::TEnableIf<sizeof...(_Args) == sizeof...(_Elts)>* = nullptr) NOEXCEPT {
        STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Elts));
        return std::get<_Idx>(tuple);
    }
    template <typename... _Args>
    struct TBroadcast_ {
        decltype(std::forward_as_tuple(std::declval<_Args>()...)) Args;
        template <size_t _Idx, size_t... N>
        auto Element(std::index_sequence<N...>) {
            return std::tuple_element_t<_Idx, vectors_tuple>(
                BroadcastElt_<_Idx>(std::get<N>(Args))...
            );
        }
    };
    template <typename... _Args, size_t... _Indices>
    TTupleVector(TBroadcast_<_Args...>&& broadcast, std::index_sequence<_Indices...>)
    :   _vectors(broadcast.template Element<_Indices>(std::index_sequence_for<_Args...>{})...)
    {}
    template <typename... _Args>
    TTupleVector(TBroadcast_<_Args...>&& broadcast)
    :   TTupleVector(std::move(broadcast), std::index_sequence_for<_Elts...>{})
    {}
    template <typename... _Args>
    TBroadcast_<_Args...> MakeBroadcast_(_Args&&... args) NOEXCEPT {
        return { std::forward_as_tuple(std::forward<_Args>(args)...) };
    }
};
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::assign(size_type count, const value_type& value) {
    Meta::static_for<Arity>([this, count, &value](auto... idx) {
         FOLD_EXPR( std::get<idx>(_vectors).assign(count, std::get<idx>(value)) );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
template <typename _It>
Meta::TEnableIf<Meta::is_iterator_v<_It>> TTupleVector<_Allocator, _Elts...>::assign(_It first, _It last) {
    Meta::static_for<Arity>([this, first, last](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).assign(MakeOutputIterable(first, last, TTupleGet<idx, _Elts...>{})) );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
template <class ... _Args>
auto TTupleVector<_Allocator, _Elts...>::emplace(const_iterator pos, _Args&&... args) -> iterator {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Elts));
    return Meta::static_for<Arity>([&, this, pos](auto... idx) -> iterator {
        const size_type off = std::distance(cbegin(), pos);
        auto argsTuple = std::forward_as_tuple(std::forward<_Args>(args)...);
        FOLD_EXPR( std::get<idx>(_vectors).emplace(std::get<idx>(_vectors) + off, std::get<idx>(argsTuple)) );
        return begin() + off;
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::emplace_back(const value_type& value) {
    Meta::static_for<Arity>([this, &value](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).emplace_back(std::get<idx>(value)) );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::emplace_back(value_type&& rvalue) {
    Meta::static_for<Arity>([this, &rvalue](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).emplace_back(std::move(std::get<idx>(rvalue))) );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
template <class ... _Args>
void TTupleVector<_Allocator, _Elts...>::emplace_back(_Args&&... args) {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Elts));
    Meta::static_for<Arity>([&, this](auto... idx) {
        auto argsTuple = std::forward_as_tuple(std::forward<_Args>(args)...);
        FOLD_EXPR( std::get<idx>(_vectors).emplace_back(std::move(std::get<idx>(argsTuple))) );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::emplace_back_AssumeNoGrow(const value_type& value) {
    Meta::static_for<Arity>([this, &value](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).emplace_back_AssumeNoGrow(std::get<idx>(value)) );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::emplace_back_AssumeNoGrow(value_type&& rvalue) {
    Meta::static_for<Arity>([this, &rvalue](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).emplace_back_AssumeNoGrow(std::move(std::get<idx>(rvalue))) );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
template <class ... _Args>
void TTupleVector<_Allocator, _Elts...>::emplace_back_AssumeNoGrow(_Args&&... args) {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Elts));
    Meta::static_for<Arity>([&, this](auto... idx) {
        auto argsTuple = std::forward_as_tuple(std::forward<_Args>(args)...);
        FOLD_EXPR( std::get<idx>(_vectors).emplace_back_AssumeNoGrow(std::move(std::get<idx>(argsTuple))) );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
auto TTupleVector<_Allocator, _Elts...>::erase(const_iterator pos) -> iterator {
    return Meta::static_for<Arity>([this, pos](auto... idx) -> iterator {
        const size_type off = std::distance(cbegin(), pos);
        FOLD_EXPR( std::get<idx>(_vectors).erase(std::get<idx>(_vectors).begin() + off) );
        return begin() + off;
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
auto TTupleVector<_Allocator, _Elts...>::erase(const_iterator first, const_iterator last) -> iterator {
    return Meta::static_for<Arity>([this, first, last](auto... idx) -> iterator {
        const size_type off = std::distance(cbegin(), first);
        const size_type count = std::distance(first, last);
        FOLD_EXPR( std::get<idx>(_vectors).erase(
            std::get<idx>(_vectors).begin() + off,
            std::get<idx>(_vectors).begin() + (off + count) ));
        return begin() + off;
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::erase_DontPreserveOrder(const_iterator pos) {
    Meta::static_for<Arity>([this, pos](auto... idx) {
        const size_type off = std::distance(cbegin(), pos);
        FOLD_EXPR( std::get<idx>(_vectors).erase_DontPreserveOrder(std::get<idx>(_vectors).begin() + off) );
        return begin() + off;
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
auto TTupleVector<_Allocator, _Elts...>::insert(const_iterator pos, size_type count, const value_type& value) -> iterator {
    return Meta::static_for<Arity>([this, pos, count, &value](auto... idx) -> iterator {
        const size_type off = std::distance(cbegin(), pos);
        FOLD_EXPR( std::get<idx>(_vectors).insert(
            std::get<idx>(_vectors).begin() + off, count, std::get<idx>(value)) );
        return begin() + off;
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
template <typename _It>
Meta::TEnableIf<Meta::is_iterator_v<_It>> TTupleVector<_Allocator, _Elts...>::insert(const_iterator pos, _It first, _It last) {
    Meta::static_for<Arity>([this, pos, first, last](auto... idx) {
        const size_type off = std::distance(cbegin(), pos);
        FOLD_EXPR( std::get<idx>(_vectors).insert(
            std::get<idx>(_vectors).begin() + off,
            MakeOutputIterator(first, TTupleGet<idx, _Elts...>{}),
            MakeOutputIterator(last, TTupleGet<idx, _Elts...>{}) ) );
    });
}
//----------------------------------------------------------------------------
template <template <class> class _Allocator, typename ... _Elts>
auto TTupleVector<_Allocator, _Elts...>::insert(const_iterator pos, const value_type& value) -> iterator {
    return Meta::static_for<Arity>([this, pos, &value](auto... idx) -> iterator {
        const size_type off = std::distance(cbegin(), pos);
        FOLD_EXPR( std::get<idx>(_vectors).insert(
            std::get<idx>(_vectors).begin() + off, std::get<idx>(value)) );
        return begin() + off;
    });
}
//----------------------------------------------------------------------------
template <template <class> class _Allocator, typename ... _Elts>
auto TTupleVector<_Allocator, _Elts...>::insert(const_iterator pos, value_type&& rvalue) -> iterator {
    return Meta::static_for<Arity>([this, pos, &rvalue](auto... idx) -> iterator {
        const size_type off = std::distance(cbegin(), pos);
        FOLD_EXPR( std::get<idx>(_vectors).insert(
            std::get<idx>(_vectors).begin() + off, std::move(std::get<idx>(rvalue))) );
        return begin() + off;
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::push_back_AssumeNoGrow(value_type&& rvalue) {
    Meta::static_for<Arity>([this, &rvalue](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).push_back_AssumeNoGrow(std::move(std::get<idx>(rvalue))) );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::push_back_AssumeNoGrow(const value_type& value) {
    Meta::static_for<Arity>([this, &value](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).push_back_AssumeNoGrow(std::get<idx>(value)) );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::pop_back() {
    Meta::static_for<Arity>([this](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).pop_back() );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
auto TTupleVector<_Allocator, _Elts...>::pop_back_ReturnBack() -> value_type {
    return Meta::static_for<Arity>([this](auto... idx) -> value_type {
        return { std::get<idx>(_vectors).pop_back_ReturnBack()... };
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::clear() {
    Meta::static_for<Arity>([this](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).clear() );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::clear_ReleaseMemory() {
    Meta::static_for<Arity>([this](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).clear_ReleaseMemory() );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::reserve(size_type count) {
    Meta::static_for<Arity>([this, count](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).reserve(count) );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::reserve_Additional(size_type count) {
    Meta::static_for<Arity>([this, count](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).reserve_Additional(count) );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::reserve_AtLeast(size_type count) {
    Meta::static_for<Arity>([this, count](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).reserve_AtLeast(count) );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::reserve_Exactly(size_type count) {
    Meta::static_for<Arity>([this, count](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).reserve_Exactly(count) );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::reserve_AssumeEmpty(size_type count) {
    Meta::static_for<Arity>([this, count](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).reserve_AssumeEmpty(count) );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::resize(size_type count) {
    Meta::static_for<Arity>([this, count](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).resize(count) );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::resize(size_type count, const_reference value) {
    Meta::static_for<Arity>([this, count, &value](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).resize(count, std::get<idx>(value)) );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::resize_Uninitialized(size_type count) {
    Meta::static_for<Arity>([this, count](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).resize_Uninitialized(count) );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::resize_AssumeEmpty(size_type count) {
    Meta::static_for<Arity>([this, count](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).resize_AssumeEmpty(count) );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::resize_AssumeEmpty(size_type count, const_reference value) {
    Meta::static_for<Arity>([this, count, &value](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).resize_AssumeEmpty(count, std::get<idx>(value)) );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
void TTupleVector<_Allocator, _Elts...>::shrink_to_fit() {
    Meta::static_for<Arity>([this](auto... idx) {
        FOLD_EXPR( std::get<idx>(_vectors).shrink_to_fit() );
    });
}
//----------------------------------------------------------------------------
template <template <class> typename _Allocator, typename... _Elts>
bool TTupleVector<_Allocator, _Elts...>::CheckInvariants() const {
#if USE_PPE_ASSERT
    return Meta::static_for<Arity>([this](auto... idx) -> bool {
        return (true && ... && std::get<idx>(_vectors).CheckInvariants());
    });
#else
    return true;
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
