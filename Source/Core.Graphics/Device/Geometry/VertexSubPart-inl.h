#pragma once

#include "Core.Graphics/Device/Geometry/VertexSubPart.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline void VertexSubPartKey::Reset(VertexSubPartFormat format, VertexSubPartSemantic semantic, size_t index) {
    _data = 0;
    bitformat_type::InplaceSet(_data, static_cast<u32>(format));
    bitsemantic_type::InplaceSet(_data, static_cast<u32>(semantic));
    bitindex_type::InplaceSet(_data, checked_cast<u32>(index));

    Assert(Format() == format);
    Assert(Semantic() == semantic);
    Assert(Index() == index);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
const T& VertexSubPart<T>::TypedGet(const void *vertex) const {
    Assert(vertex);
    return reinterpret_cast<const T&>(*(reinterpret_cast<const u8 *>(vertex) + Offset()));
}
//----------------------------------------------------------------------------
template <typename T>
void VertexSubPart<T>::TypedSet(void *const vertex, const T& value) const {
    Assert(vertex);
    reinterpret_cast<T&>(*(reinterpret_cast<u8 *>(vertex) + Offset())) = value;
}
//----------------------------------------------------------------------------
template <typename T>
void VertexSubPart<T>::Get(const void *vertex, void *const value, size_t size) const {
    Assert(sizeof(T) == size);
    *reinterpret_cast<T *>(value) = TypedGet(vertex);
}
//----------------------------------------------------------------------------
template <typename T>
void VertexSubPart<T>::Set(void *const vertex, const void *value, size_t size) const {
    Assert(sizeof(T) == size);
    TypedSet(vertex, *reinterpret_cast<const T *>(value));
}
//----------------------------------------------------------------------------
template <typename T>
void VertexSubPart<T>::Copy(void *const dst, const void *src, size_t size) const {
    Assert(dst);
    Assert(src);
    Assert(Offset() + sizeof(T) <= size);
    TypedSet(dst, *reinterpret_cast<const T *>(reinterpret_cast<const u8 *>(src) + Offset()));
}
//----------------------------------------------------------------------------
template <typename T>
bool VertexSubPart<T>::Equals(const void *lhs, const void *rhs, size_t size) const {
    Assert(lhs);
    Assert(rhs);
    Assert(Offset() + sizeof(T) <= size);
    return (TypedGet(lhs) == TypedGet(rhs));
}
//----------------------------------------------------------------------------
template <typename T>
void VertexSubPart<T>::Clear(void *const vertex) const {
    TypedSet(vertex, NumericLimits<T>::Default );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_VERTEXSUBPARTFORMAT_TRAITS(_Format, _Type) \
    template <> \
    struct VertexSubPartFormatTraits<VertexSubPartFormat::_Format> { \
        typedef std::true_type enabled; \
        typedef _Type type; \
    }; \
    template <> \
    struct VertexSubPartFormatReverseTraits<_Type> { \
        typedef std::true_type enabled; \
        static constexpr VertexSubPartFormat Format = VertexSubPartFormat::_Format; \
    }
//----------------------------------------------------------------------------
DEF_VERTEXSUBPARTFORMAT_TRAITS(Float, float);
DEF_VERTEXSUBPARTFORMAT_TRAITS(Float2, float2);
DEF_VERTEXSUBPARTFORMAT_TRAITS(Float3, float3);
DEF_VERTEXSUBPARTFORMAT_TRAITS(Float4, float4);

DEF_VERTEXSUBPARTFORMAT_TRAITS(Byte, byte);
DEF_VERTEXSUBPARTFORMAT_TRAITS(Byte2, byte2);
DEF_VERTEXSUBPARTFORMAT_TRAITS(Byte4, byte4);

DEF_VERTEXSUBPARTFORMAT_TRAITS(UByte, ubyte);
DEF_VERTEXSUBPARTFORMAT_TRAITS(UByte2, ubyte2);
DEF_VERTEXSUBPARTFORMAT_TRAITS(UByte4, ubyte4);

DEF_VERTEXSUBPARTFORMAT_TRAITS(Short, i16);
DEF_VERTEXSUBPARTFORMAT_TRAITS(Short2, short2);
DEF_VERTEXSUBPARTFORMAT_TRAITS(Short4, short4);

DEF_VERTEXSUBPARTFORMAT_TRAITS(UShort, ushort);
DEF_VERTEXSUBPARTFORMAT_TRAITS(UShort2, ushort2);
DEF_VERTEXSUBPARTFORMAT_TRAITS(UShort4, ushort4);

DEF_VERTEXSUBPARTFORMAT_TRAITS(Word, word);
DEF_VERTEXSUBPARTFORMAT_TRAITS(Word2, word2);
DEF_VERTEXSUBPARTFORMAT_TRAITS(Word3, word3);
DEF_VERTEXSUBPARTFORMAT_TRAITS(Word4, word4);

DEF_VERTEXSUBPARTFORMAT_TRAITS(UWord, uword);
DEF_VERTEXSUBPARTFORMAT_TRAITS(UWord2, uword2);
DEF_VERTEXSUBPARTFORMAT_TRAITS(UWord3, uword3);
DEF_VERTEXSUBPARTFORMAT_TRAITS(UWord4, uword4);

DEF_VERTEXSUBPARTFORMAT_TRAITS(Half, half);
DEF_VERTEXSUBPARTFORMAT_TRAITS(Half2, half2);
DEF_VERTEXSUBPARTFORMAT_TRAITS(Half4, half4);

DEF_VERTEXSUBPARTFORMAT_TRAITS(Byte2N, byte2n);
DEF_VERTEXSUBPARTFORMAT_TRAITS(Byte4N, byte4n);
DEF_VERTEXSUBPARTFORMAT_TRAITS(UByte2N, ubyte2n);
DEF_VERTEXSUBPARTFORMAT_TRAITS(UByte4N, ubyte4n);

DEF_VERTEXSUBPARTFORMAT_TRAITS(Short2N, short2n);
DEF_VERTEXSUBPARTFORMAT_TRAITS(Short4N, short4n);
DEF_VERTEXSUBPARTFORMAT_TRAITS(UShort2N, ushort2n);
DEF_VERTEXSUBPARTFORMAT_TRAITS(UShort4N, ushort4n);

DEF_VERTEXSUBPARTFORMAT_TRAITS(UX10Y10Z10W2N, UX10Y10Z10W2N);
//----------------------------------------------------------------------------
#undef DEF_VERTEXSUBPARTFORMAT_TRAITS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
