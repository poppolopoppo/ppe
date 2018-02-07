#include "stdafx.h"

#include "Compression.h"

#include "HashFunctions.h"
#include "MemoryView.h"


#include "Container/RawStorage.h"
#include "IO/FormatHelpers.h"
#include "Misc/FourCC.h"

#include "Core.External/lz4-external.h"

#include "Time/TimedScope.h"
#if USE_CORE_BENCHMARK
#   include "Diagnostic/Logger.h"
#endif

#define WITH_CORE_COMPRESSION_FINGERPRINT 1 //%_NOCOMMTI%

namespace Core {
EXTERN_LOG_CATEGORY(CORE_API, Benchmark);
namespace Compression {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static const FFourCC FILE_MAGIC_     ("LZ4B");
static const FFourCC FILE_VERSION_   (STRINGIZE(LZ4_VERSION_MAJOR) ".01");
//----------------------------------------------------------------------------
struct FFileHeader_ {
    FFourCC     Magic;
    FFourCC     Version;
    u32         SizeInBytes;
    u32         Fingerpint;
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
#if USE_CORE_BENCHMARK
struct FCompressionBenchmark_ {
    const FTimepoint StartedAt;
    FCompressionBenchmark_() : StartedAt(FTimepoint::Now()) {}
    void Finished(const wchar_t* msg, size_t a, size_t b) {
        const FTimespan elapsed = FTimepoint::ElapsedSince(StartedAt);
        LOG(Benchmark, Info, L" {0:20} | {1:8} | {2:10} ===> {3:10} : {4:10f2}% = {5:10f2} Mb/s",
            msg,
            Fmt::DurationInMs(elapsed),
            Fmt::SizeInBytes(a),
            Fmt::SizeInBytes(b),
            b * 100.0 / a,
            FMegabytes(FBytes((double)Max(a, b))).Value() / FSeconds(elapsed).Value() );
    }
};
#endif //!USE_CORE_BENCHMARK
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

#if USE_CORE_BENCHMARK
    FCompressionBenchmark_ bm;
#endif

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
            7/* ~21% faster */));
        break;

    case Core::Compression::HighCompression:
        compressedSizeInBytes += checked_cast<int>(::LZ4_compress_HC(
            (char*)src.Pointer(),
            (char*)datas.Pointer(),
            checked_cast<int>(src.SizeInBytes()),
            checked_cast<int>(datas.SizeInBytes()),
            LZ4HC_CLEVEL_OPT_MIN/* minimum settings for better compression with HC */));
        break;

    default:
        AssertNotImplemented();
        break;
    }

#if USE_CORE_BENCHMARK
    bm.Finished(L"COMPRESS", src.SizeInBytes(), compressedSizeInBytes);
#endif

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

#if USE_CORE_BENCHMARK
    FCompressionBenchmark_ bm;
#endif

    const size_t dataSizeInBytes = checked_cast<size_t>(::LZ4_decompress_fast(
        (const char*)&pheader[1],
        (char*)dst.Pointer(),
        pheader->SizeInBytes ));

#if USE_CORE_BENCHMARK
    bm.Finished(L"DECOMPRESS", src.SizeInBytes(), pheader->SizeInBytes);
#endif

    const size_t compressedSizeInBytes = dataSizeInBytes + sizeof(FFileHeader_);
    Assert(compressedSizeInBytes <= src.SizeInBytes());

#if WITH_CORE_COMPRESSION_FINGERPRINT && defined(WITH_CORE_ASSERT_RELEASE)
    const u32 readFingerprint = StreamFingerprint_(dst);
    AssertRelease(readFingerprint == pheader->Fingerpint);
#endif

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Compression
} //!namespace Core
