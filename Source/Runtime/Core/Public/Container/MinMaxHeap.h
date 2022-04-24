#pragma once

#include "Core_fwd.h"

#include "Container/Stack.h"
#include "HAL/PlatformMaths.h"
#include "Meta/Iterator.h"

/*
    Based on the research paper:
    Min-Max Heaps and Generalized Priority Queues
    M. D. Atkinson, J. R. Sack, N. Santoro and T. Strothotte
    Communications of the ACM, October 1986
    https://github.com/asansottera/sway/blob/master/include/sway/detail/minmaxheap.hpp
*/

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define STACKLOCAL_POD_HEAP(T, _Pred, _NAME, _COUNT) \
MALLOCA_POD(T, CONCAT(_Alloca_, _NAME), _COUNT); \
PPE::TPODStackHeapAdapter<T, Meta::TDecay<decltype(_Pred)> > _NAME( CONCAT(_Alloca_, _NAME).MakeView(), _Pred )
//----------------------------------------------------------------------------
#define STACKLOCAL_ASSUMEPOD_HEAP(T, _Pred, _NAME, _COUNT) \
MALLOCA_ASSUMEPOD(T, CONCAT(_Alloca_, _NAME), _COUNT); \
PPE::TPODStackHeapAdapter<T, Meta::TDecay<decltype(_Pred)> > _NAME( CONCAT(_Alloca_, _NAME).MakeView(), _Pred )
//----------------------------------------------------------------------------
#define STACKLOCAL_HEAP(T, _Pred, _NAME, _COUNT) \
MALLOCA_ASSUMEPOD(T, CONCAT(_Alloca_, _NAME), _COUNT); \
PPE::TStackHeapAdapter<T, Meta::TDecay<decltype(_Pred)> > _NAME( CONCAT(_Alloca_, _NAME).MakeView(), _Pred )
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _It, typename _Less >
class TMinMaxHeapHelper {
    STATIC_ASSERT(Meta::is_iterator_v<_It>);

public:
    using iterator_traits = Meta::TIteratorTraits<_It>;
    using value_type = typename iterator_traits::value_type;
    using pointer = typename iterator_traits::pointer;
    using difference_type = typename iterator_traits::difference_type;
    using size_type = std::size_t;

#if USE_PPE_ASSERT
#   define CheckInvariantsIFN(first, last, less) CheckInvariants(first, last, less)
    static CONSTEXPR void CheckInvariants(_It first, _It last, const _Less& less) {
        Assert_NoAssume(first <= last);
        const auto imin = std::min_element(first, last, less);
        Assert_NoAssume(first == last || imin == first);
        if (last - first >= 2) {
            const auto imax = std::max_element(first, last, less);
            Assert_NoAssume(imax == first + 1 || imax == first + 2 || imin == imax);
        }
    }
#else
#   define CheckInvariantsIFN(first, last, less) NOOP()
#endif

    static CONSTEXPR void make(_It first, _It last, const _Less& less) {
        if (last - first >= 2) {
            const difference_type off = (last - first) / 2 - 1;
            for (_It i = first + off; i > first; --i)
                trickle_down(first, last, i, less);

            trickle_down(first, last, first, less);
        }
        CheckInvariantsIFN(first, last, less);
    }

    static CONSTEXPR void popmin(_It first, _It last, const _Less& less) {
        Assert(first < last);
        CheckInvariantsIFN(first, last, less);

        std::iter_swap(first, last - 1);
        trickle_down(first, last - 1, first, less);

        CheckInvariantsIFN(first, last - 1, less);
    }

    static CONSTEXPR void popmax(_It first, _It last, const _Less& less) {
        Assert(first < last);
        CheckInvariantsIFN(first, last, less);

        if (last - first < 2)
            return;

        if (last - first == 2 || less(*(first + 2),  *(first + 1))) {
            std::iter_swap(first + 1, last - 1);
            trickle_down(first, last - 1, first + 1, less);
        }
        else {
            std::iter_swap(first + 2, last - 1);
            trickle_down(first, last - 1, first + 2, less);
        }

        CheckInvariantsIFN(first, last - 1, less);
    }

