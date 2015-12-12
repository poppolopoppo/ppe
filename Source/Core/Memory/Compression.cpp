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
static const FourCC FILE_MAGIC_     ("LZ4B");
static const FourCC FILE_VERSION_   ("1.00");
//----------------------------------------------------------------------------
struct FileHeader_ {
    FourCC  Magic;
    FourCC  Version;
    u32     Fingerpint;
    u32     SizeInBytes;
};
STATIC_ASSERT(sizeof(FileHeader_) == 16);
//----------------------------------------------------------------------------
u32 FileFingerprint_(const MemoryView<const u8>& src) {
#if WITH_CORE_COMPRESSION_FINGERPRINT
    return Fingerprint32(src);
#else
    return u32(-1);
#endif
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t CompressedMaxSize(size_t sizeInBytes) {
    return checked_cast<size_t>(::LZ4_compressBound(checked_cast<int>(sizeInBytes)))
        + sizeof(FileHeader_);
}
//----------------------------------------------------------------------------
size_t Compress(MemoryView<u8>& dst, const MemoryView<const u8>& src, Method method /* = Default */) {
    Assert(dst.Pointer());
    Assert(dst.size() >= CompressedMaxSize(src.SizeInBytes()));

    FileHeader_* const pheader = reinterpret_cast<FileHeader_*>(dst.Pointer());
    pheader->Magic = FILE_MAGIC_;
    pheader->Version = FILE_VERSION_;
    pheader->Fingerpint = FileFingerprint_(src);
    pheader->SizeInBytes = checked_cast<u32>(src.SizeInBytes());

    size_t compressedSizeInBytes = sizeof(FileHeader_);
    if (src.empty())
        return compressedSizeInBytes;
    Assert(src.Pointer());

    const MemoryView<u8> datas = dst.RemainingAfter(compressedSizeInBytes);

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
        STATIC_CONST_INTEGRAL(int, FastAccelerator, 7/* ~21% faster */);
        compressedSizeInBytes += checked_cast<int>(::LZ4_compress_fast(
            (char*)src.Pointer(),
            (char*)datas.Pointer(),
            checked_cast<int>(src.SizeInBytes()),
            checked_cast<int>(datas.SizeInBytes()),
            FastAccelerator ));
        break;

    case Core::Compression::HighCompression:
        STATIC_CONST_INTEGRAL(int, CompressionLevel, 0/* LZ4 default */);
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
        SizeInBytes(src.SizeInBytes()), SizeInBytes(compressedSizeInBytes), compressedSizeInBytes*100.0f/src.SizeInBytes() );

    return compressedSizeInBytes;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t DecompressedSize(const MemoryView<const u8>& src) {
    if (src.SizeInBytes() < sizeof(FileHeader_))
        return 0;

    const FileHeader_* pheader = reinterpret_cast<const FileHeader_*>(src.Pointer());
    return pheader->SizeInBytes;
}
//----------------------------------------------------------------------------
bool Decompress(MemoryView<u8>& dst, const MemoryView<const u8>& src) {
    Assert(dst.Pointer());
    Assert(dst.SizeInBytes() == DecompressedSize(src));

    const FileHeader_* pheader = reinterpret_cast<const FileHeader_*>(src.Pointer());
    if (nullptr == pheader ||
        FILE_MAGIC_ != pheader->Magic ||
        FILE_VERSION_ != pheader->Version )
        return false;

    const size_t compressedSizeInBytes = checked_cast<size_t>(::LZ4_decompress_fast(
        (const char*)&pheader[1],
        (char*)dst.Pointer(),
        pheader->SizeInBytes ));
    Assert(compressedSizeInBytes + sizeof(FileHeader_) <= src.SizeInBytes());

#if WITH_CORE_COMPRESSION_FINGERPRINT
    const u32 readFingerprint = FileFingerprint_(dst);
    AssertRelease(readFingerprint == pheader->Fingerpint);
#endif

    LOG(Info, L"[Compression] Decompress ratio : {0} -> {1} = {2:f2}%",
        SizeInBytes(compressedSizeInBytes + sizeof(FileHeader_)), SizeInBytes(pheader->SizeInBytes), compressedSizeInBytes*100.0f/src.SizeInBytes() );

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Compression
} //!namespace Core