#pragma once

#include "Core/Core.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct IntrusiveListNode {
    T* Next = nullptr;
    T* Prev = nullptr;
};
//----------------------------------------------------------------------------
#define INTRUSIVELIST(_Member) \
    ::Core::details::IntrusiveListTraits< decltype(_Member) >::type< _Member >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T, IntrusiveListNode<T> T::*_Member>
struct IntrusiveListAccessor;
//----------------------------------------------------------------------------
template <typename T> struct IntrusiveListTraits {};
template <typename T> struct IntrusiveListTraits< IntrusiveListNode<T> T::* > {
    template <IntrusiveListNode<T> T::*_Member>
    using type = IntrusiveListAccessor<T, _Member>;
};
//----------------------------------------------------------------------------
template <typename T, IntrusiveListNode<T> T::*_Member>
struct IntrusiveListAccessor {
    typedef IntrusiveListNode< T > node_type;

    static void Queue(T** pHead, T** pTailIFP, T* value);
    static void Deque(T** pHead, T** pTailIFP, T* value);

    static void Poke(T** pHead, T** pTailIFP, T* value) {
        Deque(pHead, pTailIFP, value);
        Queue(pHead, pTailIFP, value);
    }
};
//----------------------------------------------------------------------------
template <typename T, IntrusiveListNode<T> T::*_Member>
void IntrusiveListAccessor<T, _Member>::Queue(T** pHead, T** pTailIFP, T* value) {
    Assert(pHead);
    Assert(value);

    node_type& node = value->*_Member;
    node.Prev = nullptr;
    node.Next = *pHead;

    if (nullptr != *pHead) {
        node_type& head = (*pHead)->*_Member;
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
template <typename T, IntrusiveListNode<T> T::*_Member>
void IntrusiveListAccessor<T, _Member>::Deque(T** pHead, T** pTailIFP, T* value) {
    Assert(pHead);
    Assert(value);
    Assert(*pHead);
    Assert(nullptr == pTailIFP || *pTailIFP);

    node_type& node = value->*_Member;
    Assert( node.Next || node.Prev ||
            *pHead == value ||
            (pTailIFP && *pTailIFP == value) );

    if (node.Prev) {
        Assert((node.Prev->*_Member).Next == value);
        (node.Prev->*_Member).Next = node.Next;
    }

    if (node.Next) {
        Assert((node.Next->*_Member).Prev == value);
        (node.Next->*_Member).Prev = node.Prev;
    }

    if (*pHead == value) {
        Assert(nullptr == node.Prev);
        *pHead = node.Next;
        Assert(nullptr == *pHead || nullptr == ((*pHead)->*_Member).Prev);
    }

    if (pTailIFP && *pTailIFP == value) {
        Assert(nullptr == node.Next);
        *pTailIFP = node.Prev;
        Assert(nullptr == *pTailIFP || nullptr == ((*pTailIFP)->*_Member).Next);
    }

    node.Prev = node.Next = nullptr;
}
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