    static CONSTEXPR bool swapmax(_It first, _It last, pointer pElt, const _Less& less) {
        Assert(pElt);
        Assert(first < last);
        CheckInvariantsIFN(first, last, less);

        Assert(last - first >= 2);

#if 1
        const auto mx = max(first, last, less);
        if (less(*pElt, *mx)) {
            std::iter_swap(mx, last - 1);
            trickle_down(first, last - 1, mx, less);

            CheckInvariantsIFN(first, last - 1, less);

            std::iter_swap(last - 1, pElt);
            bubble_up(first, last, last - 1, less);

            CheckInvariantsIFN(first, last, less);
            return true;
        }
#else
        const auto mx = max(first, last, less);
        if (less(*pElt, *mx)) {
            std::iter_swap(mx, pElt);
            if (less(*mx, *first))
                bubble_up(first, last, mx, less);
            else
                trickle_down(first, last, mx, less);

            CheckInvariantsIFN(first, last, less);
            return true;
        }
#endif

        return false;
    }

    static CONSTEXPR void push(_It first, _It last, const _Less& less) {
        Assert(first < last);
        CheckInvariantsIFN(first, last - 1, less);

        bubble_up(first, last, last - 1, less);

        CheckInvariantsIFN(first, last, less);
    }

    static CONSTEXPR _It min(_It first, _It last, const _Less& less) {
        Assert(first < last);
        Unused(last);
        Unused(less);
        CheckInvariantsIFN(first, last, less);

        return first;
    }

    static CONSTEXPR _It max(_It first, _It last, const _Less& less) {
        Assert(first < last);
        CheckInvariantsIFN(first, last, less);

        const difference_type count = (last - first);
        if (count == 1)
            return first;

        const _It second = first + 1;
        if (count == 2)
            return second;

        // largest can be either the second or the third element
        const _It third = second + 1;
        return (less(*third, *second) ? second : third);
    }

    static CONSTEXPR void sortmin(_It first, _It last, const _Less& less) {
        CheckInvariantsIFN(first, last, less);
        for (; first < last; --last) // sort by popping all elements
            popmin(first, last, less);
    }

    static CONSTEXPR void sortmax(_It first, _It last, const _Less& less) {
        CheckInvariantsIFN(first, last, less);
        for (; first < last; --last) // sort by popping all elements
            popmax(first, last, less);
    }

#undef CheckInvariantsIFN

private:
    static CONSTEXPR _It left_child(_It first, _It last, _It i) {
        Assert(first <= i);
        const difference_type off = (i - first) * 2 + 1;
        return (off >= (last - first) ? last : first + off);
    }
    static CONSTEXPR _It right_child(_It first, _It last, _It i) {
        Assert(first <= i);
        const difference_type off = (i - first) * 2 + 2;
        return (off >= (last - first) ? last : first + off);
    }

    static CONSTEXPR size_type level(_It first, _It i) {
        return FPlatformMaths::FloorLog2(checked_cast<size_type>(i - first) + 1);
    }

    static CONSTEXPR _It parent(_It first, _It last, _It i) {
        const difference_type off = (i - first);
        return (off < 1 ? last : first + (off - 1) / 2 );
    }
    static CONSTEXPR _It grand_parent(_It first, _It last, _It i) {
        const difference_type off = (i - first);
        return (off < 3 ? last : first + (off - 3) / 4 );
    }

    static CONSTEXPR _It smallest_child_or_grandchild(_It first, _It last, _It i, const _Less& less) {
        _It smallest = last;

        const _It left = left_child(first, last, i);
        if (left < last) {
            smallest = left;

            const _It left_left = left_child(first, last, left);
            if (left_left < last && less(*left_left, *smallest))
                smallest = left_left;

            const _It left_right = right_child(first, last, left);
            if (left_right < last && less(*left_right, *smallest))
                smallest = left_right;
        }

        const _It right = right_child(first, last, i);
        if (right < last) {
            Assert(smallest < last);
            if (less(*right, *smallest))
                smallest = right;

            const _It right_left = left_child(first, last, right);
            if (right_left < last && less(*right_left, *smallest))
                smallest = right_left;

            const _It right_right = right_child(first, last, right);
            if (right_right < last && less(*right_right, *smallest))
                smallest = right_right;
        }

        return smallest;
    }

