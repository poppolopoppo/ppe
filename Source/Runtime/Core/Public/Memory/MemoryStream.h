#pragma once

#include "Core.h"

#include "Container/RawStorage.h"
#include "HAL/PlatformMemory.h"
#include "IO/StreamProvider.h"
#include "Memory/RefPtr.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define MEMORYSTREAM(_DOMAIN) \
    ::PPE::TMemoryStream<ALLOCATOR(_DOMAIN, u8) >
//----------------------------------------------------------------------------
#define MEMORYSTREAM_ALIGNED(_DOMAIN, _ALIGNMENT) \
    ::PPE::TMemoryStream<ALIGNED_ALLOCATOR(_DOMAIN, u8, _ALIGNMENT)>
//----------------------------------------------------------------------------
#define MEMORYSTREAM_STACK() \
    ::PPE::TMemoryStream<STACK_ALLOCATOR(u8)>
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator = ALLOCATOR(Stream, u8)>
class TMemoryStream : public IBufferedStreamReader, public IBufferedStreamWriter  {
public:
    typedef TRawStorage<u8, _Allocator> storage_type;
    typedef typename storage_type::allocator_type allocator_type;

    TMemoryStream();
    explicit TMemoryStream(allocator_type&& alloc);
    explicit TMemoryStream(storage_type&& storage);
    TMemoryStream(storage_type&& storage, std::streamsize size);
    TMemoryStream(allocator_type&& allocator, const TMemoryView<u8>& stolen);

    u8* Pointer() { return _storage.Pointer(); }
    const u8* Pointer() const { return _storage.Pointer(); }
    const storage_type& Storage() const { return _storage; }

    size_t size() const { return _size; }
    size_t capacity() const { return _storage.size(); }
    bool empty() const { return 0 == _size; }

    void resize(size_t count, bool keepData = true);
    void reserve(size_t count);
    void shrink_to_fit();
    void clear();

    TMemoryView<u8> Append(size_t sizeInBytes);

    TMemoryView<u8> MakeView() { return TMemoryView<u8>(_storage.Pointer(), _size); }
    TMemoryView<u8> MakeView(size_t offset, size_t count) {
        Assert(offset + count < _size);
        return TMemoryView<u8>(&_storage[offset], count);
    }

    TMemoryView<const u8> MakeView() const { return TMemoryView<const u8>(_storage.Pointer(), _size); }
    TMemoryView<const u8> MakeView(size_t offset, size_t count) const {
        Assert(offset + count < _size);
        return TMemoryView<const u8>(&_storage[offset], count);
    }

    void clear_ReleaseMemory();
    void clear_StealMemory(storage_type& storage);

    template <typename _OtherAllocator>
    auto StealDataUnsafe(_OtherAllocator& alloc, size_t* plen = nullptr) {
        using other_value_type = typename _OtherAllocator::value_type;
        const TMemoryView<other_value_type> stolen = _storage.StealDataUnsafe(alloc);
        if (plen) {
            Assert(Meta::IsAligned(sizeof(other_value_type), _size));
            *plen = (_size / sizeof(other_value_type));
        }
        _offsetI = _offsetO = _size = 0;
        return stolen;
    }

public: // IStreamReader
    virtual bool Eof() const override final;

    virtual bool IsSeekableI(ESeekOrigin ) const override final { return true; }

    virtual std::streamoff TellI() const override final;
    virtual std::streamoff SeekI(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final;

    virtual std::streamsize SizeInBytes() const override final;

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t ReadSome(void* storage, size_t eltsize, size_t count) override final;

public: // IBufferedStreamReader
    virtual bool Peek(char& ch) override final;
    virtual bool Peek(wchar_t& wch) override final;

public: // IStreamWriter
    virtual bool IsSeekableO(ESeekOrigin ) const override final { return true; }

    virtual std::streamoff TellO() const override final;
    virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin policy = ESeekOrigin::Begin) override final;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) override final;

public: // IBufferedStreamWriter
    virtual void Flush() override final {}

private:
    size_t _size;

    size_t _offsetI;
    size_t _offsetO;

    storage_type _storage;
};
//----------------------------------------------------------------------------
template <typename _Allocator>
TMemoryStream<_Allocator>::TMemoryStream()
    : _size(0), _offsetI(0), _offsetO(0) {}
