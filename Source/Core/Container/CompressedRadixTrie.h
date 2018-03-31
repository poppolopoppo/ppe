#pragma once

#include "Core/Core.h"

#include "Core/Memory/MemoryDomain.h"
#include "Core/Memory/MemoryTracking.h"
#include "Core/Memory/VirtualMemory.h"
#include "Core/Meta/BitCount.h"
#include "Core/Thread/AtomicSpinLock.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Thread safe radix trie optimized for storing pointers with some limitations :
//  - Key must have 8 lower bits set to 0
//  - Value must have first lower bit set to 0
//  - Value can't be 0
//  - Memory allocated will never be released
//
class FCompressedRadixTrie {
public:
    struct ALIGN(16) FNode {
        uintptr_t Keys[2];
        FNode* Children[2];
    };

    FCompressedRadixTrie()
        : _root((FNode*)NullSentinel_)
        , _freeList(nullptr)
        , _newPageAllocated(nullptr)
    {}

    ~FCompressedRadixTrie() {
        // will *NEVER*r release memory allocated
        // intended only for debugging purposes
    }

    FCompressedRadixTrie(const FCompressedRadixTrie&) = delete;
    FCompressedRadixTrie& operator =(const FCompressedRadixTrie&) = delete;

    bool empty() const { return (NullSentinel_ == uintptr_t(_root)); }

    void Insert(uintptr_t key, uintptr_t value) {
        Assert(!(key & 0xFF));
        Assert(!(value & 1));
        Assert(value);

        FNode* newNode;
        _barrier.Lock();

        if (_freeList) {
            _freeList = *(FNode**)(newNode = _freeList);
        }
        else if (_newPageAllocated) {
            newNode = _newPageAllocated;
            if (!((uintptr_t)++_newPageAllocated & (ALLOCATION_GRANULARITY - 1)))
                _newPageAllocated = ((FNode**)_newPageAllocated)[-1];
        }
        else {
            _barrier.Unlock();

            // Memory allocated here will never be freed !
            // But we're talking about never more than a few megabytes
            // 1024 * 1024 / 32 = 32768 *ALIVE* allocations for 1 mo on x64 architecture, 65536 for x86
            newNode = (FNode*)FVirtualMemory::InternalAlloc(ALLOCATION_GRANULARITY);
            AssertRelease(newNode);

            Assert(((char**)((char*)newNode + ALLOCATION_GRANULARITY))[-1] == 0);

            _barrier.Lock();
            ((FNode**)((char*)newNode + ALLOCATION_GRANULARITY))[-1] = _newPageAllocated;//in case if other thread also have just allocated a new page
            _newPageAllocated = newNode + 1;
        }

        Insert_AssumeLocked_(key, value, newNode);

        _barrier.Unlock();
    }

    uintptr_t Lookup(uintptr_t key) const {
        Assert(!(key & 0xFF));

        const FAtomicSpinLock::FScope scopeLock(_barrier);
        return Lookup_AssumeLocked_(key);
    }

    uintptr_t Erase(uintptr_t key) {
        Assert(!(key & 0xFF));
        Assert(uintptr_t(_root) != NullSentinel_); // can't erase from empty trie !

        const FAtomicSpinLock::FScope scopeLock(_barrier);
        return Erase_AssumeLocked_(key);
    }

    template <typename _Functor>
    void Foreach(_Functor&& functor) const {
        const FAtomicSpinLock::FScope scopeLock(_barrier);
        Foreach_AssumeLocked_(functor);
    }

private:
    static constexpr uintptr_t NullSentinel_ = 1;

    mutable FAtomicSpinLock _barrier;

    FNode* _root;
    FNode* _freeList;
    FNode* _newPageAllocated;

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
                else {// continue the descent
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
        Meta::bit_scan_reverse(&shift, xcmp);
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

            const uintptr_t branch = ((key >> (n->Keys[0] & 0xFF)) & 1);
            FNode* const ch0 = n->Children[branch]; // current child node
            if ((uintptr_t)ch0 & 1) { // leaf
                FNode* const ch1 = n->Children[branch ^ 1];

                Assert((n->Keys[branch] & ~uintptr_t(0xFF)) == key);
                Assert(uintptr_t(ch0) != NullSentinel_); // node's key is probably broken :(

                if (((uintptr_t)ch1 & 1) && uintptr_t(ch1) != NullSentinel_) // if other node is not a pointer
                    *pkey = (n->Keys[branch ^ 1] & ~uintptr_t(0xFF)) | ((*pkey) & 0xFF);

                *parent = ch1;
                *(FNode**)n = _freeList;
                _freeList = n;

                return ((uintptr_t)ch0 & ~uintptr_t(1));
            }

            pkey = &n->Keys[branch];
            parent = &n->Children[branch];
        }

        AssertNotReached(); // key does not exists !
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
} //!namespace Core
