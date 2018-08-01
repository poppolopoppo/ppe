#pragma once

#include "Core.h"

namespace PPE {
class IStreamWriter;
template <typename T>
class TMemoryView;
template <typename T, typename _Allocator>
class TRawStorage;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Compression {
//----------------------------------------------------------------------------
enum ECompressMethod {
    Default = 0,
    Fast,
    HighCompression,
};
//----------------------------------------------------------------------------
size_t  CompressedSizeUpperBound(size_t sizeInBytes);
//----------------------------------------------------------------------------
size_t  CompressMemory(const TMemoryView<u8>& dst, const TMemoryView<const u8>& src, ECompressMethod method = Default);
//----------------------------------------------------------------------------
size_t  DecompressedSize(const TMemoryView<const u8>& src);
//----------------------------------------------------------------------------
bool    DecompressMemory(const TMemoryView<u8>& dst, const TMemoryView<const u8>& src);
//----------------------------------------------------------------------------
template <typename _Allocator>
size_t CompressMemory(TRawStorage<u8, _Allocator>& dst, const TMemoryView<const u8>& src, ECompressMethod method = Default) {
    const size_t maxSize = CompressedSizeUpperBound(src.SizeInBytes());
    dst.Resize_DiscardData(maxSize);
    Assert(dst.SizeInBytes());
    return CompressMemory(dst.MakeView(), src, method);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
bool DecompressMemory(TRawStorage<u8, _Allocator>& dst, const TMemoryView<const u8>& src) {
    const size_t origSize = DecompressedSize(src);
    dst.Resize_DiscardData(origSize);
    return DecompressMemory(dst.MakeView(), src);
}
//----------------------------------------------------------------------------
} //!Compression
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