    static CONSTEXPR _It largest_child_or_grandchild(_It first, _It last, _It i, const _Less& less) {
        _It largest = last;

        const _It left = left_child(first, last, i);
        if (left < last) {
            largest = left;

            const _It left_left = left_child(first, last, left);
            if (left_left < last && less(*largest, *left_left))
                largest = left_left;

            const _It left_right = right_child(first, last, left);
            if (left_right < last && less(*largest, *left_right))
                largest = left_right;
        }

        const _It right = right_child(first, last, i);
        if (right < last) {
            Assert(largest < last);
            if (less(*largest, *right))
                largest = right;

            const _It right_left = left_child(first, last, right);
            if (right_left < last && less(*largest, *right_left))
                largest = right_left;

            const _It right_right = right_child(first, last, right);
            if (right_right < last && less(*largest, *right_right))
                largest = right_right;
        }

        return largest;
    }

    static CONSTEXPR void trickle_down_min(_It first, _It last, _It i, const _Less& less) {
        if (left_child(first, last, i) < last) {
            const _It m = smallest_child_or_grandchild(first, last, i, less);
            if (grand_parent(first, last, m) == i) {
                if (less(*m, *i)) {
                    std::iter_swap(m, i);

                    const _It parent_ = parent(first, last, m);
                    if (less(*parent_, *m))
                        std::iter_swap(m, parent_);

                    trickle_down_min(first, last, m, less);
                }
            } else {
                if (less(*m, *i))
                    std::iter_swap(m, i);
            }
        }
    }

    static CONSTEXPR void trickle_down_max(_It first, _It last, _It i, const _Less& less) {
        if (left_child(first, last, i) < last) {
            const _It m = largest_child_or_grandchild(first, last, i, less);
            if (grand_parent(first, last, m) == i) {
                if (less(*i, *m)) {
                    std::iter_swap(m, i);

                    const _It parent_ = parent(first, last, m);
                    if (less(*m, *parent_))
                        std::iter_swap(m, parent_);

                    trickle_down_max(first, last, m, less);
                }
            } else {
                if (less(*i, *m))
                    std::iter_swap(m, i);
            }
        }
    }

    static CONSTEXPR void trickle_down(_It first, _It last, _It i, const _Less& less) {
        if (level(first, i) % 2 == 0)
            trickle_down_min(first, last, i, less);
        else
            trickle_down_max(first, last, i, less);
    }

    static CONSTEXPR void bubble_up_min(_It first, _It last, _It i, const _Less& less) {
        const _It gp = grand_parent(first, last, i);
        if (gp != last && less(*i, *gp)) {
            std::iter_swap(i, gp);
            bubble_up_min(first, last, gp, less);
        }
    }

    static CONSTEXPR void bubble_up_max(_It first, _It last, _It i, const _Less& less) {
        const _It gp = grand_parent(first, last, i);
        if (gp != last && less(*gp, *i)) {
            std::iter_swap(i, gp);
            bubble_up_max(first, last, gp, less);
        }
    }

