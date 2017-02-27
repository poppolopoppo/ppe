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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct TIntrusiveListNode {
    T* Next = nullptr;
    T* Prev = nullptr;
};
//----------------------------------------------------------------------------
namespace details {
template <typename T, TIntrusiveListNode<T> T::*_Member>
struct TIntrusiveListAccessor;
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

private:
    T* _head;
    T* _tail;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T> struct TIntrusiveListTraits {};
template <typename T> struct TIntrusiveListTraits< TIntrusiveListNode<T> T::* > {
    template <TIntrusiveListNode<T> T::*_Member>
    struct list { typedef TIntrusiveList<T, _Member> type; };
    template <TIntrusiveListNode<T> T::*_Member>
    struct accessor { typedef TIntrusiveListAccessor<T, _Member> type; };
};
//----------------------------------------------------------------------------
template <typename T, TIntrusiveListNode<T> T::*_Member>
struct TIntrusiveListAccessor {
    typedef TIntrusiveListNode< T > node_type;

    static FORCE_INLINE node_type& Node(T* ptr) { return ptr->*_Member; }
    static FORCE_INLINE const node_type& Node(const T* ptr) { return ptr->*_Member; }

    static T* Next(T* ptr) { Assert(ptr); return Node(ptr).Next; }
    static T* Prev(T* ptr) { Assert(ptr); return Node(ptr).Prev; }

    static const T* Next(const T* ptr) { Assert(ptr); return Node(ptr).Next; }
    static const T* Prev(const T* ptr) { Assert(ptr); return Node(ptr).Prev; }

    static T* PopHead(T** pHead, T** pTailIFP);
    static T* PopTail(T** pHead, T** pTail);

    static void PushFront(T** pHead, T** pTailIFP, T* value);
    static void Erase(T** pHead, T** pTailIFP, T* value);

    template <typename _Less>
    static void Insert(T** pHead, T** pTailIFP, T* value, const _Less& pred);

    static void Poke(T** pHead, T** pTailIFP, T* value) {
        Assert(pHead);
        if (*pHead != value) {
            Erase(pHead, pTailIFP, value);
            PushFront(pHead, pTailIFP, value);
        }
    }
};
//----------------------------------------------------------------------------
template <typename T, TIntrusiveListNode<T> T::*_Member>
T* TIntrusiveListAccessor<T, _Member>::PopHead(T** pHead, T** pTailIFP) {
    Assert(pHead);

    if (nullptr == *pHead) {
        Assert(nullptr == pTailIFP || nullptr == *pTailIFP);
        return nullptr;
    }
    else {
        Assert(nullptr == pTailIFP || nullptr != *pTailIFP);

        T* const result = *pHead;
        Erase(pHead, pTailIFP, result);

        return result;
    }
}
//----------------------------------------------------------------------------
template <typename T, TIntrusiveListNode<T> T::*_Member>
T* TIntrusiveListAccessor<T, _Member>::PopTail(T** pHead, T** pTail) {
    Assert(pHead);
    Assert(pTail);

    if (nullptr == *pTail) {
        return nullptr;
    }
    else {
        Assert(nullptr != *pHead);

        T* const result = *pTail;
        Erase(pHead, pTail, result);

        return result;
    }
}
//----------------------------------------------------------------------------
template <typename T, TIntrusiveListNode<T> T::*_Member>
void TIntrusiveListAccessor<T, _Member>::PushFront(T** pHead, T** pTailIFP, T* value) {
    Assert(pHead);
    Assert(value);

    node_type& node = Node(value);
    node.Prev = nullptr;
    node.Next = *pHead;

    if (nullptr != *pHead) {
        node_type& head = Node(*pHead);
        Assert(nullptr == head.Prev);
        head.Prev = value;
    }
    else if (pTailIFP) {
        Assert(nullptr == *pTailIFP);
        *pTailIFP = value;
    }

    *pHead = value;
}
//----------------------------------------------------------------------------
template <typename T, TIntrusiveListNode<T> T::*_Member>
void TIntrusiveListAccessor<T, _Member>::Erase(T** pHead, T** pTailIFP, T* value) {
    Assert(pHead);
    Assert(value);
    Assert(*pHead);
    Assert(nullptr == pTailIFP || *pTailIFP);

    node_type& node = Node(value);
    Assert( node.Next || node.Prev ||
            *pHead == value ||
            (pTailIFP && *pTailIFP == value) );

    if (node.Prev) {
        node_type& prev = Node(node.Prev);
        Assert(prev.Next == value);
        prev.Next = node.Next;
    }

    if (node.Next) {
        node_type& next = Node(node.Next);
        Assert(next.Prev == value);
        next.Prev = node.Prev;
    }

    if (*pHead == value) {
        Assert(nullptr == node.Prev);
        *pHead = node.Next;
        Assert(nullptr == *pHead || nullptr == Node(*pHead).Prev);
    }

    if (pTailIFP && *pTailIFP == value) {
        Assert(nullptr == node.Next);
        *pTailIFP = node.Prev;
        Assert(nullptr == *pTailIFP || nullptr == Node(*pTailIFP).Next);
    }

    node.Prev = node.Next = nullptr;
}
//----------------------------------------------------------------------------
template <typename T, TIntrusiveListNode<T> T::*_Member>
template <typename _Less>
void TIntrusiveListAccessor<T, _Member>::Insert(T** pHead, T** pTailIFP, T* value, const _Less& pred) {
    Assert(pHead);
    Assert(value);

    node_type& node = Node(value);
    node.Prev = nullptr;
    node.Next = nullptr;

    T* p = *pHead;
    while (p) {
        node_type& it = Node(p);

        if (pred(*value, *p)) {
            node.Prev = it.Prev;
            node.Next = p;
            it.Prev = value;

            if (node.Prev) {
                node_type& prev = Node(node.Prev);
                Assert(p == prev.Next);
                prev.Next = value;
            }
            else {
                Assert(p == *pHead);
                *pHead = value;
            }

            return;
        }
        else if (nullptr == it.Next) {
            Assert(nullptr == pTailIFP || *pTailIFP == p);
            node.Prev = p;
            it.Next = value;

            if (pTailIFP) {
                Assert(p == *pTailIFP);
                *pTailIFP = value;
            }

            return;
        }

        p = it.Next;
    }

    Assert(nullptr == *pHead);
    *pHead = value;

    if (pTailIFP) {
        Assert(nullptr == *pTailIFP);
        *pTailIFP = value;
    }
}
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
