#pragma once

#include "Core_fwd.h"

#include "Memory/PtrRef.h"

// WAVL self-balanced binary tree implementation,
// less constrained balancing heuristic than AVL -> should do less rotations when inserting/deleting
// https://en.wikipedia.org/wiki/WAVL_tree

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct TWAVLTreeTraits {
    using pointer = T*;

    NODISCARD static i32 Rank(pointer node) { return node->Rank; }
    static void SetRank(pointer node, i32 rank) { node->Rank = rank; }

    NODISCARD static pointer Left(pointer node) { return node->Left; }
    static void SetLeft(pointer node, pointer child) { node->Left = child; }

    NODISCARD static pointer Right(pointer node) { return node->Right; }
    static void SetRight(pointer node, pointer child) { node->Right = child; }
};
//----------------------------------------------------------------------------
template <typename T, typename _Less = Meta::TLess<T>, typename _Traits = TWAVLTreeTraits<T>>
struct TWAVLTree : _Traits {
    using less_type = _Less;
    using traits_type = _Traits;
    using typename traits_type::pointer;
    using traits_type::SetRank;
    using traits_type::Left;
    using traits_type::SetLeft;
    using traits_type::Right;
    using traits_type::SetRight;

    pointer Root{ nullptr };

    NODISCARD pointer LowerBound(const T& value) const NOEXCEPT {
        pointer result = nullptr;

        for (pointer root = Root; root; ) {
            if (Less(root, &value)) {
                result = root;
                root = Right(root);
                continue;
            }

            if (Less(&value, root))
                root = Left(root);
            else
                return root; // exact match found
        }

        return result;
    }

    NODISCARD pointer UpperBound(const T& value) const NOEXCEPT {
        pointer result = nullptr;

        for (pointer root = Root; root; ) {
            if (Less(&value, root)) {
                result = root;
                root = Left(root);
                continue;
            }

            if (Less(root, &value))
                root = Right(root);
            else
                return root; // exact match found
        }

        return result;
    }

    NODISCARD bool Contains(const T* node) const NOEXCEPT {
        Assert(node);
        for (pointer root = Root; root; ) {
            if (Less(node, root))
                root = Left(root);
            else if (Less(root, node))
                root = Right(root);
            else if (root == node)
                return true; // exact match found
            else // same value, wrong node: continue in left sub-tree
                root = Left(root);
        }

        return false;
    }

    void Insert(pointer const node) NOEXCEPT {
        Assert(node);
        ONLY_IF_ASSERT(const size_t n = CountNodes_Slow(Root));
        Root = InsertAt(node, Root);
        ONLY_IF_ASSERT(const size_t n2 = CountNodes_Slow(Root));
        Assert_NoAssume(n + 1 == n2);
    }

    void Erase(pointer const node) NOEXCEPT {
        Assert(node);
        ONLY_IF_ASSERT(const size_t n = CountNodes_Slow(Root));
        Root = EraseAt(node, Root);
        ONLY_IF_ASSERT(const size_t n2 = CountNodes_Slow(Root));
        Assert_NoAssume(n == n2 + 1);
        ONLY_IF_ASSERT(SetRank(node, -1));
        ONLY_IF_ASSERT(SetLeft(node, nullptr));
        ONLY_IF_ASSERT(SetRight(node, nullptr));
    }

    NODISCARD static size_t CountNodes_Slow(pointer root) {
        if (root == nullptr)
            return 0;
        return (1 + CountNodes_Slow(Left(root)) + CountNodes_Slow(Right(root)));
    }
    NODISCARD static pointer EraseAt(pointer const node, pointer root) {
        if (root == nullptr)
            return root;

        if (Less(node, root))
            SetLeft(root, EraseAt(node, Left(root)));
        else if (Less(root, node))
            SetRight(root, EraseAt(node, Right(root)));
        else {
            Assert(node == root);

            if (Left(root) == nullptr && Right(root) == nullptr)
                return nullptr;
            if (Left(root) == nullptr || Right(root) == nullptr)
                return (Left(root) ? Left(root) : Right(root));

            pointer const inOrderSucc = MinValueNode(Right(root));

            SetRight(root, EraseAt(inOrderSucc, Right(root)));
            SetLeft(inOrderSucc, Left(root));
            SetRight(inOrderSucc, Right(root));

            root = inOrderSucc;
        }

        return Balance(root);
    }
    NODISCARD static pointer InsertAt(pointer const node, pointer root) {
        if (root == nullptr) {
            SetRank(node, 0);
            SetLeft(node, nullptr);
            SetRight(node, nullptr);
            return node;
        }

        if (Less(node, root))
            SetLeft(root, InsertAt(node, Left(root)));
        else if (Less(root, node))
            SetRight(root, InsertAt(node, Right(root)));
        else
            AssertNotReached();

        return Balance(root);
    }
    NODISCARD static pointer MinValueNode(pointer node) {
        pointer current = node;
        while (Left(current) != nullptr)
            current = Left(current);
        return current;
    }
    NODISCARD static bool Less(Meta::TAddConst<T>* lhs, Meta::TAddConst<T>* rhs) {
        return less_type{}(*lhs, *rhs);
    }
    NODISCARD static pointer LeftIFP(pointer node) {
        return (node ? Left(node) : nullptr);
    }
    NODISCARD static pointer RightIFP(pointer node) {
        return (node ? Right(node) : nullptr);
    }
    NODISCARD static pointer Balance(pointer node) {
        UpdateRank(node);

        if (Rank(Right(node)) > Rank(Left(node))) {
            if (Rank(RightIFP(Right(node))) < Rank(LeftIFP(Right(node))))
                SetRight(node, RotateRight(Right(node)));

            node = RotateLeft(node);
        }
        else {
            if (Rank(LeftIFP(Left(node))) < Rank(RightIFP(Left(node))))
                SetLeft(node, RotateLeft(Left(node)));

            node = RotateRight(node);
        }

        return node;
    }
    NODISCARD static i32 Rank(pointer node) {
        return (node != nullptr ? traits_type::Rank(node) : -1);
    }
    static void UpdateRank(pointer node) {
        SetRank(node, 1_i32 + Max(Rank(Left(node)), Rank(Right(node))));
    }
    NODISCARD static pointer RotateRight(pointer y) {
        Assert(y);
        pointer const x = Left(y);
        if (x == nullptr)
            return y;

        SetLeft(y, Right(x));
        SetRight(x, y);

        UpdateRank(y);
        UpdateRank(x);
        return x;
    }
    NODISCARD static pointer RotateLeft(pointer x) {
        Assert(x);
        pointer const y = Right(x);
        if (y == nullptr)
            return x;

        SetRight(x, Left(y));
        SetLeft(y, x);

        UpdateRank(x);
        UpdateRank(y);
        return y;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
