#pragma once

#include "Core_fwd.h"

#include "Container/RawStorage_fwd.h"
#include "IO/Stream_fwd.h"
#include "Memory/MemoryView.h"

namespace PPE {
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
PPE_CORE_API size_t  CompressedSizeUpperBound(size_t sizeInBytes);
//----------------------------------------------------------------------------
PPE_CORE_API size_t  CompressMemory(const TMemoryView<u8>& dst, const TMemoryView<const u8>& src, ECompressMethod method = Default);
//----------------------------------------------------------------------------
PPE_CORE_API size_t  DecompressedSize(const TMemoryView<const u8>& src);
//----------------------------------------------------------------------------
PPE_CORE_API bool    DecompressMemory(const TMemoryView<u8>& dst, const TMemoryView<const u8>& src);
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
