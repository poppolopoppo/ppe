#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/IO/StreamProvider.h"
#include "Core/Memory/MemoryDomain.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FBufferedStreamAllocator = THREAD_LOCAL_ALLOCATOR(Stream, u8);
//----------------------------------------------------------------------------
class FBufferedStreamReader : public IBufferedStreamReader, FBufferedStreamAllocator {
public:
    FBufferedStreamReader();
    explicit FBufferedStreamReader(IStreamReader* nonBuffered);
    ~FBufferedStreamReader();

    FBufferedStreamReader(const FBufferedStreamReader& ) = delete;
    FBufferedStreamReader& operator =(const FBufferedStreamReader& ) = delete;

    inline friend void swap(FBufferedStreamReader& lhs, FBufferedStreamReader rhs) {
        std::swap(lhs._nonBuffered, rhs._nonBuffered);
        std::swap(lhs._buffer, rhs._buffer);
        std::swap(lhs._origin, rhs._origin);
        std::swap(lhs._offset, rhs._offset);
        std::swap(lhs._capacity, rhs._capacity);
    }

public: // IStreamReader
    virtual bool Eof() const override final;

    virtual bool IsSeekableI(ESeekOrigin origin = ESeekOrigin::All) const override final { return _nonBuffered->IsSeekableI(origin); }

    virtual std::streamoff TellI() const override final;
    virtual std::streamoff SeekI(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final;

    virtual std::streamsize SizeInBytes() const override final { return _nonBuffered->SizeInBytes(); }

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t ReadSome(void* storage, size_t eltsize, size_t count) override final;

public: // IBufferedStreamReader
    virtual bool Peek(char& ch) override final;
    virtual bool Peek(wchar_t& wch) override final;

private:
    IStreamReader* _nonBuffered;
    u8* _buffer;
    std::streamoff _origin;
    u32 _offset;
    u32 _capacity;

    bool RefillBuffer_();
};
//----------------------------------------------------------------------------
class FBufferedStreamWriter : public IBufferedStreamWriter, FBufferedStreamAllocator {
public:
    FBufferedStreamWriter();
    explicit FBufferedStreamWriter(IStreamWriter* nonBuffered);
    ~FBufferedStreamWriter();

    FBufferedStreamWriter(const FBufferedStreamWriter& ) = delete;
    FBufferedStreamWriter& operator =(const FBufferedStreamWriter& ) = delete;

    inline friend void swap(FBufferedStreamWriter& lhs, FBufferedStreamWriter rhs) {
        std::swap(lhs._nonBuffered, rhs._nonBuffered);
        std::swap(lhs._buffer, rhs._buffer);
        std::swap(lhs._origin, rhs._origin);
        std::swap(lhs._offset, rhs._offset);
    }

public: // IStreamWriter
    virtual bool IsSeekableO(ESeekOrigin origin = ESeekOrigin::All) const override final { return _nonBuffered->IsSeekableO(origin); }

    virtual std::streamoff TellO() const override final;
    virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) override final;

public: // IBufferedStreamWriter
    virtual void Flush() override final;

private:
    IStreamWriter* _nonBuffered;
    u8* _buffer;
    std::streamoff _origin;
    size_t _offset;

    bool CommitBuffer_();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Lambda, typename _Return = void>
_Return UsingBufferedStream(IStreamReader* reader, _Lambda&& lambda) {
    Assert(reader);
    if (IBufferedStreamReader* bufferedIFP = reader->ToBufferedI()) {
        return lambda(bufferedIFP);
    }
    else {
        FBufferedStreamReader buffered(reader);
        return lambda(&buffered);
    }
}
//----------------------------------------------------------------------------
template <typename _Lambda, typename _Return = void>
_Return UsingBufferedStream(IStreamWriter* writer, _Lambda&& lambda) {
    Assert(writer);
    if (IBufferedStreamWriter* bufferedIFP = writer->ToBufferedO()) {
        return lambda(bufferedIFP);
    }
    else {
        FBufferedStreamWriter buffered(writer);
        return lambda(&buffered);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
