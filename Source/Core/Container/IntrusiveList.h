#pragma once

#include "Core/Core.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define INTRUSIVELIST_ACCESSOR(_Member) \
    ::Core::details::TIntrusiveListTraits< decltype(_Member) >::accessor< _Member >::type
//----------------------------------------------------------------------------
#define INTRUSIVELIST(_Member) \
    ::Core::details::TIntrusiveListTraits< decltype(_Member) >::list< _Member >::type
//----------------------------------------------------------------------------
#define INTRUSIVESINGLELIST_ACCESSOR(_Member) \
    ::Core::details::TIntrusiveSingleListTraits< decltype(_Member) >::accessor< _Member >::type
//----------------------------------------------------------------------------
#define INTRUSIVESINGLELIST(_Member) \
    ::Core::details::TIntrusiveSingleListTraits< decltype(_Member) >::list< _Member >::type
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct TIntrusiveListNode {
    T* Next = nullptr;
    T* Prev = nullptr;
};
//----------------------------------------------------------------------------
template <typename T>
struct TIntrusiveSingleListNode {
    T* Next = nullptr;
};
//----------------------------------------------------------------------------
namespace details {
template <typename T, TIntrusiveListNode<T> T::*_Member>
struct TIntrusiveListAccessor;
template <typename T, TIntrusiveSingleListNode<T> T::*_Member>
struct TIntrusiveSingleListAccessor;
} //!details
//----------------------------------------------------------------------------
template <typename T, TIntrusiveListNode<T> T::*_Member>
class TIntrusiveList {
public:
    typedef details::TIntrusiveListAccessor<T, _Member> traits_type;
    typedef typename traits_type::node_type node_type;

    TIntrusiveList() : _head(nullptr), _tail(nullptr) {}
    ~TIntrusiveList() { Assert(nullptr == _head); Assert(nullptr == _tail); }

    bool empty() const {
        Assert((nullptr == _tail) == (nullptr == _head));
        return (nullptr == _head);
    }

    T* Head() const { return _head; }
    T* Tail() const { return _tail; }

    node_type& Node(T* ptr) { return traits_type::Node(ptr); }
    const node_type& Node(const T* ptr) { return traits_type::Node(ptr); }

    T* Next(T* ptr) { return traits_type::Next(ptr); }
    T* Prev(T* ptr) { return traits_type::Prev(ptr); }

    const T* Next(const T* ptr) { return traits_type::Next(ptr); }
    const T* Prev(const T* ptr) { return traits_type::Prev(ptr); }

    T* PopHead() { return traits_type::PopHead(&_head, &_tail); }
    T* PopTail() { return traits_type::PopTail(&_head, &_tail); }

    void PushFront(T* value) { traits_type::PushFront(&_head, &_tail, value); }
    void Erase(T* value) { traits_type::Erase(&_head, &_tail, value); }

    template <typename _Less>
    void Insert(T* value, const _Less& pred) { traits_type::Insert(&_head, &_tail, value, pred); }

    void Poke(T* value) { traits_type::Poke(&_head, &_tail, value); }

    bool Contains(T* value) const { return traits_type::Contains(_head, value); }

    void Clear() { _head = _tail = nullptr; }

private:
    T* _head;
    T* _tail;
};
//----------------------------------------------------------------------------
template <typename T, TIntrusiveSingleListNode<T> T::*_Member>
class TIntrusiveSingleList {
public:
    typedef details::TIntrusiveSingleListAccessor<T, _Member> traits_type;
    typedef typename traits_type::node_type node_type;

    TIntrusiveSingleList() : _head(nullptr) {}
    ~TIntrusiveSingleList() { Assert(nullptr == _head); }

    bool empty() const { return (nullptr == _head); }

    T* Head() const { return _head; }

    node_type& Node(T* ptr) { return traits_type::Node(ptr); }
    const node_type& Node(const T* ptr) { return traits_type::Node(ptr); }

    T* Next(T* ptr) { return traits_type::Next(ptr); }
    const T* Next(const T* ptr) { return traits_type::Next(ptr); }

    T* PopHead() { return traits_type::PopHead(&_head); }
    void PushFront(T* value) { traits_type::PushFront(&_head, value); }

    void Erase(T* value, T* prev = nullptr) { traits_type::Erase(&_head, prev, value); }

    template <typename _Less>
    void Insert(T* value, const _Less& pred) { traits_type::Insert(&_head, value, pred); }

    void Poke(T* value, T* prev = nullptr) { traits_type::Poke(&_head, prev, value); }

    bool Contains(T* value) const { return traits_type::Contains(_head, value); }

    void Clear() { _head = nullptr; }

private:
    T* _head;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Container/IntrusiveList-inl.h"
