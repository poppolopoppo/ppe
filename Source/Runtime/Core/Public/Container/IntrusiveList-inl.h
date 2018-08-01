#pragma once

#include "Container/IntrusiveList.h"

namespace PPE {
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

    static FORCE_INLINE T*& Next(T* ptr) { Assert(ptr); return Node(ptr).Next; }
    static FORCE_INLINE T*& Prev(T* ptr) { Assert(ptr); return Node(ptr).Prev; }

    static FORCE_INLINE const T* Next(const T* ptr) { Assert(ptr); return Node(ptr).Next; }
    static FORCE_INLINE const T* Prev(const T* ptr) { Assert(ptr); return Node(ptr).Prev; }

    static T* PopHead(T** pHead, T** pTailIFP);
    static T* PopTail(T** pHead, T** pTail);

    static void PushFront(T** pHead, T** pTailIFP, T* value);
    static void PushTail(T** pHead, T** pTailIFP, T* value);

    static void Erase(T** pHead, T** pTailIFP, T* value);

    template <typename _Less>
    static void Insert(T** pHead, T** pTailIFP, T* value, const _Less& pred);

    static bool Contains(T* head, T* value);

    static void PokeFront(T** pHead, T** pTailIFP, T* value) {
        Assert(pHead);
        if (*pHead != value) {
            Erase(pHead, pTailIFP, value);
            PushFront(pHead, pTailIFP, value);
        }
    }

    static void PokeTail(T** pHead, T** pTail, T* value) {
        Assert(pHead);
        Assert(pTail);
        if (*pTail != value) {
            Erase(pHead, pTail, value);
            PushTail(pHead, pTail, value);
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
void TIntrusiveListAccessor<T, _Member>::PushTail(T** pHead, T** pTail, T* value) {
    Assert(pHead);
    Assert(pTail);
    Assert(value);

    node_type& node = Node(value);
    node.Prev = *pTail;
    node.Next = nullptr;

    if (nullptr != *pTail) {
        node_type& tail = Node(*pTail);
        Assert(nullptr == tail.Next);
        tail.Next = value;
    }
    else {
        Assert(nullptr == *pHead);
        *pHead = value;
    }

    *pTail = value;
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

    ONLY_IF_ASSERT(node.Prev = node.Next = nullptr);
}
//----------------------------------------------------------------------------
template <typename T, TIntrusiveListNode<T> T::*_Member>
template <typename _Less>
void TIntrusiveListAccessor<T, _Member>::Insert(T** pHead, T** pTailIFP, T* value, const _Less& pred) {
    Assert(pHead);
    Assert(value);

    node_type& node = Node(value);

    T* prev = nullptr;
    for (T* p = *pHead; p; prev = p, p = Next(p)) {
        if (pred(*value, *p))
            break;
    }

    if (prev) {
        T*& next = Node(prev).Next;
        node.Next = next;
        next = value;
    }
    else {
        node.Next = *pHead;
        *pHead = value;
    }

    if (node.Next) {
        Node(node.Next).Prev = value;
    }
    else if (pTailIFP) {
        Assert(nullptr == *pTailIFP || node.Prev == *pTailIFP);
        *pTailIFP = value;
    }
}
//----------------------------------------------------------------------------
template <typename T, TIntrusiveListNode<T> T::*_Member>
bool TIntrusiveListAccessor<T, _Member>::Contains(T* head, T* value) {
    for (; head; head = Next(head)) {
        if (head == value)
            return true;
    }
    return false;
}
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T> struct TIntrusiveSingleListTraits {};
template <typename T> struct TIntrusiveSingleListTraits < TIntrusiveSingleListNode<T> T::* > {
    template <TIntrusiveSingleListNode<T> T::*_Member>
    struct list { typedef TIntrusiveSingleList<T, _Member> type; };
    template <TIntrusiveSingleListNode<T> T::*_Member>
    struct accessor { typedef TIntrusiveSingleListAccessor<T, _Member> type; };
};
//----------------------------------------------------------------------------
template <typename T, TIntrusiveSingleListNode<T> T::*_Member>
struct TIntrusiveSingleListAccessor {
    typedef TIntrusiveSingleListNode< T > node_type;

    static FORCE_INLINE node_type& Node(T* ptr) { return ptr->*_Member; }
    static FORCE_INLINE const node_type& Node(const T* ptr) { return ptr->*_Member; }

    static FORCE_INLINE T*& Next(T* ptr) { Assert(ptr); return Node(ptr).Next; }
    static FORCE_INLINE const T* Next(const T* ptr) { Assert(ptr); return Node(ptr).Next; }

    static T* PopHead(T** pHead);
    static void PushFront(T** pHead, T* value);

    static void Erase(T** pHead, T* prev, T* value);

    template <typename _Less>
    static void Insert(T** pHead, T* value, const _Less& pred);

    static bool Contains(T* head, T* value);

    static void PokeFront(T** pHead, T* prev, T* value) {
        Assert(pHead);
        if (*pHead != value) {
            Erase(pHead, prev, value);
            PushFront(pHead, value);
        }
        else {
            Assert(nullptr == prev);
        }
    }
};
//----------------------------------------------------------------------------
template <typename T, TIntrusiveSingleListNode<T> T::*_Member>
T* TIntrusiveSingleListAccessor<T, _Member>::PopHead(T** pHead) {
    Assert(pHead);

    if (nullptr == *pHead)
        return nullptr;

    T* const result = *pHead;
    node_type& node = Node(*pHead);
    *pHead = node.Next;
    ONLY_IF_ASSERT(node.Next = nullptr);

    return result;
}
//----------------------------------------------------------------------------
template <typename T, TIntrusiveSingleListNode<T> T::*_Member>
void TIntrusiveSingleListAccessor<T, _Member>::PushFront(T** pHead, T* value) {
    Assert(pHead);
    Assert(value);

    node_type& node = Node(value);
    node.Next = *pHead;
    *pHead = value;
}
//----------------------------------------------------------------------------
template <typename T, TIntrusiveSingleListNode<T> T::*_Member>
void TIntrusiveSingleListAccessor<T, _Member>::Erase(T** pHead, T* prev, T* value) {
    Assert(pHead);
    Assert(*pHead);
    Assert(value);

    node_type& node = Node(value);

    if (prev) {
        Assert(value != *pHead);
        node_type& p = Node(prev);
        Assert(p.Next == value);
        p.Next = node.Next;
    }
    else {
        Assert(value == *pHead);
        *pHead = node.Next;
    }

    ONLY_IF_ASSERT(node.Next = nullptr);
}
//----------------------------------------------------------------------------
template <typename T, TIntrusiveSingleListNode<T> T::*_Member>
template <typename _Less>
void TIntrusiveSingleListAccessor<T, _Member>::Insert(T** pHead, T* value, const _Less& pred) {
    Assert(pHead);
    Assert(value);

    node_type& node = Node(value);

    T* prev = nullptr;
    for (T* p = *pHead; p; prev = p, p = Next(p)) {
        if (pred(*value, *p))
            break;
    }

    if (prev) {
        T*& next = Node(prev).Next;
        node.Next = next;
        next = value;
    }
    else {
        node.Next = *pHead;
        *pHead = value;
    }
}
//----------------------------------------------------------------------------
template <typename T, TIntrusiveSingleListNode<T> T::*_Member>
bool TIntrusiveSingleListAccessor<T, _Member>::Contains(T* head, T* value) {
    for (; head; head = Next(head)) {
        if (head == value)
            return true;
    }
    return false;
}
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
