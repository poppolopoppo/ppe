#pragma once

#include "Core/Core.h"

#include "Core/Container/RawStorage.h"
#include "Core/IO/StreamProvider.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define MEMORYSTREAM(_DOMAIN) \
    ::Core::TMemoryStream<ALLOCATOR(_DOMAIN, u8) >
//----------------------------------------------------------------------------
#define MEMORYSTREAM_THREAD_LOCAL(_DOMAIN) \
    ::Core::TMemoryStream<THREAD_LOCAL_ALLOCATOR(_DOMAIN, u8) >
//----------------------------------------------------------------------------
#define MEMORYSTREAM_ALIGNED(_DOMAIN, _ALIGNMENT) \
    ::Core::TMemoryStream<ALIGNED_ALLOCATOR(_DOMAIN, u8, _ALIGNMENT)>
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator = ALLOCATOR(Stream, u8)>
class TMemoryStream : public IStreamReader, public IStreamWriter  {
public:
    typedef TRawStorage<u8, _Allocator> storage_type;

    TMemoryStream();
    explicit TMemoryStream(storage_type&& storage);
    TMemoryStream(storage_type&& storage, std::streamsize size);

    size_t size() const { return _size; }
    size_t capacity() const { return _storage.size(); }
    bool empty() const { return 0 == _size; }

    void resize(size_t count, bool keepData = true);
    void reserve(size_t count);
    void shrink_to_fit();
    void clear();

    TMemoryView<u8> Append(size_t sizeInBytes);

    void Clear_ReleaseMemory();
    void Clear_StealMemory(storage_type& storage);

    const storage_type& Storage() const { return _storage; }

    u8* Pointer() { return _storage.Pointer(); }
    const u8* Pointer() const { return _storage.Pointer(); }

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

public: // IStreamReader
    virtual bool Eof() const override;

    virtual bool IsSeekableI(ESeekOrigin ) const override { return true; }

    virtual std::streamoff TellI() const override;
    virtual bool SeekI(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override;

    virtual std::streamsize SizeInBytes() const override;

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override;
    virtual std::streamsize ReadSome(void* storage, size_t eltsize, std::streamsize count) override;

    virtual bool Peek(char& ch) override;
    virtual bool Peek(wchar_t& ch) override;

public: // IStreamWriter
    virtual bool IsSeekableO(ESeekOrigin ) const override { return true; }

    virtual std::streamoff TellO() const override;
    virtual bool SeekO(std::streamoff offset, ESeekOrigin policy = ESeekOrigin::Begin) override;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override;
    virtual bool WriteSome(const void* storage, size_t eltsize, std::streamsize count) override;

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
void TMemoryStream<_Allocator>::resize(size_t count, bool keepData/* = true */) {
    if  (keepData) {
        reserve(count);
    }
    else if (count > _storage.size()) {
        // can only grow, except in shrink_to_fit() or Clear_ReleaseMemory()
        _storage.Resize_DiscardData(count);
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
    if (count > _storage.size()) // can only grow, except in shrink_to_fit() or Clear_ReleaseMemory()
        _storage.Resize_KeepData(count);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TMemoryStream<_Allocator>::shrink_to_fit() {
    if (_size != _storage.size()) {
        Assert(_storage.size() > _size); // capacity cannot be smaller than size
        _storage.Resize_KeepData(_size);
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
        const size_t growCapacity = (_storage.size() ? _storage.size() * 2 : 128);
        const size_t newCapacity = Max(_size, growCapacity);
        _storage.Resize_KeepData(newCapacity);
    }

    Assert(_storage.size() >= _size);
    const TMemoryView<u8> reserved = _storage.MakeView().SubRange(_offsetO, sizeInBytes);

    _offsetO += sizeInBytes;
    return reserved;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TMemoryStream<_Allocator>::Clear_ReleaseMemory() {
    _size = _offsetI = _offsetO = 0;
    _storage.Clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TMemoryStream<_Allocator>::Clear_StealMemory(storage_type& storage) {
    storage = std::move(_storage);
    Clear_ReleaseMemory();
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
bool TMemoryStream<_Allocator>::SeekI(std::streamoff offset, ESeekOrigin origin/* = ESeekOrigin::Begin */) {
    switch (origin)
    {
    case ESeekOrigin::Begin:
        if (offset > checked_cast<std::streamoff>(_size))
            return false;
        _offsetI = checked_cast<size_t>(offset);
        break;
    case ESeekOrigin::Relative:
        if (checked_cast<std::streamoff>(_offsetI) + offset < 0 ||
            checked_cast<std::streamoff>(_offsetI) + offset > checked_cast<std::streamoff>(_size) )
            return false;
        _offsetI = checked_cast<size_t>(_offsetI + offset);
        break;
    case ESeekOrigin::End:
        if (checked_cast<std::streamoff>(_size) + offset < 0 ||
            checked_cast<std::streamoff>(_size) + offset > checked_cast<std::streamoff>(_size) )
            return false;
        _offsetI = checked_cast<size_t>(_size + offset);
        break;
    default:
        AssertNotImplemented();
        return false;
    }
    return true;
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
    memcpy(storage, _storage.Pointer() + _offsetI, sizeT);

    _offsetI += sizeT;
    return true;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
std::streamsize TMemoryStream<_Allocator>::ReadSome(void* storage, size_t eltsize, std::streamsize count) {
    Assert(_size >= _offsetI);
    Assert(eltsize > 0);
    const std::streamsize remaining = checked_cast<std::streamsize>(_size - _offsetI);
    const std::streamsize wantedsize = eltsize*count;
    const std::streamsize realsize = remaining < wantedsize ? remaining : wantedsize;
    return (TMemoryStream::Read(storage, eltsize * count) ) ? realsize : 0;
}
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
bool TMemoryStream<_Allocator>::SeekO(std::streamoff offset, ESeekOrigin policy /* = ESeekOrigin::Begin */) {
    switch (policy)
    {
    case ESeekOrigin::Begin:
        if (offset > checked_cast<std::streamoff>(_size))
            return false;
        _offsetO = checked_cast<size_t>(offset);
        break;
    case ESeekOrigin::Relative:
        if (checked_cast<std::streamoff>(_offsetO) + offset < 0 ||
            checked_cast<std::streamoff>(_offsetO) + offset > checked_cast<std::streamoff>(_size) )
            return false;
        _offsetO = checked_cast<size_t>(_offsetO + offset);
        break;
    case ESeekOrigin::End:
        if (checked_cast<std::streamoff>(_size) + offset < 0 ||
            checked_cast<std::streamoff>(_size) + offset > checked_cast<std::streamoff>(_size) )
            return false;
        _offsetO = checked_cast<size_t>(_size + offset);
        break;
    default:
        AssertNotImplemented();
        return false;
    }
    return true;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
bool TMemoryStream<_Allocator>::Write(const void* storage, std::streamsize sizeInBytes) {
    const TMemoryView<u8> reserved = Append(checked_cast<size_t>(sizeInBytes));
    Assert(std::streamsize(reserved.SizeInBytes()) == sizeInBytes);

    memcpy(reserved.data(), storage, reserved.SizeInBytes());
    return true;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
bool TMemoryStream<_Allocator>::WriteSome(const void* storage, size_t eltsize, std::streamsize count) {
    return TMemoryStream::Write(storage, eltsize * count);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
