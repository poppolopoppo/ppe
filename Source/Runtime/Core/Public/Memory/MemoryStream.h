#pragma once

#include "Core.h"

#include "Container/RawStorage.h"
#include "HAL/PlatformMemory.h"
#include "IO/StreamProvider.h"
#include "Memory/RefPtr.h"
#include "Misc/Function.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define MEMORYSTREAM(_DOMAIN) \
    ::PPE::TMemoryStream< ALLOCATOR(_DOMAIN) >
//----------------------------------------------------------------------------
#define MEMORYSTREAM_ALIGNED(_DOMAIN, _ALIGNMENT) \
    ::PPE::TMemoryStream< ALIGNED_ALLOCATOR(_DOMAIN, _ALIGNMENT) >
//----------------------------------------------------------------------------
#define MEMORYSTREAM_STACKLOCAL() \
    ::PPE::TMemoryStream< STACKLOCAL_ALLOCATOR() >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator = ALLOCATOR(Stream) >
class TMemoryStream : public IBufferedStreamReader, public IBufferedStreamWriter  {
public:
    typedef TRawStorage<u8, _Allocator> storage_type;
    typedef typename storage_type::allocator_type allocator_type;
    typedef typename storage_type::allocator_traits allocator_traits;

    TMemoryStream() = default;
    explicit TMemoryStream(allocator_type&& ralloc) NOEXCEPT;
    explicit TMemoryStream(const allocator_type& alloc) NOEXCEPT;
    explicit TMemoryStream(storage_type&& storage) NOEXCEPT;
    TMemoryStream(storage_type&& storage, std::streamsize size);

    TMemoryStream(const TMemoryStream&) = delete;
    TMemoryStream& operator =(const TMemoryStream&) = delete;

    TMemoryStream(TMemoryStream&& rvalue) NOEXCEPT { operator =(std::move(rvalue)); }
    TMemoryStream& operator =(TMemoryStream&& rvalue) NOEXCEPT;

    CONSTF u8* data() { return _storage.data(); }
    CONSTF const u8* data() const { return _storage.data(); }

    CONSTF u8* Pointer() { return _storage.Pointer(); }
    CONSTF const u8* Pointer() const { return _storage.Pointer(); }

    const storage_type& Storage() const { return _storage; }

    CONSTF size_t size() const { return _size; }
    CONSTF size_t capacity() const { return _storage.size(); }
    CONSTF bool empty() const { return 0 == _size; }

    void resize(size_t count, bool keepData = true);
    void reserve(size_t count);
    void reserve_Additional(size_t count);
    void shrink_to_fit();
    void clear();

    allocator_type& get_allocator() { return _storage.get_allocator(); }
    const allocator_type& get_allocator() const { return _storage.get_allocator(); }

    FRawMemory Append(size_t sizeInBytes);

    FRawMemory MakeView() { return FRawMemory(_storage.Pointer(), _size); }
    FRawMemory MakeView(size_t offset, size_t count) {
        Assert(offset + count < _size);
        return FRawMemory(&_storage[offset], count);
    }

    TMemoryView<const u8> MakeView() const { return TMemoryView<const u8>(_storage.Pointer(), _size); }
    TMemoryView<const u8> MakeView(size_t offset, size_t count) const {
        Assert(offset + count < _size);
        return TMemoryView<const u8>(&_storage[offset], count);
    }

    void clear_ReleaseMemory();
    void clear_StealMemory(storage_type& storage);

    NODISCARD bool AcquireDataUnsafe(FAllocatorBlock b, size_t sz = 0) NOEXCEPT {
        if (_storage.AcquireDataUnsafe(b)) {
            _size = sz;
            _offsetI = _offsetO = 0;
            return true;
        }
        else {
            return false;
        }
    }

    NODISCARD FAllocatorBlock StealDataUnsafe(size_t* sz = nullptr) NOEXCEPT {
        const FAllocatorBlock b = _storage.StealDataUnsafe();
        if (b) {
            if (sz) *sz = _size;
            _offsetI = _offsetO = _size = 0;
        }
        return b;
    }

public: // IStreamReader
    virtual bool Eof() const NOEXCEPT override final;

    virtual bool IsSeekableI(ESeekOrigin ) const NOEXCEPT override final { return true; }

    virtual std::streamoff TellI() const NOEXCEPT override final;
    virtual std::streamoff SeekI(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final;

    virtual std::streamsize SizeInBytes() const NOEXCEPT override final;

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t ReadSome(void* storage, size_t eltsize, size_t count) override final;

public: // IBufferedStreamReader
    virtual bool Peek(char& ch) override final;
    virtual bool Peek(wchar_t& wch) override final;

    virtual bool ReadAt_SkipBuffer(const FRawMemory& storage, std::streamoff absolute) override final;

public: // IStreamWriter
    virtual bool IsSeekableO(ESeekOrigin ) const NOEXCEPT override final { return true; }

    virtual std::streamoff TellO() const NOEXCEPT override final;
    virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin policy = ESeekOrigin::Begin) override final;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) override final;

public: // IBufferedStreamWriter
    using typename IBufferedStreamWriter::read_f;
    virtual size_t StreamCopy(const read_f& read, size_t blockSz) override final;

    virtual void Flush() override final {}

