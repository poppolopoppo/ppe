#pragma once

#include "Core/Core.h"

namespace Core {
class IStreamWriter;
template <typename T>
class MemoryView;
template <typename T, typename _Allocator>
class RawStorage;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Compression {
//----------------------------------------------------------------------------
enum CompressMethod {
    Default = 0,
    Fast,
    HighCompression,
};
//----------------------------------------------------------------------------
size_t  CompressedSizeUpperBound(size_t sizeInBytes);
//----------------------------------------------------------------------------
size_t  CompressMemory(MemoryView<u8>& dst, const MemoryView<const u8>& src, CompressMethod method = Default);
//----------------------------------------------------------------------------
size_t  DecompressedSize(const MemoryView<const u8>& src);
//----------------------------------------------------------------------------
bool    DecompressMemory(MemoryView<u8>& dst, const MemoryView<const u8>& src);
//----------------------------------------------------------------------------
template <typename _Allocator>
size_t CompressMemory(RawStorage<u8, _Allocator>& dst, const MemoryView<const u8>& src, CompressMethod method = Default) {
    const size_t maxSize = CompressedSizeUpperBound(src.SizeInBytes());
    dst.Resize_DiscardData(maxSize);
    Assert(dst.SizeInBytes());
    return CompressMemory(dst.MakeView(), src, method);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
bool DecompressMemory(RawStorage<u8, _Allocator>& dst, const MemoryView<const u8>& src) {
    const size_t origSize = DecompressedSize(src);
    dst.Resize_DiscardData(origSize);
    return DecompressMemory(dst.MakeView(), src);
}
//----------------------------------------------------------------------------
} //!Compression
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