//----------------------------------------------------------------------------
template <typename _Allocator>
TMemoryStream<_Allocator>::TMemoryStream(allocator_type&& alloc)
    : _size(0), _offsetI(0), _offsetO(0)
    , _storage(std::move(alloc)) {}
//----------------------------------------------------------------------------
template <typename _Allocator>
TMemoryStream<_Allocator>::TMemoryStream(storage_type&& storage)
    : _size(0), _offsetI(0), _offsetO(0)
    , _storage(std::move(storage)) {}
//----------------------------------------------------------------------------
template <typename _Allocator>
TMemoryStream<_Allocator>::TMemoryStream(storage_type&& storage, std::streamsize size)
    : _size(checked_cast<size_t>(size)), _offsetI(0), _offsetO(0)
    , _storage(std::move(storage)) {
    Assert(_size <= _storage.size());
}
//----------------------------------------------------------------------------
template <typename _Allocator>
TMemoryStream<_Allocator>::TMemoryStream(allocator_type&& allocator, const TMemoryView<u8>& stolen)
    : _size(0), _offsetI(0), _offsetO(0)
    , _storage(std::move(allocator), stolen)
{}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TMemoryStream<_Allocator>::resize(size_t count, bool keepData/* = true */) {
    if  (keepData) {
        reserve(count);
    }
    else if (count > _storage.size()) {
        // can only grow, except in shrink_to_fit() or clear_ReleaseMemory()
        _storage.Resize_DiscardData(SafeAllocatorSnapSize(_storage.get_allocator(), count));
    }

    _size = count;
    Assert(_storage.size() >= _size);

    if (_offsetI > _size)
        _offsetI = _size;

    if (_offsetO > _size)
        _offsetO = _size;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TMemoryStream<_Allocator>::reserve(size_t count) {
    if (count > _storage.size()) // can only grow, except in shrink_to_fit() or clear_ReleaseMemory()
        _storage.Resize_KeepData(SafeAllocatorSnapSize(_storage.get_allocator(), count));
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TMemoryStream<_Allocator>::shrink_to_fit() {
    if (_size != _storage.size()) {
        Assert(_storage.size() > _size); // capacity cannot be smaller than size
        _storage.Resize_KeepData(SafeAllocatorSnapSize(_storage.get_allocator(), _size));
    }
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TMemoryStream<_Allocator>::clear() {
    _size = _offsetI = _offsetO = 0;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
TMemoryView<u8> TMemoryStream<_Allocator>::Append(size_t sizeInBytes) {
    _size = Max(sizeInBytes + _offsetO, _size);

    if (_size > _storage.size()) { // doubles the storage size if there is not enough space
        const size_t newCapacity = SafeAllocatorSnapSize(_storage.get_allocator(), Max(_size, _storage.size() * 2));
        Assert(newCapacity >= _size);
        _storage.Resize_KeepData(newCapacity);
    }

    Assert(_storage.size() >= _size);
    const TMemoryView<u8> reserved = _storage.MakeView().SubRange(_offsetO, sizeInBytes);

    _offsetO += sizeInBytes;
    return reserved;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TMemoryStream<_Allocator>::clear_ReleaseMemory() {
    _size = _offsetI = _offsetO = 0;
    _storage.clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TMemoryStream<_Allocator>::clear_StealMemory(storage_type& storage) {
    _size = _offsetI = _offsetO = 0;
    storage = std::move(_storage);
}
//----------------------------------------------------------------------------
// IStreamReader
//----------------------------------------------------------------------------
template <typename _Allocator>
bool TMemoryStream<_Allocator>::Eof() const {
    Assert(_offsetI <= _size);
    return (_size == _offsetI);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
std::streamoff TMemoryStream<_Allocator>::TellI() const {
    Assert(_offsetI <= _size);
    return checked_cast<std::streamoff>(_offsetI);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
std::streamoff TMemoryStream<_Allocator>::SeekI(std::streamoff offset, ESeekOrigin origin/* = ESeekOrigin::Begin */) {
    switch (origin)
    {
    case ESeekOrigin::Begin:
        if (offset > checked_cast<std::streamoff>(_size))
            return std::streamoff(-1);
        _offsetI = checked_cast<size_t>(offset);
        break;
    case ESeekOrigin::Relative:
        if (checked_cast<std::streamoff>(_offsetI) + offset < 0 ||
            checked_cast<std::streamoff>(_offsetI) + offset > checked_cast<std::streamoff>(_size) )
            return std::streamoff(-1);
        _offsetI = checked_cast<size_t>(_offsetI + offset);
        break;
    case ESeekOrigin::End:
        if (checked_cast<std::streamoff>(_size) + offset < 0 ||
            checked_cast<std::streamoff>(_size) + offset > checked_cast<std::streamoff>(_size) )
            return std::streamoff(-1);
        _offsetI = checked_cast<size_t>(_size + offset);
        break;
    default:
        AssertNotImplemented();
    }
    return checked_cast<std::streamoff>(_offsetI);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
std::streamsize TMemoryStream<_Allocator>::SizeInBytes() const {
    return checked_cast<std::streamsize>(_size);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
bool TMemoryStream<_Allocator>::Read(void* storage, std::streamsize sizeInBytes) {
    const size_t sizeT = checked_cast<size_t>(sizeInBytes);
    if (_offsetI + sizeT > _size)
        return false;

    Assert(storage);
    FPlatformMemory::Memcpy(storage, _storage.Pointer() + _offsetI, sizeT);

    _offsetI += sizeT;
    return true;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
size_t TMemoryStream<_Allocator>::ReadSome(void* storage, size_t eltsize, size_t count) {
    Assert(_size >= _offsetI);
    Assert(eltsize > 0);

    const size_t remaining = (_size - _offsetI);
    const size_t wantedsize = eltsize*count;
    const size_t realsize = remaining < wantedsize ? remaining : wantedsize;

    return (TMemoryStream::Read(storage, realsize) ? (realsize/eltsize) : 0 );
}
//----------------------------------------------------------------------------
// IBufferedStreamReader
//----------------------------------------------------------------------------
template <typename _Allocator>
bool TMemoryStream<_Allocator>::Peek(char& ch) {
    Assert(_offsetI <= _size);
    if (_offsetI == _size)
        return false;

    ch = checked_cast<char>(_storage[_offsetI]);
    return true;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
bool TMemoryStream<_Allocator>::Peek(wchar_t& wch) {
    Assert(_offsetI <= _size);
    if (_offsetI + sizeof(wchar_t) > _size)
        return false;

    wch = *reinterpret_cast<const wchar_t*>(&_storage[_offsetI]);
    return true;
}
//----------------------------------------------------------------------------
// IStreamWriter
//----------------------------------------------------------------------------
template <typename _Allocator>
std::streamoff TMemoryStream<_Allocator>::TellO() const {
    Assert(_offsetO <= _size);
    return checked_cast<std::streamoff>(_offsetO);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
std::streamoff TMemoryStream<_Allocator>::SeekO(std::streamoff offset, ESeekOrigin policy /* = ESeekOrigin::Begin */) {
    switch (policy)
    {
    case ESeekOrigin::Begin:
        if (offset > checked_cast<std::streamoff>(_size))
            return std::streamoff(-1);
        _offsetO = checked_cast<size_t>(offset);
        break;
    case ESeekOrigin::Relative:
        if (checked_cast<std::streamoff>(_offsetO) + offset < 0 ||
            checked_cast<std::streamoff>(_offsetO) + offset > checked_cast<std::streamoff>(_size) )
            return std::streamoff(-1);
        _offsetO = checked_cast<size_t>(_offsetO + offset);
        break;
    case ESeekOrigin::End:
        if (checked_cast<std::streamoff>(_size) + offset < 0 ||
            checked_cast<std::streamoff>(_size) + offset > checked_cast<std::streamoff>(_size) )
            return std::streamoff(-1);
        _offsetO = checked_cast<size_t>(_size + offset);
        break;
    default:
        AssertNotImplemented();
    }
    return checked_cast<std::streamoff>(_offsetO);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
bool TMemoryStream<_Allocator>::Write(const void* storage, std::streamsize sizeInBytes) {
    const TMemoryView<u8> reserved = Append(checked_cast<size_t>(sizeInBytes));
    Assert(std::streamsize(reserved.SizeInBytes()) == sizeInBytes);

    FPlatformMemory::Memcpy(reserved.data(), storage, reserved.SizeInBytes());
    return true;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
size_t TMemoryStream<_Allocator>::WriteSome(const void* storage, size_t eltsize, size_t count) {
    return (TMemoryStream::Write(storage, eltsize * count) ? count : 0);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE