#pragma once

#include "Core/Core.h"

#include "Core/Container/RawStorage.h"

namespace Core {
class IBufferedStreamWriter;
template <typename T>
class TMemoryView;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace LZJB {
//----------------------------------------------------------------------------
void CompressMemory(IBufferedStreamWriter* dst, const TMemoryView<const u8>& src);
bool DecompressMemory(RAWSTORAGE(Stream, u8)* dst, const TMemoryView<const u8>& src);
bool DecompressMemory(RAWSTORAGE_THREAD_LOCAL(Stream, u8)* dst, const TMemoryView<const u8>& src);
//----------------------------------------------------------------------------
} //!LZJB
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
