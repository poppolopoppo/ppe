#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBase.h"
#include "Allocator/PageAllocator.h"
#include "HAL/PlatformMaths.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"
#include "Thread/ReadWriteLock.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Radix-trie optimized for storing pointers with some limitations:
//  - Key must have 8 lower bits set to 0
//  - Value must have first lower bit set to 0
//  - Value can't be 0
//  - Memory allocated will only released when cleared/destroyed
// You shouldn't be using as a regular container, as it's more intended for low level
//
template <typename _Allocator = FPageAllocator>
class TCompressedRadixTrie : _Allocator {
public:
    using _Allocator::PageSize; // we need a *Page* allocator, not any allocator
    using allocator_traits = TAllocatorTraits<_Allocator>;

    struct ALIGN(16) FNode {
        uintptr_t Keys[2];
        FNode* Children[2];
    };

    explicit TCompressedRadixTrie(ARG0_IF_MEMORYDOMAINS(FMemoryTracking& trackingData))
#if USE_PPE_MEMORYDOMAINS
    :   _trackingDataRef(&trackingData)
#endif
    {}

    explicit TCompressedRadixTrie(_Allocator&& alloc ARGS_IF_MEMORYDOMAINS(FMemoryTracking& trackingData))
    :   _Allocator(std::move(alloc))
#if USE_PPE_MEMORYDOMAINS
    ,   _trackingDataRef(&trackingData)
#endif
    {}

    explicit TCompressedRadixTrie(const _Allocator& alloc ARGS_IF_MEMORYDOMAINS(FMemoryTracking& trackingData))
    :   _Allocator(alloc)
#if USE_PPE_MEMORYDOMAINS
    ,   _trackingDataRef(&trackingData)
#endif
    {}

    TCompressedRadixTrie(const TCompressedRadixTrie&) = delete;
    TCompressedRadixTrie& operator =(const TCompressedRadixTrie&) = delete;

    ~TCompressedRadixTrie() {
        Clear_ReleaseMemory();
    }

    bool empty() const { return (NullSentinel_ == uintptr_t(_root)); }

    void Insert(const void* pkey, const void* pvalue) { Insert(uintptr_t(pkey), uintptr_t(pvalue)); }
    void Insert(uintptr_t key, uintptr_t value) {
        Assert(!(key & 0xFF));
        Assert(!(value & 1));
        Assert(value);

        FNode* newNode;
        if (_freeList) {
            _freeList = *(FNode**)(newNode = _freeList);
        }
        else if (_newPageAllocated) {
            newNode = _newPageAllocated;
            STATIC_ASSERT(Meta::IsPow2(PageSize));
            if (Meta::IsAlignedPow2(PageSize, ++_newPageAllocated + 1/* last block used for bookkeeping */))
                _newPageAllocated = nullptr; // will request for a new page on next alloc
        }
        else {
            Assert(Meta::IsPow2(PageSize));

            FNode* const newPage = (FNode*)allocator_traits::Allocate(*this, PageSize).Data;
            AssertRelease(newPage);
            ONLY_IF_MEMORYDOMAINS(_trackingDataRef->AllocateSystem(PageSize));

            ((FNode**)((char*)newPage + PageSize))[-1] = _allPages;
            _allPages = newPage;

            // eat first block for user request, and saves the rest for later insertions
            newNode = newPage;
            _newPageAllocated = (newPage + 1);
        }

        InsertAt_(key, value, newNode);
    }

    void Clear_ReleaseMemory() {
        ONLY_IF_MEMORYDOMAINS(_trackingDataRef->ReleaseAllUser());

        while (_allPages) {
            FNode* const nextPage = ((FNode**)((char*)_allPages + PageSize))[-1];
            ONLY_IF_MEMORYDOMAINS(_trackingDataRef->DeallocateSystem(PageSize));
            allocator_traits::Deallocate(*this, { _allPages, PageSize });
            _allPages = nextPage;
        }

        Assert(nullptr == _allPages);
        _root = (FNode*)NullSentinel_;
        _freeList = nullptr;
        _newPageAllocated = nullptr;
    }

