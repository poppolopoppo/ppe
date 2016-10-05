#pragma once

#include "Core.Serialize/XML/XML.h"

#include "Core/Container/Vector.h"
#include "Core/IO/StringView.h"
#include "Core/Maths/ScalarMatrix_fwd.h"
#include "Core/Maths/ScalarVector_fwd.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
class FBitSet;
namespace XML {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using TArray = VECTOR_THREAD_LOCAL(XML, T);
//----------------------------------------------------------------------------
bool Parse(TArray<FStringView>& dst, const FStringView& str);
bool Parse(const TMemoryView<FStringView>& dst, const FStringView& str);
//----------------------------------------------------------------------------
bool Parse(bool* dst, const FStringView& str);
bool Parse(FBitSet& dst, const FStringView& str);
//----------------------------------------------------------------------------
bool Parse(i32* dst, const FStringView& str);
bool Parse(TArray<i32>& dst, const FStringView& str);
bool Parse(const TMemoryView<i32>& dst, const FStringView& str);
//----------------------------------------------------------------------------
bool Parse(float* dst, const FStringView& str);
bool Parse(TArray<float>& dst, const FStringView& str);
bool Parse(const TMemoryView<float>& dst, const FStringView& str);
//----------------------------------------------------------------------------
bool Parse(word2* dst, const FStringView& str);
bool Parse(float2* dst, const FStringView& str);
bool Parse(TArray<word2>& dst, const FStringView& str);
bool Parse(TArray<float2>& dst, const FStringView& str);
bool Parse(const TMemoryView<word2>& dst, const FStringView& str);
bool Parse(const TMemoryView<float2>& dst, const FStringView& str);
//----------------------------------------------------------------------------
bool Parse(word3* dst, const FStringView& str);
bool Parse(float3* dst, const FStringView& str);
bool Parse(TArray<word3>& dst, const FStringView& str);
bool Parse(TArray<float3>& dst, const FStringView& str);
bool Parse(const TMemoryView<word3>& dst, const FStringView& str);
bool Parse(const TMemoryView<float3>& dst, const FStringView& str);
//----------------------------------------------------------------------------
bool Parse(word4* dst, const FStringView& str);
bool Parse(float4* dst, const FStringView& str);
bool Parse(TArray<word4>& dst, const FStringView& str);
bool Parse(TArray<float4>& dst, const FStringView& str);
bool Parse(const TMemoryView<word4>& dst, const FStringView& str);
bool Parse(const TMemoryView<float4>& dst, const FStringView& str);
//----------------------------------------------------------------------------
bool Parse(float2x2* dst, const FStringView& str);
bool Parse(TArray<float2x2>& dst, const FStringView& str);
bool Parse(const TMemoryView<float2x2>& dst, const FStringView& str);
//----------------------------------------------------------------------------
bool Parse(float3x3* dst, const FStringView& str);
bool Parse(TArray<float3x3>& dst, const FStringView& str);
bool Parse(const TMemoryView<float3x3>& dst, const FStringView& str);
//----------------------------------------------------------------------------
bool Parse(float4x4* dst, const FStringView& str);
bool Parse(TArray<float4x4>& dst, const FStringView& str);
bool Parse(const TMemoryView<float4x4>& dst, const FStringView& str);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace XML
} //!namespace Core
