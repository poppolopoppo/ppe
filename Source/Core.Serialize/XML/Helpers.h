#pragma once

#include "Core.Serialize/XML/XML.h"

#include "Core/Container/Vector.h"
#include "Core/IO/StringView.h"
#include "Core/Maths/ScalarMatrix_fwd.h"
#include "Core/Maths/ScalarVector_fwd.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
class BitSet;
namespace XML {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using Array = VECTOR_THREAD_LOCAL(XML, T);
//----------------------------------------------------------------------------
bool Parse(Array<StringView>& dst, const StringView& str);
bool Parse(const MemoryView<StringView>& dst, const StringView& str);
//----------------------------------------------------------------------------
bool Parse(bool* dst, const StringView& str);
bool Parse(BitSet& dst, const StringView& str);
//----------------------------------------------------------------------------
bool Parse(i32* dst, const StringView& str);
bool Parse(Array<i32>& dst, const StringView& str);
bool Parse(const MemoryView<i32>& dst, const StringView& str);
//----------------------------------------------------------------------------
bool Parse(float* dst, const StringView& str);
bool Parse(Array<float>& dst, const StringView& str);
bool Parse(const MemoryView<float>& dst, const StringView& str);
//----------------------------------------------------------------------------
bool Parse(word2* dst, const StringView& str);
bool Parse(float2* dst, const StringView& str);
bool Parse(Array<word2>& dst, const StringView& str);
bool Parse(Array<float2>& dst, const StringView& str);
bool Parse(const MemoryView<word2>& dst, const StringView& str);
bool Parse(const MemoryView<float2>& dst, const StringView& str);
//----------------------------------------------------------------------------
bool Parse(word3* dst, const StringView& str);
bool Parse(float3* dst, const StringView& str);
bool Parse(Array<word3>& dst, const StringView& str);
bool Parse(Array<float3>& dst, const StringView& str);
bool Parse(const MemoryView<word3>& dst, const StringView& str);
bool Parse(const MemoryView<float3>& dst, const StringView& str);
//----------------------------------------------------------------------------
bool Parse(word4* dst, const StringView& str);
bool Parse(float4* dst, const StringView& str);
bool Parse(Array<word4>& dst, const StringView& str);
bool Parse(Array<float4>& dst, const StringView& str);
bool Parse(const MemoryView<word4>& dst, const StringView& str);
bool Parse(const MemoryView<float4>& dst, const StringView& str);
//----------------------------------------------------------------------------
bool Parse(float2x2* dst, const StringView& str);
bool Parse(Array<float2x2>& dst, const StringView& str);
bool Parse(const MemoryView<float2x2>& dst, const StringView& str);
//----------------------------------------------------------------------------
bool Parse(float3x3* dst, const StringView& str);
bool Parse(Array<float3x3>& dst, const StringView& str);
bool Parse(const MemoryView<float3x3>& dst, const StringView& str);
//----------------------------------------------------------------------------
bool Parse(float4x4* dst, const StringView& str);
bool Parse(Array<float4x4>& dst, const StringView& str);
bool Parse(const MemoryView<float4x4>& dst, const StringView& str);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace XML
} //!namespace Core