    void* Lookup(const void* pkey) const NOEXCEPT { return reinterpret_cast<void*>(Lookup(uintptr_t(pkey))); }
    uintptr_t Lookup(uintptr_t key) const NOEXCEPT {
        Assert(!(key & 0xFF));
        const FNode* node = _root;
        const uintptr_t* pkey = nullptr;

        while (!(uintptr_t(node) & 1)) {
            int branch = ((key >> (node->Keys[0] & 0xFF)) & 1);
            pkey = &node->Keys[branch];
            node = node->Children[branch];
        }

        Assert(pkey && (*pkey & ~uintptr_t(0xFF)) == key);
        return (uintptr_t(node) & ~uintptr_t(1)); // can't fail, key is *ALWAYS* here
    }

    bool Find(void** pvalue, const void* pkey) const NOEXCEPT { return Find((uintptr_t*)pvalue, uintptr_t(pkey)); }
    bool Find(uintptr_t* pvalue, uintptr_t key) const NOEXCEPT {
        Assert(pvalue);
        Assert(!(key & 0xFF));
        const FNode* node = _root;
        const uintptr_t* pkey = nullptr;

        while (!(uintptr_t(node) & 1)) {
            int branch = ((key >> (node->Keys[0] & 0xFF)) & 1);
            pkey = &node->Keys[branch];
            node = node->Children[branch];
        }

        if (Likely(pkey && (*pkey & ~uintptr_t(0xFF)) == key)) {
            *pvalue = (uintptr_t(node) & ~uintptr_t(1));
            Assert_NoAssume(uintptr_t(node) != NullSentinel_ || *pvalue == 0);
            return (uintptr_t(node) != NullSentinel_);
        }

        return false;
    }

    void* Erase(const void* pkey) NOEXCEPT { return reinterpret_cast<void*>(Erase(uintptr_t(pkey))); }
    uintptr_t Erase(uintptr_t key) NOEXCEPT {
        Assert(!(key & 0xFF));
        Assert(uintptr_t(_root) != NullSentinel_); // can't erase from empty trie !
        FNode** parent = &_root;
        uintptr_t* pkey = nullptr;

        for (;;) {
            FNode* n = *parent;

            const u32 branch = u32((key >> (n->Keys[0] & 0xFF)) & 1);
            FNode* const child = n->Children[branch]; // current child node
            if (uintptr_t(child) & 1) { // leaf
                Assert((n->Keys[branch] & ~uintptr_t(0xFF)) == key);
                return EraseAt_(parent, pkey, branch);
            }

            pkey = &n->Keys[branch];
            parent = &n->Children[branch];
        }
    }

    template <typename _Predicate>
    void DeleteIf(_Predicate&& pred) NOEXCEPT {
        if (NullSentinel_ != uintptr_t(_root))
            DeleteIf_Recursive_(pred, &_root, nullptr);
    }

    template <typename _Predicate>
    bool Where(_Predicate&& pred) const NOEXCEPT {
        return Foreach(std::forward<_Predicate>(pred));
    }

    template <typename _Functor>
    auto Foreach(_Functor&& functor) const NOEXCEPT {
        using result_type = decltype(functor(uintptr_t{}, uintptr_t{}));
        if (Likely(NullSentinel_ != uintptr_t(_root))) {
            CONSTEXPR u32 stack_capacity = 128;
            const FNode* stack[stack_capacity];
            stack[0] = _root;
            u32 stacked = 1;

            do {
                const FNode* node = stack[--stacked];

                forrange(branch, 0, u32(2)) {
                    if (uintptr_t(node->Children[branch]) & 1) {
                        if (uintptr_t(node->Children[branch]) != NullSentinel_) {
                            const uintptr_t key = (node->Keys[branch] & ~uintptr_t(0xFF));
                            const uintptr_t value = (uintptr_t(node->Children[branch]) & ~uintptr_t(1));

                            IF_CONSTEXPR(std::is_same_v<void, result_type>)
                                functor(key, value);
                        else if (const result_type result = functor(key, value))
                            return result;
                        }
                    }
                    else {
                        Assert(stacked < stack_capacity);
                        stack[stacked++] = node->Children[branch];
                    }
                }

            } while (stacked);
        }

        IF_CONSTEXPR(not std::is_same_v<void, result_type>)
            return Meta::DefaultValue<result_type>();
    }

private:
    STATIC_CONST_INTEGRAL(uintptr_t, NullSentinel_, 1);

