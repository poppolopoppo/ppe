#include "stdafx.h"

#include "Compression.h"

#include "HashFunctions.h"
#include "MemoryView.h"

#include "Diagnostic/Logger.h"
#include "Container/RawStorage.h"
#include "Misc/FourCC.h"

#include "External/lz4.h"
#include "External/lz4hc.h"

#define WITH_CORE_COMPRESSION_FINGERPRINT 1 //%_NOCOMMTI%

namespace Core {
namespace Compression {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static const FFourCC FILE_MAGIC_     ("LZ4B");
static const FFourCC FILE_VERSION_   ("1.00");
//----------------------------------------------------------------------------
struct FFileHeader_ {
    FFourCC  Magic;
    FFourCC  Version;
    u32     SizeInBytes;
    u32     Fingerpint;
};
STATIC_ASSERT(sizeof(FFileHeader_) == 16);
//----------------------------------------------------------------------------
static u32 StreamFingerprint_(const TMemoryView<const u8>& src) {
#if WITH_CORE_COMPRESSION_FINGERPRINT
    return Fingerprint32(src);
#else
    return 0xBAADF00Dul;
#endif
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t CompressedSizeUpperBound(size_t sizeInBytes) {
    return checked_cast<size_t>(::LZ4_compressBound(checked_cast<int>(sizeInBytes)))
        + sizeof(FFileHeader_);
}
//----------------------------------------------------------------------------
size_t CompressMemory(const TMemoryView<u8>& dst, const TMemoryView<const u8>& src, ECompressMethod method /* = Default */) {
    Assert(dst.Pointer());
    Assert(dst.size() >= CompressedSizeUpperBound(src.SizeInBytes()));

    FFileHeader_* const pheader = reinterpret_cast<FFileHeader_*>(dst.Pointer());
    pheader->Magic = FILE_MAGIC_;
    pheader->Version = FILE_VERSION_;
    pheader->SizeInBytes = checked_cast<u32>(src.SizeInBytes());
    pheader->Fingerpint = StreamFingerprint_(src);

    size_t compressedSizeInBytes = sizeof(FFileHeader_);
    if (src.empty())
        return compressedSizeInBytes;
    Assert(src.Pointer());

    const TMemoryView<u8> datas = dst.CutStartingAt(compressedSizeInBytes);

    STATIC_CONST_INTEGRAL(int, FastAccelerator,  7/* ~21% faster */);
    STATIC_CONST_INTEGRAL(int, CompressionLevel, 9/* LZ4 default */);

    switch (method)
    {
    case Core::Compression::Default:
        compressedSizeInBytes += checked_cast<int>(::LZ4_compress_default(
            (char*)src.Pointer(),
            (char*)datas.Pointer(),
            checked_cast<int>(src.SizeInBytes()),
            checked_cast<int>(datas.SizeInBytes()) ));
        break;

    case Core::Compression::Fast:
        compressedSizeInBytes += checked_cast<int>(::LZ4_compress_fast(
            (char*)src.Pointer(),
            (char*)datas.Pointer(),
            checked_cast<int>(src.SizeInBytes()),
            checked_cast<int>(datas.SizeInBytes()),
            FastAccelerator ));
        break;

    case Core::Compression::HighCompression:
        compressedSizeInBytes += checked_cast<int>(::LZ4_compress_HC(
            (char*)src.Pointer(),
            (char*)datas.Pointer(),
            checked_cast<int>(src.SizeInBytes()),
            checked_cast<int>(datas.SizeInBytes()),
            CompressionLevel ));
        break;

    default:
        AssertNotImplemented();
        break;
    }

    LOG(Info, L"[Compression] Compress ratio : {0} -> {1} = {2:f2}%",
        FSizeInBytes(src.SizeInBytes()), FSizeInBytes(compressedSizeInBytes), compressedSizeInBytes*100.0f/src.SizeInBytes() );

    return compressedSizeInBytes;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t DecompressedSize(const TMemoryView<const u8>& src) {
    if (src.SizeInBytes() < sizeof(FFileHeader_))
        return 0;

    const FFileHeader_* pheader = reinterpret_cast<const FFileHeader_*>(src.Pointer());
    return pheader->SizeInBytes;
}
//----------------------------------------------------------------------------
bool DecompressMemory(const TMemoryView<u8>& dst, const TMemoryView<const u8>& src) {
    Assert(dst.Pointer());
    Assert(dst.SizeInBytes() == DecompressedSize(src));

    const FFileHeader_* pheader = reinterpret_cast<const FFileHeader_*>(src.Pointer());
    if (nullptr == pheader ||
        FILE_MAGIC_ != pheader->Magic ||
        FILE_VERSION_ != pheader->Version )
        return false;

    const size_t dataSizeInBytes = checked_cast<size_t>(::LZ4_decompress_fast(
        (const char*)&pheader[1],
        (char*)dst.Pointer(),
        pheader->SizeInBytes ));

    const size_t compressedSizeInBytes = dataSizeInBytes + sizeof(FFileHeader_);
    Assert(compressedSizeInBytes <= src.SizeInBytes());

#if WITH_CORE_COMPRESSION_FINGERPRINT
    const u32 readFingerprint = StreamFingerprint_(dst);
    AssertRelease(readFingerprint == pheader->Fingerpint);
#endif

    LOG(Info, L"[Compression] Decompress ratio : {0} -> {1} = {2:f2}%",
        FSizeInBytes(compressedSizeInBytes), FSizeInBytes(pheader->SizeInBytes), compressedSizeInBytes*100.0f/dst.SizeInBytes() );

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Compression
} //!namespace Core
