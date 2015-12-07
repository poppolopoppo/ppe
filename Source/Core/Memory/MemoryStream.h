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
    ::Core::MemoryStream<ALLOCATOR(_DOMAIN, u8) >
//----------------------------------------------------------------------------
#define MEMORYSTREAM_THREAD_LOCAL(_DOMAIN) \
    ::Core::MemoryStream<THREAD_LOCAL_ALLOCATOR(_DOMAIN, u8) >
//----------------------------------------------------------------------------
#define MEMORYSTREAM_ALIGNED(_DOMAIN, _ALIGNMENT) \
    ::Core::MemoryStream<ALIGNED_ALLOCATOR(_DOMAIN, u8, _ALIGNMENT)>
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator = ALLOCATOR(Stream, u8)>
class MemoryStream : public IStreamReader, public IStreamWriter  {
public:
    typedef RawStorage<u8, _Allocator> storage_type;

    MemoryStream();
    explicit MemoryStream(storage_type&& storage);

    size_t size() const { return _size; }
    size_t capacity() const { return _storage.size(); }
    bool empty() const { return 0 == _size; }

    void resize(size_t count);
    void reserve(size_t count);
    void shrink_to_fit();
    void clear();

    void Clear_ReleaseMemory();
    void Clear_StealMemory(storage_type& storage);

    const storage_type& Storage() const { return _storage; }

    u8* Pointer() { return _storage.Pointer(); }
    const u8* Pointer() const { return _storage.Pointer(); }

    MemoryView<const u8> MakeView() const { return MemoryView<const u8>(_storage.Pointer(), _size); }
    MemoryView<const u8> MakeView(size_t offset, size_t count) const {
        Assert(offset + count < _size);
        return MemoryView<const u8>(&_storage[offset], count);
    }

public: // IStreamReader
    virtual bool Eof() const override;

    virtual std::streamoff TellI() const override;
    virtual bool SeekI(std::streamoff offset, SeekOrigin origin = SeekOrigin::Begin) override;

    virtual std::streamsize SizeInBytes() const override;

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override;
    virtual std::streamsize ReadSome(void* storage, size_t eltsize, std::streamsize count) override;

