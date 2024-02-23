#pragma once

#include "Core_fwd.h"

#include "Container/RawStorage_fwd.h"
#include "Memory/SharedBuffer_fwd.h"
#include "Memory/MemoryView.h"
#include "Meta/Optional.h"

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
NODISCARD PPE_CORE_API size_t  CompressedSizeUpperBound(size_t sizeInBytes) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API size_t  CompressMemory(const TMemoryView<u8>& dst, const TMemoryView<const u8>& src, ECompressMethod method = Default);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API size_t  DecompressedSize(const TMemoryView<const u8>& src);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool    DecompressMemory(const TMemoryView<u8>& dst, const TMemoryView<const u8>& src);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FUniqueBuffer DecompressBuffer(const FSharedBuffer& src);
//----------------------------------------------------------------------------
template <typename _Allocator>
bool CompressMemory(TRawStorage<u8, _Allocator>& dst, const TMemoryView<const u8>& src, ECompressMethod method = Default) {
    const size_t maxSize = CompressedSizeUpperBound(src.SizeInBytes());
    dst.Resize_DiscardData(maxSize);
    Assert(dst.SizeInBytes());
    const size_t actualSize = CompressMemory(dst.MakeView(), src, method);
    if (actualSize > 0) {
        dst.Resize_KeepData(actualSize);
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
NODISCARD bool DecompressMemory(TRawStorage<u8, _Allocator>& dst, const TMemoryView<const u8>& src) {
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
