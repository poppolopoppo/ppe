#pragma once

#include "Core_fwd.h"

#include "Memory/PtrRef.h"

// AVL self-balanced binary tree implementation
// https://en.wikipedia.org/wiki/AVL_tree

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct TAVLTreeTraits {
    using pointer = T*;

    NODISCARD static i32 Height(pointer node) { return node->Height; }
    static void SetHeight(pointer node, i32 height) { node->Height = height; }

    NODISCARD static pointer Left(pointer node) { return node->Left; }
    static void SetLeft(pointer node, pointer child) { node->Left = child; }

    NODISCARD static pointer Right(pointer node) { return node->Right; }
    static void SetRight(pointer node, pointer child) { node->Right = child; }
};
//----------------------------------------------------------------------------
template <typename T, typename _Less = Meta::TLess<T>, typename _Traits = TAVLTreeTraits<T>>
struct TAVLTree : _Traits {
    using less_type = _Less;
    using traits_type = _Traits;
    using typename traits_type::pointer;
    using traits_type::SetHeight;
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
        ONLY_IF_ASSERT(const size_t n = CountNodes_Slow(Root));
        Root = InsertAt(node, Root);
        ONLY_IF_ASSERT(const size_t n2 = CountNodes_Slow(Root));
        Assert_NoAssume(n + 1 == n2);
    }

    void Erase(pointer const node) NOEXCEPT {
        ONLY_IF_ASSERT(const size_t n = CountNodes_Slow(Root));
        Root = EraseAt(node, Root);
        ONLY_IF_ASSERT(const size_t n2 = CountNodes_Slow(Root));
        Assert_NoAssume(n == n2 + 1);
        ONLY_IF_ASSERT(SetHeight(node, -1));
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

            // Node with only one child or no child
            if (Left(root) == nullptr)
                return Right(root);
            if (Right(root) == nullptr)
                return Left(root);

            // Node with two children, get the in-order successor (smallest in right subtree)
            pointer const inOrderSucc = MinValueNode(Right(root));

            SetRight(root, EraseAt(inOrderSucc, Right(root)));
            SetLeft(inOrderSucc, Left(root));
            SetRight(inOrderSucc, Right(root));
            return inOrderSucc;
        }

        UpdateHeight(root);
        const i32 balance = BalanceFactor(root);

        // left heavy
        if (balance > 1) {
            if (BalanceFactor(Left(root)) >= 0)
                return RotateRight(root);

            SetLeft(root, RotateLeft(Left(root)));
            return RotateRight(root);
        }

        // right heavy
        if (balance < -1) {
            if (BalanceFactor(Right(root)) <= 0)
                return RotateLeft(root);

            SetRight(root, RotateRight(Right(root)));
            return RotateLeft(root);
        }

        return root;
    }
    NODISCARD static pointer InsertAt(pointer const node, pointer root) {
        Assert_NoAssume(node);

        if (root == nullptr) {
            SetHeight(node, 0);
            SetLeft(node, nullptr);
            SetRight(node, nullptr);
            return node;
        }

        if (Less(node, root))
            SetLeft(root, InsertAt(node, Left(root)));
        else if (Less(root, node))
            SetRight(root, InsertAt(node, Right(root)));

        UpdateHeight(root);
        const i32 balance = BalanceFactor(root);

        // left heavy
        if (balance > 1) {
            if (Less(node, Left(root)))
                return RotateRight(root);

            SetLeft(root, RotateLeft(Left(root)));
            return RotateRight(root);
        }

        // right heavy
        if (balance < -1) {
            if (Less(Right(root), node))
                return RotateLeft(root);

            SetRight(root, RotateRight(Right(root)));
            return RotateLeft(root);
        }

        return root;
    }
    NODISCARD static pointer MinValueNode(pointer node) {
        pointer current = node;
        while (Left(current) != nullptr)
            current = Left(current);
        return current;
    }
    NODISCARD static bool Less(Meta::TAddConst<T>* lhs, Meta::TAddConst<T>* rhs) {
        Assert(lhs && rhs);
        return less_type{}(*lhs, *rhs);
    }
    NODISCARD static i32 Height(pointer node) {
        if (nullptr == node)
            return 0;

        return traits_type::Height(node);
    }
    static void UpdateHeight(pointer node) {
        if (node) SetHeight(node, 1_i32 + Max(
            Height(Left(node)),
            Height(Right(node))));
    }
    NODISCARD static i32 BalanceFactor(pointer node) {
        if (nullptr == node)
            return 0;

        pointer const left = Left(node);
        pointer const right = Right(node);
        return (Height(left) - Height(right));
    }
    NODISCARD static pointer RotateRight(pointer y) {
        Assert(y);
        pointer const x = Left(y);
        if (x == nullptr)
            return y;
        pointer const t2 = Right(x);

        SetRight(x, y);
        SetLeft(y, t2);

        UpdateHeight(y);
        UpdateHeight(x);
        return x;
    }
    NODISCARD static pointer RotateLeft(pointer x) {
        Assert(x);
        pointer const y = Right(x);
        if (y == nullptr)
            return x;
        pointer const t2 = Left(y);

        SetLeft(y, x);
        SetRight(x, t2);

        UpdateHeight(x);
        UpdateHeight(y);
        return y;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