    static CONSTEXPR void bubble_up(_It first, _It last, _It i, const _Less& less) {
        const _It parent_ = parent(first, last, i);
        if (level(first, i) % 2 == 0) {
            if (parent_ != last && less(*parent_, *i)) {
                std::iter_swap(i, parent_);
                bubble_up_max(first, last, parent_, less);
            } else {
                bubble_up_min(first, last, i, less);
            }
        } else {
            if (parent_ != last && less(*i, *parent_)) {
                std::iter_swap(i, parent_);
                bubble_up_min(first, last, parent_, less);
            } else {
                bubble_up_max(first, last, i, less);
            }
        }
    }
};
//----------------------------------------------------------------------------
namespace details {
template <typename _It>
using iter_less = Meta::TLess<Meta::TDecay<
    decltype( *std::declval<_It>() )
>>;
}
//----------------------------------------------------------------------------
// Rearranges the values in the range [first,last) as a min-max heap.
template <typename _It, typename _Less = details::iter_less<_It> >
CONSTEXPR void MakeMinMaxHeap(_It first, _It last, const _Less& less = {}) {
    TMinMaxHeapHelper<_It, _Less>::make(first, last, less);
}
//----------------------------------------------------------------------------
// Returns an iterator pointing to the smallest element.
template <typename _It, typename _Less = details::iter_less<_It> >
CONSTEXPR _It Min_MinMaxHeap(_It first, _It last, const _Less& less = {}) {
    return TMinMaxHeapHelper<_It, _Less>::min(first, last, less);
}
//----------------------------------------------------------------------------
// Returns an iterator pointing to the largest element.
template <typename _It, typename _Less = details::iter_less<_It> >
CONSTEXPR _It Max_MinMaxHeap(_It first, _It last, const _Less& less = {}) {
    return TMinMaxHeapHelper<_It, _Less>::max(first, last, less);
}
//----------------------------------------------------------------------------
// Moves the smallest value in the min-max heap to the end of the sequence,
// shortening the actual min-max heap range by one position.
template <typename _It, typename _Less = details::iter_less<_It> >
CONSTEXPR void PopMin_MinMaxHeap(_It first, _It last, const _Less& less = {}) {
    TMinMaxHeapHelper<_It, _Less>::popmin(first, last, less);
}
//----------------------------------------------------------------------------
// Moves the largest value in the min-max heap to the end of the sequence,
// shortening the actual min-max heap range by one position.
template <typename _It, typename _Less = details::iter_less<_It> >
CONSTEXPR void PopMax_MinMaxHeap(_It first, _It last, const _Less& less = {}) {
    TMinMaxHeapHelper<_It, _Less>::popmax(first, last, less);
}
//----------------------------------------------------------------------------
// Given a min-max heap on the range [first,last), moves the element in the
// last-1) position to its correct position.
template <typename _It, typename _Less = details::iter_less<_It> >
CONSTEXPR void Push_MinMaxHeap(_It first, _It last, const _Less& less = {}) {
    TMinMaxHeapHelper<_It, _Less>::push(first, last, less);
}
//----------------------------------------------------------------------------
// Given a min-max heap on the range [first,last), moves the element in the
// last-1) position to its correct position.
template <typename _It, typename T, typename _Less = details::iter_less<_It> >
CONSTEXPR bool SwapMax_MinMaxHeap(_It first, _It last, T* pElt, const _Less& less = {}) {
    return TMinMaxHeapHelper<_It, _Less>::swapmax(first, last, pElt, less);
}
//----------------------------------------------------------------------------
// Convert a min max heap to a sorted sequence, from largest to smallest.
template <typename _It, typename _Less = details::iter_less<_It> >
CONSTEXPR void SortMin_MinMaxHeap(_It first, _It last, const _Less& less = {}) {
    TMinMaxHeapHelper<_It, _Less>::sortmin(first, last, less);
}
//----------------------------------------------------------------------------
template <typename _It, typename _Less = details::iter_less<_It> >
// Convert a min max heap to a sorted sequence, from smallest to largest.
CONSTEXPR void SortMax_MinMaxHeap(_It first, _It last, const _Less& less = {}) {
    TMinMaxHeapHelper<_It, _Less>::sortmax(first, last, less);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Less = Meta::TLess<T>, bool _IsPod = Meta::is_pod_v<T>>
class TStackHeapAdapter : private _Less {
public:
    typedef TStack<T, _IsPod> stack_type;

    typedef typename stack_type::reference reference;
    typedef typename stack_type::const_reference const_reference;

    typedef typename stack_type::pointer pointer;
    typedef typename stack_type::const_pointer const_pointer;

    typedef typename stack_type::size_type size_type;

    using iterator_category = typename stack_type::iterator_category;

    using iterator = typename stack_type::iterator;
    using const_iterator = typename stack_type::const_iterator;

	using reverse_iterator = typename stack_type::reverse_iterator;
	using const_reverse_iterator = typename stack_type::const_reverse_iterator;

    explicit TStackHeapAdapter(const TMemoryView<T>& storage) NOEXCEPT : _stack(storage) {}
    TStackHeapAdapter(const TMemoryView<T>& storage, _Less&& pred) NOEXCEPT : _Less(std::move(pred)), _stack(storage) {}

    size_type capacity() const { return _stack.capacity(); }
    size_type size() const { return _stack.size(); }
    bool empty() const { return _stack.empty(); }

    iterator begin() { return _stack.begin(); }
    iterator end() { return _stack.end(); }

	const_iterator begin() const { return _stack.begin(); }
    const_iterator end() const { return _stack.end(); }

    reverse_iterator rbegin() { return _stack.rbegin(); }
    reverse_iterator rend() { return _stack.rend(); }

    const_reverse_iterator rbegin() const { return _stack.rbegin(); }
    const_reverse_iterator rend() const { return _stack.rend(); }

    const_reference Min() const NOEXCEPT {
        return *Min_MinMaxHeap(_stack.begin(), _stack.end(), static_cast<const _Less&>(*this));
    }
    const_reference Max() const NOEXCEPT {
        return *Max_MinMaxHeap(_stack.begin(), _stack.end(), static_cast<const _Less&>(*this));
    }

    const_pointer PeekMin() const NOEXCEPT {
        return (empty() ? nullptr : &Min());
    }
    const_pointer PeekMax() const NOEXCEPT {
        return (empty() ? nullptr : &Max());
    }

    template <typename _Arg0, typename... _Args>
    void Push(_Arg0&& arg0, _Args&&... args) NOEXCEPT {
        _stack.Push(std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
        Push_MinMaxHeap(_stack.begin(), _stack.end(), static_cast<const _Less&>(*this));
    }

    template <typename _Arg0, typename... _Args>
    void Roll(_Arg0&& arg0, _Args&&... args) NOEXCEPT {
        if (Likely(size() == capacity())) {
            T elt{ std::forward<_Arg0>(arg0), std::forward<_Args>(args)... };
            SwapMax_MinMaxHeap(_stack.begin(), _stack.end(), &elt, static_cast<const _Less&>(*this));
        }
        else {
            Push(std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
        }
    }

    bool PopMin(pointer pMin = nullptr) NOEXCEPT {
        if (Likely(not _stack.empty())) {
            PopMin_MinMaxHeap(_stack.begin(), _stack.end(), static_cast<const _Less&>(*this));
            _stack.Pop(pMin);
            return true;
        }
        return false;
    }

    bool PopMax(pointer pMax = nullptr) NOEXCEPT {
        if (Likely(not _stack.empty())) {
            PopMax_MinMaxHeap(_stack.begin(), _stack.end(), static_cast<const _Less&>(*this));
            _stack.Pop(pMax);
            return true;
        }
        return false;
    }

    void Sort() NOEXCEPT { // beware, this won't be a valid heap after sorting
        SortMin_MinMaxHeap(_stack.begin(), _stack.end(), static_cast<const _Less&>(*this));
    }

    void Clear() NOEXCEPT {
        _stack.clear();
    }

    TMemoryView<T> MakeView() { return _stack.MakeView(); }
    TMemoryView<const T> MakeView() const { return _stack.MakeView(); }
    TMemoryView<const T> MakeConstView() const { return _stack.MakeConstView(); }

    friend void swap(TStackHeapAdapter& lhs, TStackHeapAdapter& rhs) NOEXCEPT {
        swap(static_cast<_Less&>(lhs), static_cast<_Less&>(rhs));
        lhs._stack.Swap(rhs._stack);
    }

private:
    stack_type _stack;
};
//----------------------------------------------------------------------------
template <typename T, typename _Less>
using TPODStackHeapAdapter = TStackHeapAdapter<T, _Less, true>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
