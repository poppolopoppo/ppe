// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "IO/CompressedStream.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformMemory.h"

#include "lz4-external.h"

namespace PPE {
LOG_CATEGORY(, LZ4_stream)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void PrepareLz4StreamBuffers_(size_t maxBlockSize,
    RAWSTORAGE(Compress, char)* compressBuffer,
    RAWSTORAGE(Compress, char)* ringBuffer) {
    /*! LZ4_compressBound() :
    *    Provides the maximum size that LZ4 compression may output in a "worst case" scenario (input data not compressible)
    *    Note that LZ4_compress_default() compresses faster when dstCapacity is >= LZ4_compressBound(srcSize)
    *  ==> hence use of LZ4_compressBound() to size compress buffer bellow
    */
    size_t compressSafeBounds = checked_cast<size_t>(
        LZ4_compressBound(checked_cast<int>(maxBlockSize)));
    compressSafeBounds = malloc_snap_size(compressSafeBounds);
    compressBuffer->Resize_DiscardData(compressSafeBounds);

    /* From LZ4.h:
    *  - Decompression buffer is larger than encoding buffer, by a minimum of maxBlockSize more bytes.
    *    In which case, encoding and decoding buffers do not need to be synchronized,
    *    and encoding ring buffer can have any size, including small ones ( < 64 KB).
    *  ==> hence use of `2*maxBlockSize` instead of `1*maxBlockSize` bellow
    */
    const int safeBounds = LZ4_decoderRingBufferSize(checked_cast<int>(maxBlockSize*2/* see comment above */));
    const size_t ringBufferSafeBounds = malloc_snap_size(checked_cast<size_t>(safeBounds)); // round to allocator boundary
    ringBuffer->Resize_DiscardData(ringBufferSafeBounds);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FCompressedStreamReader::FCompressedStreamReader(IStreamReader* compressedStream, size_t maxBlockSize)
:   _compressedStream(compressedStream)
,   _compressorContext(::LZ4_createStreamDecode())
,   _maxBlockSize(maxBlockSize) {
    Assert(_compressedStream);
    PrepareLz4StreamBuffers_(_maxBlockSize, &_compressBuffer, &_ringBuffer);
}
//----------------------------------------------------------------------------
FCompressedStreamReader::~FCompressedStreamReader() {
    LZ4_freeStreamDecode(static_cast<LZ4_streamDecode_t*>(_compressorContext));
}
//----------------------------------------------------------------------------
bool FCompressedStreamReader::Eof() const noexcept {
    if (_ringBufferReadOffset == _ringBufferWriteOffset)
        return _compressedStream->Eof();
    return false;
}
//----------------------------------------------------------------------------
bool FCompressedStreamReader::Read(void* storage, std::streamsize sizeInBytesL) {
    const size_t sizeInBytes = checked_cast<size_t>(sizeInBytesL);
    return (ReadSome(storage, 1, sizeInBytes) == sizeInBytes);
}
//----------------------------------------------------------------------------
size_t FCompressedStreamReader::ReadSome(void* storage, size_t eltsize, size_t count) {
    const size_t sizeInBytes = (eltsize * count);
    if (sizeInBytes == 0)
        return 0;

    Assert(sizeInBytes <= _maxBlockSize);

    size_t totalRead = 0;
    while (totalRead < sizeInBytes) {
        Assert(_ringBufferReadOffset <= _ringBufferWriteOffset);
        if (_ringBufferReadOffset == _ringBufferWriteOffset && not ReadNextCompressionBlock_())
            break;
        Assert(_ringBufferReadOffset < _ringBufferWriteOffset);

        const FRawMemory dstView = MakeRawView(storage, sizeInBytes).SubRange(totalRead,
            Min(_ringBufferWriteOffset - _ringBufferReadOffset, sizeInBytes - totalRead));
        const TMemoryView<const char> srcView = _ringBuffer.MakeConstView().SubRange(_ringBufferReadOffset,
            dstView.SizeInBytes());

        FPlatformMemory::Memcpy(dstView.data(), srcView.data(), dstView.SizeInBytes());
        totalRead += dstView.SizeInBytes();

        _ringBufferReadOffset += dstView.SizeInBytes();
        if (_ringBufferReadOffset == _ringBufferWriteOffset &&
            _ringBufferReadOffset >= _ringBuffer.SizeInBytes() - _maxBlockSize) {

            // wrap around input ring-buffer: documented as detected by LZ4 :p
            _ringBufferOrigin += checked_cast<std::streamsize>(_ringBufferWriteOffset);
            _ringBufferReadOffset = 0;
            _ringBufferWriteOffset = 0;
        }
    }

    return (totalRead);
}
//----------------------------------------------------------------------------
bool FCompressedStreamReader::ReadNextCompressionBlock_() {
    // read next block size
    u64 blockSize;
    PPE_LOG_CHECK(LZ4_stream, _compressedStream->Read(&blockSize, sizeof(blockSize)));
    PPE_LOG_CHECK(LZ4_stream, blockSize <= _maxBlockSize);

    // read next block compressed data
    PPE_LOG_CHECK(LZ4_stream, _compressedStream->Read(_compressBuffer.data(), checked_cast<std::streamsize>(blockSize)));

    // prepare space in decompressed ring buffer
    const TMemoryView<char> decompressView = _ringBuffer.MakeView().CutStartingAt(_ringBufferWriteOffset);
    Assert_NoAssume(decompressView.SizeInBytes() >= _maxBlockSize);

    // decode compressed data
    const int decompressedBytes = LZ4_decompress_safe_continue(
        static_cast<LZ4_streamDecode_t*>(_compressorContext),
        _compressBuffer.data(),
        decompressView.data(),
        checked_cast<int>(blockSize),
        checked_cast<int>(decompressView.SizeInBytes()));
    Assert(checked_cast<size_t>(decompressedBytes) <= _maxBlockSize);
    PPE_LOG_CHECK(LZ4_stream, Unused("LZ4_decompress_safe_continue"), decompressedBytes > 0);

    // bump ring buffer offset, only wrapping around when read is ready
    _ringBufferWriteOffset += checked_cast<size_t>(decompressedBytes);
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FCompressedStreamWriter::FCompressedStreamWriter(
    IStreamWriter* compressedStream,
    size_t maxBlockSize,
    Compression::ECompressMethod method)
:   _compressedStream(compressedStream)
,   _compressorContext(::LZ4_createStreamHC())
,   _maxBlockSize(maxBlockSize) {
    Assert(_compressedStream);
    PrepareLz4StreamBuffers_(_maxBlockSize, &_compressBuffer, &_ringBuffer);

    switch (method) {
    case Compression::Default:
        break;
    case Compression::Fast:
        LZ4_resetStreamHC_fast(static_cast<LZ4_streamHC_t*>(_compressorContext), LZ4HC_CLEVEL_MIN);
        break;
    case Compression::HighCompression:
        LZ4_resetStreamHC_fast(static_cast<LZ4_streamHC_t*>(_compressorContext), LZ4HC_CLEVEL_OPT_MIN);
        break;
    }
}
//----------------------------------------------------------------------------
FCompressedStreamWriter::~FCompressedStreamWriter() {
    // check if an incomplete block is still waiting for compression
    WriteNextCompressionBlock_();
    LZ4_freeStreamHC(static_cast<LZ4_streamHC_t*>(_compressorContext));
}
//----------------------------------------------------------------------------
bool FCompressedStreamWriter::Write(const void* storage, std::streamsize sizeInBytesL) {
    const size_t sizeInBytes = checked_cast<size_t>(sizeInBytesL);
    Assert_NoAssume(sizeInBytes <= _maxBlockSize);
    Assert_NoAssume(_ringBufferWriteOffset + sizeInBytes <= _ringBuffer.SizeInBytes());
    if (0 == sizeInBytes)
        return true;

    if (_ringBufferWriteOffset + sizeInBytes - _ringBufferReadOffset >= _maxBlockSize) {
        if (not WriteNextCompressionBlock_())
            return false;
    }

    char* const uncompressedPtr = &_ringBuffer[_ringBufferWriteOffset];
    FPlatformMemory::Memcpy(uncompressedPtr, storage, sizeInBytes);
    _ringBufferWriteOffset += sizeInBytes;

    if (_ringBufferWriteOffset >= _ringBuffer.SizeInBytes() - _maxBlockSize) {
        if (not WriteNextCompressionBlock_())
            return false;

        AssertRelease(_ringBufferReadOffset == _ringBufferWriteOffset);
        // wrap around input ring-buffer: documented as detected by LZ4 :p
        _ringBufferOrigin += checked_cast<std::streamsize>(_ringBufferWriteOffset);
        _ringBufferReadOffset = 0;
        _ringBufferWriteOffset = 0;
    }
    return true;
}
//----------------------------------------------------------------------------
size_t FCompressedStreamWriter::WriteSome(const void* storage, size_t eltsize, size_t count) {
    const size_t sizeInBytes = Min(eltsize * count, _maxBlockSize);
    if (Write(storage, checked_cast<std::streamsize>(sizeInBytes)))
        return count;
    return 0;
}
//----------------------------------------------------------------------------
bool FCompressedStreamWriter::WriteNextCompressionBlock_() {
    if (_ringBufferReadOffset == _ringBufferWriteOffset)
        return true;

    Assert(_ringBufferReadOffset < _ringBufferWriteOffset);
    const TMemoryView<const char> sourceView = _ringBuffer.MakeConstView()
        .SubRange(_ringBufferReadOffset, _ringBufferWriteOffset - _ringBufferReadOffset);
    _ringBufferReadOffset = _ringBufferWriteOffset;

    AssertRelease_NoAssume(sourceView.SizeInBytes() <= _maxBlockSize);

    TMemoryView<char> compressView = _compressBuffer.MakeView();
    compressView = compressView.CutStartingAt(sizeof(u64)); // reserve size for block size

    // compress the block using LZ4_HC
    const int compressedBytes = LZ4_compress_HC_continue(
        static_cast<LZ4_streamHC_t*>(_compressorContext),
        sourceView.data(),
        compressView.data(),
        checked_cast<int>(sourceView.SizeInBytes()),
        checked_cast<int>(compressView.SizeInBytes()));
    Assert(checked_cast<size_t>(compressedBytes) <= _maxBlockSize);
    PPE_LOG_CHECK(LZ4_stream, Unused("LZ4_compress_HC_continue"), compressedBytes > 0);

    // record block size before compressed data: compressed block must be decoded in whole
    *reinterpret_cast<u64*>(_compressBuffer.data()) = checked_cast<u64>(compressedBytes);

    // finally write compressed block to underlying stream
    PPE_LOG_CHECK(LZ4_stream, _compressedStream->Write(_compressBuffer.data(),
        compressedBytes + static_cast<std::streamsize>(sizeof(u64))/* block size */));
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
