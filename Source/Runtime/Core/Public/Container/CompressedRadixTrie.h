#pragma once

#include "Core_fwd.h"

#include "HAL/PlatformMaths.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"
#include "Memory/VirtualMemory.h"

#define USE_PPE_COMPRESSEDRADIXTRIE_MUTEX (1)

#if USE_PPE_COMPRESSEDRADIXTRIE_MUTEX
#   include "Thread/ReadWriteLock.h"
#else
#   include "Thread/AtomicSpinLock.h"
#endif

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

#if USE_PPE_COMPRESSEDRADIXTRIE_MUTEX
#   define COMPRESSEDRADIXTRIE_READER_SCOPE(_RWLOCK)  const FReadWriteLock::FScopeLockRead ANONYMIZE(scopeReader)(_RWLOCK)
#   define COMPRESSEDRADIXTRIE_WRITER_SCOPE(_RWLOCK)  const FReadWriteLock::FScopeLockWrite ANONYMIZE(scopeWriter)(_RWLOCK)
#else
#   define COMPRESSEDRADIXTRIE_READER_SCOPE(_RWLOCK)  const FAtomicTicketRWLock::FReaderScope ANONYMIZE(scopeReader)(_RWLOCK)
#   define COMPRESSEDRADIXTRIE_WRITER_SCOPE(_RWLOCK)  const FAtomicTicketRWLock::FWriterScope ANONYMIZE(scopeWiter)(_RWLOCK)
#endif

    void Insert(const void* pkey, const void* pvalue) { Insert(uintptr_t(pkey), uintptr_t(pvalue)); }
    void Insert(uintptr_t key, uintptr_t value) {
        Assert(!(key & 0xFF));
        Assert(!(value & 1));
        Assert(value);

        COMPRESSEDRADIXTRIE_WRITER_SCOPE(_rwlock);

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

    void* Lookup(const void* pkey) const NOEXCEPT { return reinterpret_cast<void*>(Lookup(uintptr_t(pkey))); }
    uintptr_t Lookup(uintptr_t key) const NOEXCEPT {
        Assert(!(key & 0xFF));

        COMPRESSEDRADIXTRIE_READER_SCOPE(_rwlock);

        return Lookup_AssumeLocked_(key);
    }

    bool Find(void** pvalue, const void* pkey) const NOEXCEPT { return Find((uintptr_t*)pvalue, uintptr_t(pkey)); }
    bool Find(uintptr_t* pvalue, uintptr_t key) const NOEXCEPT {
        Assert(pvalue);
        Assert(!(key & 0xFF));

        COMPRESSEDRADIXTRIE_READER_SCOPE(_rwlock);

        return Find_AssumeLocked_(pvalue, key);
    }

    void* Erase(const void* pkey) NOEXCEPT { return reinterpret_cast<void*>(Erase(uintptr_t(pkey))); }
    uintptr_t Erase(uintptr_t key) NOEXCEPT {
        Assert(!(key & 0xFF));
        Assert(uintptr_t(_root) != NullSentinel_); // can't erase from empty trie !

        COMPRESSEDRADIXTRIE_WRITER_SCOPE(_rwlock);

        return Erase_AssumeLocked_(key);
    }

    template <typename _Predicate>
    void DeleteIf(_Predicate&& pred) NOEXCEPT {
        COMPRESSEDRADIXTRIE_WRITER_SCOPE(_rwlock);

        DeleteIf_AssumeLocked_(std::forward<_Predicate>(pred));
    }

    template <typename _Predicate>
    bool TryDeleteIf(_Predicate&& pred) NOEXCEPT {
        if (_rwlock.TryLockRead()) {
            DeleteIf_AssumeLocked_(std::forward<_Predicate>(pred));
            _rwlock.UnlockRead();
            return true;
        }
        return false;
    }

    template <typename _Predicate>
    bool Where(_Predicate&& pred) const NOEXCEPT {
        COMPRESSEDRADIXTRIE_READER_SCOPE(_rwlock);
        return Foreach_AssumeLocked_(std::forward<_Predicate>(pred));
    }

    template <typename _Functor>
    void Foreach(_Functor&& functor) const NOEXCEPT {
        COMPRESSEDRADIXTRIE_READER_SCOPE(_rwlock);
        Foreach_AssumeLocked_(std::forward<_Functor>(functor));
    }

    template <typename _Functor>
    bool TryForeach(_Functor&& functor) const NOEXCEPT {
        if (_rwlock.TryLockRead()) {
            Foreach_AssumeLocked_(std::forward<_Functor>(functor));
            _rwlock.UnlockRead();
            return true;
        }
        return false;
    }

private:
    STATIC_CONST_INTEGRAL(uintptr_t, NullSentinel_, 1);
    STATIC_CONST_INTEGRAL(size_t, TriePageSize_, ALLOCATION_GRANULARITY);

#if USE_PPE_COMPRESSEDRADIXTRIE_MUTEX
    mutable FReadWriteLock _rwlock;
#else
    mutable FAtomicTicketRWLock _rwlock;
#endif

    FNode* _root;
    FNode* _freeList;
    FNode* _newPageAllocated;

#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking* const _trackingDataRef;
#endif

    void Insert_AssumeLocked_(const uintptr_t key, const uintptr_t value, FNode* const newNode) {
        uintptr_t xcmp = 0;
        uintptr_t nkey = (uintptr_t(_root) == NullSentinel_ ? 0 : _root->Keys[0]);

#if USE_PPE_MEMORYDOMAINS
        _trackingDataRef->AllocateUser(sizeof(FNode));
#endif

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

    uintptr_t Lookup_AssumeLocked_(uintptr_t key) const NOEXCEPT {
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

    bool Find_AssumeLocked_(uintptr_t* pvalue, uintptr_t key) const NOEXCEPT {
        const FNode* node = _root;
        const uintptr_t* pkey = nullptr;

        while (!((uintptr_t)node & 1)) {
            int branch = ((key >> (node->Keys[0] & 0xFF)) & 1);
            pkey = &node->Keys[branch];
            node = node->Children[branch];
        }

        if (Likely(pkey && (*pkey & ~uintptr_t(0xFF)) == key)) {
            *pvalue = ((uintptr_t)node & ~uintptr_t(1));
            return true;
        }
        else {
            return false;
        }
    }

    FORCE_INLINE uintptr_t EraseAt_AssumeLocked_(FNode** parent, uintptr_t* pkey, u32 branch) NOEXCEPT {
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

#if USE_PPE_MEMORYDOMAINS
        _trackingDataRef->DeallocateUser(sizeof(FNode));
#endif

        return ((uintptr_t)child & ~uintptr_t(1));
    }

    uintptr_t Erase_AssumeLocked_(uintptr_t key) NOEXCEPT {
        FNode** parent = &_root;
        uintptr_t* pkey = nullptr;

        for (;;) {
            FNode* n = *parent;

            const u32 branch = u32((key >> (n->Keys[0] & 0xFF)) & 1);
            FNode* const child = n->Children[branch]; // current child node
            if (uintptr_t(child) & 1) { // leaf
                Assert((n->Keys[branch] & ~uintptr_t(0xFF)) == key);
                return EraseAt_AssumeLocked_(parent, pkey, branch);
            }

            pkey = &n->Keys[branch];
            parent = &n->Children[branch];
        }
    }

    template <typename _Predicate>
    void DeleteIf_AssumeLocked_(_Predicate&& pred) NOEXCEPT {
        if (NullSentinel_ != uintptr_t(_root))
            DeleteIf_AssumeLocked_(pred, &_root, nullptr);
    }

    template <typename _Predicate>
    bool DeleteIf_AssumeLocked_(const _Predicate& pred, FNode** parent, uintptr_t* pkey) NOEXCEPT {
    RESET_TO_PARENT: // avoid keeping track of visited nodes
        Assert_NoAssume(not (uintptr_t(*parent) & 1));
        FNode* const n = *parent;
        FNode* const child = n->Children[0]; // left child

        if (uintptr_t(child) & 1) { // leaf
            if (NullSentinel_ != uintptr_t(child) &&
                pred(n->Keys[0] & ~uintptr_t(0xFF), (uintptr_t)child & ~uintptr_t(1)) ) {
                EraseAt_AssumeLocked_(parent, pkey, 0);
                return false; // goto RESET_TO_PARENT in callee
            }
        }
        else {
            if (not DeleteIf_AssumeLocked_(pred, &n->Children[0], &n->Keys[0]))
                goto RESET_TO_PARENT;
        }

        FNode* const other = n->Children[1]; // right child
        if (uintptr_t(other) & 1) { // leaf
            if (NullSentinel_ != uintptr_t(other) &&
                pred(n->Keys[1] & ~uintptr_t(0xFF), (uintptr_t)other & ~uintptr_t(1)) ) {
                EraseAt_AssumeLocked_(parent, pkey, 1);
            }
        }
        else {
            if (not DeleteIf_AssumeLocked_(pred, &n->Children[1], &n->Keys[1]))
                goto RESET_TO_PARENT;
        }

        return true;
    }

    template <typename _Functor>
    auto Foreach_AssumeLocked_(_Functor&& foreach) const NOEXCEPT {
        using result_type = decltype(foreach(uintptr_t{}, uintptr_t{}));
        if (Likely(NullSentinel_ != uintptr_t(_root))) {
            CONSTEXPR u32 stack_capacity = 128;
            u32 stacked = 1;
            const FNode* stack[stack_capacity];
            stack[0] = _root;

            do {
                const FNode* node = stack[--stacked];

                forrange(branch, 0, u32(2)) {
                    if ((uintptr_t)node->Children[branch] & 1) {
                        if (uintptr_t(node->Children[branch]) != NullSentinel_) {
                            const uintptr_t key = (node->Keys[branch] & ~uintptr_t(0xFF));
                            const uintptr_t value  = ((uintptr_t)node->Children[branch] & ~uintptr_t(1));

                            IF_CONSTEXPR(std::is_same_v<void, result_type>)
                                foreach(key, value);
                            else if (const result_type result = foreach(key, value))
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

#undef COMPRESSEDRADIXTRIE_READER_SCOPE
#undef COMPRESSEDRADIXTRIE_WRITER_SCOPE
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#undef USE_PPE_COMPRESSEDRADIXTRIE_MUTEX