    FNode* _root{ (FNode*)NullSentinel_ };
    FNode* _freeList{ nullptr };
    FNode* _newPageAllocated{ nullptr };
    FNode* _allPages{ nullptr };

#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking* const _trackingDataRef{ nullptr };
#endif

    void InsertAt_(const uintptr_t key, const uintptr_t value, FNode* const newNode) {
        uintptr_t xcmp = 0;
        uintptr_t nkey = (uintptr_t(_root) == NullSentinel_ ? 0 : _root->Keys[0]);

        ONLY_IF_MEMORYDOMAINS(_trackingDataRef->AllocateUser(sizeof(FNode)));

        FNode** parent = &_root;
        for (;;) {
            FNode* const n = *parent;
            if (Likely(!((uintptr_t)n & 1))) { // not a leaf ?
                const uintptr_t nbit = (n->Keys[0] & 0xFF);
                const uintptr_t mask = (~uintptr_t(1) << nbit);

                xcmp = (n->Keys[0] ^ key);

                if (xcmp & mask) { // prefix doesn't match : insert a new node behind this one
                    nkey = (n->Keys[0] & ~uintptr_t(0xFF));
                    break;
                }
                else { // continue the descent
                    const uintptr_t branch = ((key >> nbit) & 1);
                    nkey = (n->Keys[branch] & ~uintptr_t(0xFF));
                    parent = &n->Children[branch];
                }
            }
            else {
                if (Likely(uintptr_t(n) != NullSentinel_)) { // insert
                    xcmp = (nkey ^ key);
                    Assert(xcmp & ~uintptr_t(0xFF)); // key already inserted !
                    break;
                }
                else { // replace sentinel with new node
                    *parent = newNode;
                    newNode->Keys[0] = key;
                    newNode->Keys[1] = 0;
                    newNode->Children[0] = (FNode*)(value | 1);
                    newNode->Children[1] = (FNode*)NullSentinel_;
                    return;
                }
            }
        }

        Assert(xcmp);

        u32 shift = 0;
        FPlatformMaths::bsr(&shift, xcmp);
        Assert((shift & 0xFF) == shift);

        const uintptr_t branch = ((key >> shift) & 1);
        newNode->Keys[branch] = key;
        newNode->Keys[branch ^ 1] = nkey;
        newNode->Keys[0] |= shift;
        newNode->Children[branch] = (FNode*)(value | 1);
        newNode->Children[branch ^ 1] = *parent;

        *parent = newNode;
    }

    FORCE_INLINE uintptr_t EraseAt_(FNode** parent, uintptr_t* pkey, u32 branch) NOEXCEPT {
        Assert(parent);
        FNode* const n = *parent;
        Assert(n);
        Assert_NoAssume(0 == (uintptr_t(n) & 1));

        FNode* const child = n->Children[branch]; // current child node
        FNode* const other = n->Children[branch ^ 1];

        Assert_NoAssume(uintptr_t(child) & 1); // node's key is probably broken :(
        Assert_NoAssume(uintptr_t(child) != NullSentinel_); // node's key is probably broken :(

        if ((uintptr_t(other) & 1) && uintptr_t(other) != NullSentinel_) { // if other node is not a pointer
            Assert(pkey);
            *pkey = (n->Keys[branch ^ 1] & ~uintptr_t(0xFF)) | ((*pkey) & 0xFF);
        }

        *parent = other;
        *(FNode**)n = _freeList;
        _freeList = n;

        ONLY_IF_MEMORYDOMAINS(_trackingDataRef->DeallocateUser(sizeof(FNode)));

        return ((uintptr_t)child & ~uintptr_t(1));
    }

