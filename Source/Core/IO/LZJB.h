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
namespace LZJB {
//----------------------------------------------------------------------------
void Compress(IStreamWriter* dst, const MemoryView<const u8>& src);
bool Decompress(RAWSTORAGE(Stream, u8)* dst, const MemoryView<const u8>& src);
bool Decompress(RAWSTORAGE_THREAD_LOCAL(Stream, u8)* dst, const MemoryView<const u8>& src);
//----------------------------------------------------------------------------
} //!LZJB
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core