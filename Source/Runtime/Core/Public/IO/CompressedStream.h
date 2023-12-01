#pragma once

#include "Core_fwd.h"

#include "Container/RawStorage.h"
#include "Container/RingBuffer.h"
#include "IO/StreamProvider.h"
#include "Memory/Compression.h"
#include "Memory/MemoryStream.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FCompressedStreamReader final : public IStreamReader, Meta::FNonCopyableNorMovable {
public:
    explicit FCompressedStreamReader(
        IStreamReader* compressedStream,
        size_t maxBlockSize = ALLOCATION_BOUNDARY);
    virtual ~FCompressedStreamReader() override;

    size_t MaxBlockSize() const { return _maxBlockSize; }

    inline friend void swap(FCompressedStreamReader& lhs, FCompressedStreamReader& rhs) NOEXCEPT = delete;

public: // IStreamReader
    virtual bool Eof() const NOEXCEPT override final;

    virtual bool IsSeekableI(ESeekOrigin origin = ESeekOrigin::All) const NOEXCEPT override final {
        Unused(origin);
        return false;
    }

    virtual std::streamoff TellI() const NOEXCEPT override final {
        return (_ringBufferOrigin + checked_cast<std::streamsize>(_ringBufferReadOffset));
    }
    virtual std::streamoff SeekI(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final {
        Unused(offset, origin);
        AssertNotImplemented();
    }

    virtual std::streamsize SizeInBytes() const NOEXCEPT override final { return _compressedStream->SizeInBytes(); }

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t ReadSome(void* storage, size_t eltsize, size_t count) override final;

private:
    bool ReadNextCompressionBlock_();

    RAWSTORAGE(Compress, char) _compressBuffer;
    RAWSTORAGE(Compress, char) _ringBuffer;

    std::streamoff _ringBufferOrigin{ 0 };

    IStreamReader* const _compressedStream{ nullptr };
    void* const _compressorContext{ nullptr };
    size_t const _maxBlockSize{ 0 };

    size_t _ringBufferReadOffset{ 0 };
    size_t _ringBufferWriteOffset{ 0 };
};
//----------------------------------------------------------------------------
class PPE_CORE_API FCompressedStreamWriter final : public IStreamWriter, Meta::FNonCopyableNorMovable {
public:
    explicit FCompressedStreamWriter(
        IStreamWriter* compressedStream,
        size_t maxBlockSize = ALLOCATION_BOUNDARY,
        Compression::ECompressMethod method = Compression::Default);
    virtual ~FCompressedStreamWriter() override;

    size_t MaxBlockSize() const { return _maxBlockSize; }

    inline friend void swap(FCompressedStreamWriter& lhs, FCompressedStreamWriter& rhs) NOEXCEPT = delete;

public: // IStreamWriter
    virtual bool IsSeekableO(ESeekOrigin origin = ESeekOrigin::All) const NOEXCEPT override final {
        Unused(origin);
        return false;
    }

    virtual std::streamoff TellO() const NOEXCEPT override final {
        return (_ringBufferOrigin + checked_cast<std::streamsize>(_ringBufferWriteOffset));
    }
    virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final {
        Unused(offset, origin);
        AssertNotImplemented();
    }

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) override final;

private:
    bool WriteNextCompressionBlock_();

    RAWSTORAGE(Compress, char) _compressBuffer;
    RAWSTORAGE(Compress, char) _ringBuffer;

    std::streamoff _ringBufferOrigin{ 0 };

    IStreamWriter* const _compressedStream{ nullptr };
    void* const _compressorContext{ nullptr };
    size_t const _maxBlockSize{ 0 };

    size_t _ringBufferReadOffset{ 0 };
    size_t _ringBufferWriteOffset{ 0 };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Lambda, decltype(std::declval<_Lambda&&>()(std::declval<FCompressedStreamReader*>()))* = nullptr>
auto UsingCompressedStream(IStreamReader* reader, _Lambda&& lambda) {
    Assert(reader);
    FCompressedStreamReader compressed(reader);
    return lambda(&compressed);
}
//----------------------------------------------------------------------------
template <typename _Lambda, decltype(std::declval<_Lambda&&>()(std::declval<FCompressedStreamWriter*>()))* = nullptr>
auto UsingCompressedStream(IStreamWriter* writer, _Lambda&& lambda) {
    Assert(writer);
    FCompressedStreamWriter compressed(writer);
    return lambda(&compressed);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