    virtual char PeekChar() override;
    virtual wchar_t PeekCharW() override;

public: // IStreamWriter
    virtual std::streamoff TellO() const override;
    virtual bool SeekO(std::streamoff offset, SeekOrigin policy = SeekOrigin::Begin) override;

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
MemoryStream<_Allocator>::MemoryStream()
    : _size(0), _offsetI(0), _offsetO(0) {}
//----------------------------------------------------------------------------
template <typename _Allocator>
MemoryStream<_Allocator>::MemoryStream(storage_type&& storage)
    : _size(0), _offsetI(0), _offsetO(0)
    , _storage(std::move(storage)) {}
//----------------------------------------------------------------------------
template <typename _Allocator>
void MemoryStream<_Allocator>::resize(size_t count) {
    reserve(count);

    _size = count;

    if (_offsetI > _size)
        _offsetI = _size;

    if (_offsetO > _size)
        _offsetO = _size;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void MemoryStream<_Allocator>::reserve(size_t count) {
    if (count > _storage.size()) // can only grow, except in shrink_to_fit() or Clear_ReleaseMemory()
        _storage.Resize_KeepData(count);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void MemoryStream<_Allocator>::shrink_to_fit() {
    if (_size != _storage.size()) {
        Assert(_storage.size() > _size); // capacity cannot be smaller than size
        _storage.Resize_KeepData(_size);
    }
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void MemoryStream<_Allocator>::clear() {
    _size = _offsetI = _offsetO = 0;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void MemoryStream<_Allocator>::Clear_ReleaseMemory() {
    _size = _offsetI = _offsetO = 0;
    _storage.Clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void MemoryStream<_Allocator>::Clear_StealMemory(storage_type& storage) {
    storage = std::move(_storage);
    Clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
// IStreamReader
//----------------------------------------------------------------------------
template <typename _Allocator>
bool MemoryStream<_Allocator>::Eof() const {
    Assert(_offsetI <= _size);
    return (_size == _offsetI);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
std::streamoff MemoryStream<_Allocator>::TellI() const {
    Assert(_offsetI <= _size);
    return checked_cast<std::streamoff>(_offsetI);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
bool MemoryStream<_Allocator>::SeekI(std::streamoff offset, SeekOrigin origin/* = SeekOrigin::Begin */) {
    switch (origin)
    {
    case SeekOrigin::Begin:
        if (offset > checked_cast<std::streamoff>(_size))
            return false;
        _offsetI = checked_cast<size_t>(offset);
        break;
    case SeekOrigin::Relative:
        if (checked_cast<std::streamoff>(_offsetI) + offset < 0 ||
            checked_cast<std::streamoff>(_offsetI) + offset > checked_cast<std::streamoff>(_size) )
            return false;
        _offsetI = checked_cast<size_t>(_offsetI + offset);
        break;
    case SeekOrigin::End:
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
std::streamsize MemoryStream<_Allocator>::SizeInBytes() const {
    return checked_cast<std::streamsize>(_size);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
bool MemoryStream<_Allocator>::Read(void* storage, std::streamsize sizeInBytes) {
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
std::streamsize MemoryStream<_Allocator>::ReadSome(void* storage, size_t eltsize, std::streamsize count) {
    Assert(_size >= _offsetI);
    Assert(eltsize > 0);
    const std::streamsize remaining = checked_cast<std::streamsize>(_size - _offsetI);
    const std::streamsize wantedsize = eltsize*count;
    const std::streamsize realsize = remaining < wantedsize ? remaining : wantedsize;
    return (MemoryStream::Read(storage, eltsize * count) ) ? realsize : 0;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
char MemoryStream<_Allocator>::PeekChar() {
    Assert(_offsetI <= _size);
    if (_offsetI == _size)
        return '\0';

    return checked_cast<char>(_storage[_offsetI]);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
wchar_t MemoryStream<_Allocator>::PeekCharW() {
    Assert(_offsetI <= _size);
    if (_offsetI + sizeof(wchar_t) > _size)
        return L'\0';

    return *reinterpret_cast<const wchar_t*>(&_storage[_offsetI]);
}
//----------------------------------------------------------------------------
// IStreamWriter
//----------------------------------------------------------------------------
template <typename _Allocator>
std::streamoff MemoryStream<_Allocator>::TellO() const {
    Assert(_offsetO <= _size);
    return checked_cast<std::streamoff>(_offsetO);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
bool MemoryStream<_Allocator>::SeekO(std::streamoff offset, SeekOrigin policy /* = SeekOrigin::Begin */) {
    switch (policy)
    {
    case SeekOrigin::Begin:
        if (offset > checked_cast<std::streamoff>(_size))
            return false;
        _offsetO = checked_cast<size_t>(offset);
        break;
    case SeekOrigin::Relative:
        if (checked_cast<std::streamoff>(_offsetO) + offset < 0 ||
            checked_cast<std::streamoff>(_offsetO) + offset > checked_cast<std::streamoff>(_size) )
            return false;
        _offsetO = checked_cast<size_t>(_offsetO + offset);
        break;
    case SeekOrigin::End:
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
bool MemoryStream<_Allocator>::Write(const void* storage, std::streamsize sizeInBytes) {
    const size_t sizeT = checked_cast<size_t>(sizeInBytes);
    _size = std::max(sizeT + _offsetO, _size);

    if (_size > _storage.size()) { // doubles the storage size if there is not enough space
        size_t newCapacity = std::max(_size, (_storage.size() + 1) * 2 );
        newCapacity = ROUND_TO_NEXT_128(newCapacity);
        _storage.Resize_KeepData(newCapacity);
    }

    Assert(_storage.size() >= _size);
    memcpy(_storage.Pointer() + _offsetO, storage, sizeT);

    _offsetO += sizeT;
    return true;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
bool MemoryStream<_Allocator>::WriteSome(const void* storage, size_t eltsize, std::streamsize count) {
    return MemoryStream::Write(storage, eltsize * count);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