    template <typename _Predicate>
    bool DeleteIf_Recursive_(const _Predicate& pred, FNode** parent, uintptr_t* pkey) NOEXCEPT {
    RESET_TO_PARENT: // avoid keeping track of visited nodes
        Assert_NoAssume(not (uintptr_t(*parent) & 1));
        FNode* const n = *parent;
        FNode* const child = n->Children[0]; // left child

        if (uintptr_t(child) & 1) { // leaf
            if (NullSentinel_ != uintptr_t(child) &&
                pred(n->Keys[0] & ~uintptr_t(0xFF), (uintptr_t)child & ~uintptr_t(1)) ) {
                EraseAt_(parent, pkey, 0);
                return false; // goto RESET_TO_PARENT in callee
            }
        }
        else {
            if (not DeleteIf_Recursive_(pred, &n->Children[0], &n->Keys[0]))
                goto RESET_TO_PARENT;
        }

        FNode* const other = n->Children[1]; // right child
        if (uintptr_t(other) & 1) { // leaf
            if (NullSentinel_ != uintptr_t(other) &&
                pred(n->Keys[1] & ~uintptr_t(0xFF), (uintptr_t)other & ~uintptr_t(1)) ) {
                EraseAt_(parent, pkey, 1);
            }
        }
        else {
            if (not DeleteIf_Recursive_(pred, &n->Children[1], &n->Keys[1]))
                goto RESET_TO_PARENT;
        }

        return true;
    }
};
//----------------------------------------------------------------------------
using FCompressedRadixTrie = TCompressedRadixTrie<>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator = FPageAllocator>
class TReadWriteCompressedRadixTrie : private TCompressedRadixTrie<_Allocator> {
    using parent_type = TCompressedRadixTrie<_Allocator>;
public:
    explicit TReadWriteCompressedRadixTrie(ARG0_IF_MEMORYDOMAINS(FMemoryTracking& trackingData)) NOEXCEPT
#if USE_PPE_MEMORYDOMAINS
    :   parent_type(trackingData)
#endif
    {}

    using parent_type::empty;

    void Insert(uintptr_t key, uintptr_t value) {
        const FReadWriteLock::FScopeLockWrite scopeWrite(_rwlock);
        parent_type::Insert(key, value);
    }

    uintptr_t Lookup(uintptr_t key) const NOEXCEPT {
        const FReadWriteLock::FScopeLockRead scopeRead(_rwlock);
        return parent_type::Lookup(key);
    }

    bool Find(uintptr_t* pvalue, uintptr_t key) const NOEXCEPT {
        const FReadWriteLock::FScopeLockRead scopeRead(_rwlock);
        return parent_type::Find(pvalue, key);
    }

    uintptr_t Erase(uintptr_t key) NOEXCEPT {
        const FReadWriteLock::FScopeLockWrite scopeWrite(_rwlock);
        return parent_type::Erase(key);
    }

    template <typename _Predicate>
    void DeleteIf(_Predicate&& pred) NOEXCEPT {
        const FReadWriteLock::FScopeLockWrite scopeWrite(_rwlock);
        parent_type::DeleteIf(std::move(pred));
    }

    template <typename _Predicate>
    bool Where(_Predicate&& pred) const NOEXCEPT {
        const FReadWriteLock::FScopeLockRead scopeRead(_rwlock);
        return parent_type::Where(std::move(pred));
    }

    template <typename _Functor>
    auto Foreach(_Functor&& functor) const NOEXCEPT {
        const FReadWriteLock::FScopeLockRead scopeRead(_rwlock);
        return parent_type::Foreach(std::move(functor));
    }

private:
    FReadWriteLock _rwlock;
};
//----------------------------------------------------------------------------
using FReadWriteCompressedRadixTrie = TReadWriteCompressedRadixTrie<>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
