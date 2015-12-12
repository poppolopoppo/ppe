#pragma once

#include "Core/Core.h"

#include "Core/Container/RawStorage.h"

namespace Core {
class IStreamWriter;
template <typename T>
class MemoryView;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Compression {
//----------------------------------------------------------------------------
enum Method {
    Default = 0,
    Fast,
    HighCompression,
};
//----------------------------------------------------------------------------
size_t CompressedMaxSize(size_t sizeInBytes);
//----------------------------------------------------------------------------
size_t Compress(MemoryView<u8>& dst, const MemoryView<const u8>& src, Method method = Default);
//----------------------------------------------------------------------------
template <typename _Allocator>
size_t Compress(RawStorage<u8, _Allocator>& dst, const MemoryView<const u8>& src, Method method = Default) {
    const size_t maxSize = CompressedMaxSize(src.SizeInBytes());
    dst.Resize_DiscardData(maxSize);
    Assert(dst.SizeInBytes());
    return Compress(dst.MakeView(), src, method);
}
//----------------------------------------------------------------------------
} //!Compression
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Compression {
//----------------------------------------------------------------------------
size_t DecompressedSize(const MemoryView<const u8>& src);
//----------------------------------------------------------------------------
bool Decompress(MemoryView<u8>& dst, const MemoryView<const u8>& src);
//----------------------------------------------------------------------------
template <typename _Allocator>
bool Decompress(RawStorage<u8, _Allocator>& dst, const MemoryView<const u8>& src) {
    const size_t origSize = DecompressedSize(src);
    dst.Resize_DiscardData(origSize);
    return Decompress(dst.MakeView(), src);
}
//----------------------------------------------------------------------------
} //!Compression
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