private:
    storage_type _storage;

    size_t _size{ 0 };

    size_t _offsetI{ 0 };
    size_t _offsetO{ 0 };
};
//----------------------------------------------------------------------------
template <typename _Allocator>
TMemoryStream<_Allocator>::TMemoryStream(allocator_type&& ralloc) NOEXCEPT
    : _storage(std::move(ralloc)) {}
//----------------------------------------------------------------------------
template <typename _Allocator>
TMemoryStream<_Allocator>::TMemoryStream(const allocator_type& alloc) NOEXCEPT
    : _storage(alloc) {}
//----------------------------------------------------------------------------
template <typename _Allocator>
TMemoryStream<_Allocator>::TMemoryStream(storage_type&& storage) NOEXCEPT
    : _storage(std::move(storage)) {}
//----------------------------------------------------------------------------
template <typename _Allocator>
TMemoryStream<_Allocator>::TMemoryStream(storage_type&& storage, std::streamsize size)
    : _storage(std::move(storage)), _size(checked_cast<size_t>(size)) {
    Assert_NoAssume(_size <= _storage.size());
}
//----------------------------------------------------------------------------
template <typename _Allocator>
auto TMemoryStream<_Allocator>::operator =(TMemoryStream&& rvalue) NOEXCEPT -> TMemoryStream& {
    _size = rvalue._size;
    _offsetI = rvalue._offsetI;
    _offsetO = rvalue._offsetO;
    _storage = std::move(rvalue._storage);

    rvalue._size = 0;
    rvalue._offsetI = 0;
    rvalue._offsetO = 0;

    return (*this);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TMemoryStream<_Allocator>::resize(size_t count, bool keepData/* = true */) {
    if (keepData) {
        reserve(count);
    }
    else if (count > _storage.size()) {
        // can only grow, except in shrink_to_fit() or clear_ReleaseMemory()
        _storage.Resize_DiscardData(allocator_traits::SnapSize(_storage.get_allocator(), count));
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
        _storage.Resize_KeepData(allocator_traits::SnapSize(_storage.get_allocator(), count));
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TMemoryStream<_Allocator>::reserve_Additional(size_t count) {
    reserve(_offsetO + count);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TMemoryStream<_Allocator>::shrink_to_fit() {
    if (_size != _storage.size()) {
        Assert(_storage.size() > _size); // capacity cannot be smaller than size
        _storage.Resize_KeepData(allocator_traits::SnapSize(_storage.get_allocator(), _size));
    }
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TMemoryStream<_Allocator>::clear() {
    _size = _offsetI = _offsetO = 0;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
FRawMemory TMemoryStream<_Allocator>::Append(size_t sizeInBytes) {
    _size = Max(sizeInBytes + _offsetO, _size);

    if (_size > _storage.size()) { // doubles the storage size if there is not enough space
        const size_t newCapacity = allocator_traits::SnapSize(_storage.get_allocator(), Max(_size, _storage.size() * 2));
        Assert(newCapacity >= _size);
        _storage.Resize_KeepData(newCapacity);
    }

    Assert(_storage.size() >= _size);
    const FRawMemory reserved = _storage.MakeView().SubRange(_offsetO, sizeInBytes);

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
bool TMemoryStream<_Allocator>::Eof() const NOEXCEPT {
    Assert(_offsetI <= _size);
    return (_size == _offsetI);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
std::streamoff TMemoryStream<_Allocator>::TellI() const NOEXCEPT {
    Assert(_offsetI <= _size);
    return checked_cast<std::streamoff>(_offsetI);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
std::streamoff TMemoryStream<_Allocator>::SeekI(std::streamoff offset, ESeekOrigin origin/* = ESeekOrigin::Begin */) {
    switch (origin) {
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
std::streamsize TMemoryStream<_Allocator>::SizeInBytes() const NOEXCEPT {
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
template <typename _Allocator>
bool TMemoryStream<_Allocator>::ReadAt_SkipBuffer(const FRawMemory& storage, std::streamoff absolute) {
    if (absolute + storage.size() <= _storage.size()) {
        _storage.SubRange(checked_cast<size_t>(absolute), storage.size()).CopyTo(storage);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
// IStreamWriter
//----------------------------------------------------------------------------
template <typename _Allocator>
std::streamoff TMemoryStream<_Allocator>::TellO() const NOEXCEPT {
    Assert(_offsetO <= _size);
    return checked_cast<std::streamoff>(_offsetO);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
std::streamoff TMemoryStream<_Allocator>::SeekO(std::streamoff offset, ESeekOrigin policy /* = ESeekOrigin::Begin */) {
    switch (policy) {
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
    const FRawMemory reserved = Append(checked_cast<size_t>(sizeInBytes));
    Assert(std::streamsize(reserved.SizeInBytes()) == sizeInBytes);

    FPlatformMemory::Memcpy(reserved.data(), storage, reserved.SizeInBytes());
    return true;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
size_t TMemoryStream<_Allocator>::WriteSome(const void* storage, size_t eltsize, size_t count) {
    return (TMemoryStream::Write(storage, std::streamsize(eltsize) * count) ? count : 0);
}
//----------------------------------------------------------------------------
// IBufferedStreamWriter
//----------------------------------------------------------------------------
template <typename _Allocator>
size_t TMemoryStream<_Allocator>::StreamCopy(const read_f& read, size_t blockSz) {
    reserve_Additional(blockSz);

    blockSz = read(_storage.MakeView().SubRange(_offsetO, blockSz));

    _offsetO += blockSz;
    _size = Max(_offsetO, _size);

    return blockSz;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
