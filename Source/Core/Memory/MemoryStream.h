#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator = ALLOCATOR(Container, T)>
class MemoryStream : _Allocator {
public:
    typedef T value_type;
    typedef typename std::add_pointer<T>::type pointer;
    typedef typename std::add_pointer<const T>::type const_pointer;
    typedef typename std::add_lvalue_reference<T>::type reference;
    typedef typename std::add_lvalue_reference<const T>::type const_reference;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef _Allocator allocator_type;

    STATIC_CONST_INTEGRAL(size_type, BookKeepingOverhead, (size_type)std::ceil((0.5f + sizeof(T *))/sizeof(T)) );
    STATIC_CONST_INTEGRAL(size_type, DefaultBlockSize, 4096 - BookKeepingOverhead);

    explicit MemoryStream(size_type blockSize = DefaultBlockSize);
    MemoryStream(size_type blockSize, allocator_type&& alloc);
    MemoryStream(size_type blockSize, const allocator_type& alloc);
    ~MemoryStream();

    MemoryStream(const MemoryStream& other) = delete;
    MemoryStream& operator =(const MemoryStream& other) = delete;

    MemoryStream(MemoryStream&& rvalue);
    MemoryStream& operator =(MemoryStream&& rvalue);

    size_type BlockSize() const { return _blockSize; }
    size_type BlockCount() const;

    size_type size() const { return _size; }
    bool empty() const { return 0 == _size; }
    size_type capacity() const { return BlockCount() * _blockSize; }

    void push_back(T&& rvalue);
    void push_back(const T& value);
    template <typename... _Args>
    void emplace_back(const _Args& args...);
    void clear();

    MemoryView<T> FirstBlock();
    MemoryView<T> NextBlock(const MemoryView<T>& block) const;

    MemoryView<const T> FirstBlock() const { return FirstBlock().Cast<const T>(); }
    MemoryView<const T> NextBlock(const MemoryView<const T>& block) const { return NextBlock(MemoryView<T>(const_cast<T *>(block.Pointer()), block.size())).Cast<const T>(); }

private:
    void GrowOneIFN_();

    size_type _blockSize;
    size_type _size;
    T *_lastBlock;
    T *_firstBlock;
};
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
MemoryStream<T, _Allocator>::MemoryStream(size_type blockSize /* = 4096 - 1 */)
:   _blockSize(blockSize)
,   _size(0)
,   _lastBlock(nullptr)
,   _firstBlock(nullptr) {
    Assert(blockSize > 1);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
MemoryStream<T, _Allocator>::MemoryStream(size_type blockSize, allocator_type&& alloc)
:   allocator_type(std::move(alloc))
,   _blockSize(blockSize)
,   _size(0)
,   _lastBlock(nullptr)
,   _firstBlock(nullptr) {
    Assert(blockSize > 1);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
MemoryStream<T, _Allocator>::MemoryStream(size_type blockSize, const allocator_type& alloc)
:   allocator_type(alloc)
,   _blockSize(blockSize)
,   _size(0)
,   _lastBlock(nullptr)
,   _firstBlock(nullptr) {
    Assert(blockSize > 1);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
MemoryStream<T, _Allocator>::~MemoryStream() {
    clear();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
MemoryStream<T, _Allocator>::MemoryStream(MemoryStream&& rvalue)
:   _blockSize(rvalue._blockSize)
,   _size(rvalue._size)
,   _lastBlock(rvalue._lastBlock)
,   _firstBlock(rvalue._firstBlock){
    rvalue._size = 0;
    rvalue._firstBlock = rvalue._lastBlock = nullptr;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto MemoryStream<T, _Allocator>::operator =(MemoryStream&& rvalue) -> MemoryStream& {
    _blockSize = rvalue._blockSize;
    _size = rvalue._size;
    _lastBlock = rvalue._lastBlock;
    _firstBlock = rvalue._firstBlock;

    rvalue._size = 0;
    rvalue._firstBlock = rvalue._lastBlock = nullptr;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto MemoryStream<T, _Allocator>::BlockCount() const -> size_type {
    size_type result = 0;
    for (const T *pBlock = _firstBlock; pBlock; pBlock = pBlock = *reinterpret_cast<const T**>(pBlock) )
        ++result;
    return result;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void MemoryStream<T, _Allocator>::push_back(T&& rvalue) {
    GrowOneIFN_();

    Assert(_lastBlock);

    ++_size;
    new ((void *)&(_lastBlock + BookKeepingOverhead + (_size % _blockSize))) T(std::move(rvalue));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void MemoryStream<T, _Allocator>::push_back(const T& value) {
    GrowOneIFN_();

    Assert(_lastBlock);

    ++_size;
    new ((void *)&(_lastBlock + BookKeepingOverhead + (_size % _blockSize))) T(value);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename... _Args>
void MemoryStream<T, _Allocator>::emplace_back(const _Args& args...) {
    GrowOneIFN_();

    Assert(_lastBlock);

    ++_size;
    new ((void *)&(_lastBlock + BookKeepingOverhead + (_size % _blockSize))) T(std::forward(args));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void MemoryStream<T, _Allocator>::clear() {

    const T *pBlock = _firstBlock;
    Assert(pBlock || nullptr == _lastBlock);

    while (pBlock) {
        const T *nextBlock = *reinterpret_cast<const T **>(pBlock);
        Assert(nextBlock || pBlock == _lastBlock);

        if (!std::is_pod<T>::value) {
            T *firstElt = const_cast<T *>(pBlock + BookKeepingOverhead);
            for (size_t i = 0; i < _blockSize; ++i)
                firstElt[i]->~T();
        }

        allocator_type::deallocate(pBlock, _blockSize + BookKeepingOverhead);
        pBlock = nextBlock;
    }

    _size = 0;
    _firstBlock = _lastBlock = nullptr;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
MemoryView<T> MemoryStream<T, _Allocator>::FirstBlock() {
    if (0 == _size) {
        Assert(nullptr == _firstBlock);
        Assert(nullptr == _lastBlock);
        return MemoryView<T>();
    }
    else {
        Assert(_firstBlock);
        Assert(_lastBlock);
        return MemoryView<T>(_firstBlock + BookKeepingOverhead, std::min(_size, _blockSize));
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
MemoryView<T> MemoryStream<T, _Allocator>::NextBlock(const MemoryView<T>& block) {
    Assert(block.Pointer()); // else should not call this method
    Assert(_size);
    Assert(_firstBlock);
    Assert(_lastBlock);

    T *const nextBlock = *reinterpret_cast<T **>(block.Pointer() - BookKeepingOverhead);
    Assert(nextBlock || _lastBlock == block.Pointer() - BookKeepingOverhead);

    if (!nextBlock)
        return MemoryView<T>();

    const nextBlockSize = nextBlock == _lastBlock ? _size % _blockSize : _blockSize;
    return MemoryView<T>(nextBlock + BookKeepingOverhead, nextBlockSize);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void MemoryStream<T, _Allocator>::GrowOneIFN_() {
    if (nullptr == _lastBlock) {
        Assert(0 == _size);
        Assert(nullptr == _lastBlock);

        _firstBlock = _lastBlock = allocator_type::allocate(_blockSize + BookKeepingOverhead);
        *reinterpret_cast<T **>(_firstBlock) = nullptr;
    }
    else if (0 == (_size + 1) % _blockSize) {
        Assert(_size > 1);
        Assert(_firstBlock);
        Assert(_lastBlock);

        T *const nextBlock = allocator_type::allocate(_blockSize + BookKeepingOverhead);

        *reinterpret_cast<T **>(nextBlock) = nullptr;
        *reinterpret_cast<T **>(_lastBlock) = nextBlock;

        _lastBlock = nextBlock;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
