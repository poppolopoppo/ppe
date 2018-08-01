#pragma once

#include "Core.h"

#include "HAL/PlatformMaths.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"
#include "Memory/VirtualMemory.h"
#include "Thread/AtomicSpinLock.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Thread safe radix trie optimized for storing pointers with some limitations :
//  - Key must have 8 lower bits set to 0
//  - Value must have first lower bit set to 0
//  - Value can't be 0
//  - Memory allocated will never be released
// You shouldn't be using as a regular container, as it's more intended for low level
//
class FCompressedRadixTrie {
public:
    struct ALIGN(16) FNode {
        uintptr_t Keys[2];
        FNode* Children[2];
    };

#if USE_PPE_MEMORYDOMAINS
    FCompressedRadixTrie(FMemoryTracking& trackingData)
        : _root((FNode*)NullSentinel_)
        , _freeList(nullptr)
        , _newPageAllocated(nullptr)
        , _trackingDataRef(&trackingData)
#else
    FCompressedRadixTrie()
        : _root((FNode*)NullSentinel_)
        , _freeList(nullptr)
        , _newPageAllocated(nullptr)
#endif
    {}

    FCompressedRadixTrie(const FCompressedRadixTrie&) = delete;
    FCompressedRadixTrie& operator =(const FCompressedRadixTrie&) = delete;

    bool empty() const { return (NullSentinel_ == uintptr_t(_root)); }

    void Insert(uintptr_t key, uintptr_t value) {
        Assert(!(key & 0xFF));
        Assert(!(value & 1));
        Assert(value);

        const FAtomicOrderedLock::FScope scopeLock(_barrier);

        FNode* newNode;
        if (_freeList) {
            _freeList = *(FNode**)(newNode = _freeList);
        }
        else if (_newPageAllocated) {
            newNode = _newPageAllocated;
            if (!((uintptr_t)++_newPageAllocated & (TriePageSize_ - 1)))
                _newPageAllocated = ((FNode**)_newPageAllocated)[-1];
        }
        else {
            // !! this memory block will *NEVER* be released !!
#if USE_PPE_MEMORYDOMAINS
            FNode* const newPage = (FNode*)FVirtualMemory::InternalAlloc(TriePageSize_, *_trackingDataRef);
#else
            FNode* const newPage = (FNode*)FVirtualMemory::InternalAlloc(TriePageSize_);
#endif
            AssertRelease(newPage);

            // in case if other thread also have just allocated a new page
            Assert(((char**)((char*)newPage + TriePageSize_))[-1] == nullptr);
            ((FNode**)((char*)newPage + TriePageSize_))[-1] = _newPageAllocated;

            // eat first block and saves the rest for later insertions
            newNode = newPage;
            _newPageAllocated = newPage + 1;
        }

        Insert_AssumeLocked_(key, value, newNode);
    }

    uintptr_t Lookup(uintptr_t key) const {
        Assert(!(key & 0xFF));

        const FAtomicOrderedLock::FScope scopeLock(_barrier);

        return Lookup_AssumeLocked_(key);
    }

    uintptr_t Erase(uintptr_t key) {
        Assert(!(key & 0xFF));
        Assert(uintptr_t(_root) != NullSentinel_); // can't erase from empty trie !

        const FAtomicOrderedLock::FScope scopeLock(_barrier);

        return Erase_AssumeLocked_(key);
    }

    template <typename _Functor>
    void Foreach(_Functor&& functor) const {

        const FAtomicOrderedLock::FScope scopeLock(_barrier);

        Foreach_AssumeLocked_(functor);
    }

private:
    STATIC_CONST_INTEGRAL(uintptr_t, NullSentinel_, 1);
    STATIC_CONST_INTEGRAL(size_t, TriePageSize_, ALLOCATION_GRANULARITY);

    mutable FAtomicOrderedLock _barrier;

    FNode* _root;
    FNode* _freeList;
    FNode* _newPageAllocated;

#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking* const _trackingDataRef;
#endif

    void Insert_AssumeLocked_(const uintptr_t key, const uintptr_t value, FNode* const newNode) {
        uintptr_t xcmp = 0;
        uintptr_t nkey = (uintptr_t(_root) == NullSentinel_ ? 0 : _root->Keys[0]);

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

    uintptr_t Lookup_AssumeLocked_(uintptr_t key) const {
        const FNode* node = _root;
        const uintptr_t* pkey = nullptr;

        while (!((uintptr_t)node & 1)) {
            int branch = ((key >> (node->Keys[0] & 0xFF)) & 1);
            pkey = &node->Keys[branch];
            node = node->Children[branch];
        }

        Assert(pkey && (*pkey & ~uintptr_t(0xFF)) == key);
        return ((uintptr_t)node & ~uintptr_t(1)); // can't fail, key is *ALWAYS* here
    }

    uintptr_t Erase_AssumeLocked_(uintptr_t key) {
        FNode** parent = &_root;
        uintptr_t* pkey = nullptr;

        for (;;) {
            FNode* n = *parent;

            const size_t branch = ((key >> (n->Keys[0] & 0xFF)) & 1);
            FNode* const child = n->Children[branch]; // current child node
            if (uintptr_t(child) & 1) { // leaf
                FNode* const other = n->Children[branch ^ 1];

                Assert((n->Keys[branch] & ~uintptr_t(0xFF)) == key);
                Assert(uintptr_t(child) != NullSentinel_); // node's key is probably broken :(

                if ((uintptr_t(other) & 1) && uintptr_t(other) != NullSentinel_) // if other node is not a pointer
                    *pkey = (n->Keys[branch ^ 1] & ~uintptr_t(0xFF)) | ((*pkey) & 0xFF);

                *parent = other;
                *(FNode**)n = _freeList;
                _freeList = n;

                return ((uintptr_t)child & ~uintptr_t(1));
            }

            pkey = &n->Keys[branch];
            parent = &n->Children[branch];
        }
    }

    template <typename _Functor>
    void Foreach_AssumeLocked_(_Functor& foreach) const {
        if (NullSentinel_ == uintptr_t(_root))
            return;

        constexpr size_t stack_capacity = 128;

        size_t stacked = 1;
        const FNode* stack[stack_capacity];
        stack[0] = _root;

        do {
            const FNode* node = stack[--stacked];

            for (int b = 0; b < 2; ++b) {
                if ((uintptr_t)node->Children[b] & 1) {
                    if (uintptr_t(node->Children[b]) != NullSentinel_)
                        foreach((node->Keys[b] & ~uintptr_t(0xFF)),
                                ((uintptr_t)node->Children[b] & ~uintptr_t(1)));
                }
                else {
                    Assert(stacked < stack_capacity);
                    stack[stacked++] = node->Children[b];
                }
            }

        } while (stacked);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
