#pragma once

#include "Core.h"

#include "Allocator/Allocation.h"
#include "IO/StreamProvider.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FBufferedStreamAllocator = ALLOCATOR(Stream);
//----------------------------------------------------------------------------
// https://docs.microsoft.com/en-us/previous-versions/windows/it-pro/windows-2000-server/cc938632(v=technet.10)?redirectedfrom=MSDN
constexpr size_t GBufferedStreamDefaultBufferSize = 64_KiB;
//----------------------------------------------------------------------------
class PPE_CORE_API FBufferedStreamReader : public IBufferedStreamReader, FBufferedStreamAllocator {
public:
    FBufferedStreamReader();
    virtual ~FBufferedStreamReader() override;

    explicit FBufferedStreamReader(IStreamReader* nonBuffered, size_t bufferSize = GBufferedStreamDefaultBufferSize);

    FBufferedStreamReader(const FBufferedStreamReader& ) = delete;
    FBufferedStreamReader& operator =(const FBufferedStreamReader& ) = delete;

    size_t BufferSize() const { return _bufferSize; }

    void ForceAllocateInnerBuffer(); // use to disable lazy buffer allocation
    void SetStream(IStreamReader* nonBuffered);

    inline friend void swap(FBufferedStreamReader& lhs, FBufferedStreamReader& rhs) NOEXCEPT {
        std::swap(lhs._nonBuffered, rhs._nonBuffered);
        std::swap(lhs._buffer, rhs._buffer);
        std::swap(lhs._origin, rhs._origin);
        std::swap(lhs._offset, rhs._offset);
        std::swap(lhs._capacity, rhs._capacity);
    }

public: // IStreamReader
    virtual bool Eof() const NOEXCEPT override final;

    virtual bool IsSeekableI(ESeekOrigin origin = ESeekOrigin::All) const NOEXCEPT override final { return _nonBuffered->IsSeekableI(origin); }

    virtual std::streamoff TellI() const NOEXCEPT override final;
    virtual std::streamoff SeekI(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final;

    virtual std::streamsize SizeInBytes() const NOEXCEPT override final { return _nonBuffered->SizeInBytes(); }

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t ReadSome(void* storage, size_t eltsize, size_t count) override final;

public: // IBufferedStreamReader
    virtual bool Peek(char& ch) override final;
    virtual bool Peek(wchar_t& wch) override final;

    virtual bool ReadAt_SkipBuffer(const FRawMemory& storage, std::streamoff absolute) override final;

private:
    IStreamReader* _nonBuffered{ nullptr };
    u8* _buffer{ nullptr };
    std::streamoff _origin{ 0 };
    u32 _offset{ 0 };
    u32 _capacity{ 0 };
    size_t _bufferSize{ 0 };

    bool RefillBuffer_();
};
//----------------------------------------------------------------------------
class PPE_CORE_API FBufferedStreamWriter : public IBufferedStreamWriter, FBufferedStreamAllocator {
public:
    FBufferedStreamWriter();
    virtual ~FBufferedStreamWriter() override;

    explicit FBufferedStreamWriter(IStreamWriter* nonBuffered, size_t bufferSize = GBufferedStreamDefaultBufferSize);

    FBufferedStreamWriter(const FBufferedStreamWriter& ) = delete;
    FBufferedStreamWriter& operator =(const FBufferedStreamWriter& ) = delete;

    size_t BufferSize() const { return _bufferSize; }

    void ForceAllocateInnerBuffer(); // use to disable lazy buffer allocation
    void SetStream(IStreamWriter* nonBuffered);
    void ResetStream();

    inline friend void swap(FBufferedStreamWriter& lhs, FBufferedStreamWriter& rhs) NOEXCEPT {
        std::swap(lhs._nonBuffered, rhs._nonBuffered);
        std::swap(lhs._buffer, rhs._buffer);
        std::swap(lhs._origin, rhs._origin);
        std::swap(lhs._offset, rhs._offset);
    }

public: // IStreamWriter
    virtual bool IsSeekableO(ESeekOrigin origin = ESeekOrigin::All) const NOEXCEPT override final { return _nonBuffered->IsSeekableO(origin); }

    virtual std::streamoff TellO() const NOEXCEPT override final;
    virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) override final;

public: // IBufferedStreamWriter
    using IBufferedStreamWriter::read_f;
    virtual size_t StreamCopy(const read_f& read, size_t blockSz) override final;

    virtual void Flush() override final;

private:
    IStreamWriter* _nonBuffered{ nullptr };
    u8* _buffer{ nullptr };
    std::streamoff _origin{ 0 };
    size_t _offset{ 0 };
    size_t _bufferSize{ 0 };

    bool CommitBuffer_();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Lambda, typename _Return = decltype(std::declval<const _Lambda&>()(std::declval<IBufferedStreamReader*>()))>
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
template <typename _Lambda, typename _Return = decltype(std::declval<const _Lambda&>()(std::declval<IBufferedStreamWriter*>()))>
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
} //!namespace PPE
