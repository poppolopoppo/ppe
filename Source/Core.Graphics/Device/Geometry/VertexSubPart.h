#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Packing/PackedVectors.h"
#include "Core/Memory/AlignedStorage.h"
#include "Core/Meta/BitField.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class VertexSubPartSemantic {
    Position = 0,
    TexCoord,
    Color,
    Normal,
    Tangent,
    Binormal,
};
//----------------------------------------------------------------------------
StringSlice VertexSubPartSemanticToCStr(VertexSubPartSemantic semantic);
//----------------------------------------------------------------------------
enum class VertexSubPartFormat {
    Float = 0,
    Float2,
    Float3,
    Float4,

    Byte,
    Byte2,
    Byte4,

    UByte,
    UByte2,
    UByte4,

    Short,
    Short2,
    Short4,

    UShort,
    UShort2,
    UShort4,

    Word,
    Word2,
    Word3,
    Word4,

    UWord,
    UWord2,
    UWord3,
    UWord4,

    Half,
    Half2,
    Half4,

    Byte2N,
    Byte4N,
    UByte2N,
    UByte4N,

    Short2N,
    Short4N,
    UShort2N,
    UShort4N,

    UX10Y10Z10W2N,
};
//----------------------------------------------------------------------------
StringSlice VertexSubPartFormatToCStr(VertexSubPartFormat format);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <VertexSubPartFormat _Format>
struct VertexSubPartFormatTraits {
    typedef std::false_type enabled;
};
//----------------------------------------------------------------------------
template <typename T>
struct VertexSubPartFormatReverseTraits {
    typedef std::false_type enabled;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct VertexSubPartKey {
    typedef Meta::Bit<u32>::First<8>::type bitformat_type;
    typedef Meta::Bit<u32>::After<bitformat_type>::Field<8>::type bitsemantic_type;
    typedef Meta::Bit<u32>::After<bitsemantic_type>::Remain::type bitindex_type;

    FORCE_INLINE VertexSubPartFormat Format() const { return static_cast<VertexSubPartFormat>(bitformat_type::Get(_data)); }
    FORCE_INLINE VertexSubPartSemantic Semantic() const { return static_cast<VertexSubPartSemantic>(bitsemantic_type::Get(_data)); }
    FORCE_INLINE size_t Index() const { return bitindex_type::Get(_data); }

    void Reset(VertexSubPartFormat format, VertexSubPartSemantic semantic, size_t index);

    u32 _data;
};
//----------------------------------------------------------------------------
inline bool operator ==(const VertexSubPartKey& lhs, const VertexSubPartKey& rhs) {
    return lhs._data == rhs._data;
}
//----------------------------------------------------------------------------
inline bool operator !=(const VertexSubPartKey& lhs, const VertexSubPartKey& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class AbstractVertexSubPart {
protected:
    AbstractVertexSubPart(size_t offset)
        : _offset(checked_cast<u32>(offset)) {}

public:
    virtual ~AbstractVertexSubPart() {}

    FORCE_INLINE size_t Offset() const { return _offset; }

    virtual size_t SizeInBytes() const = 0;

    virtual void Get(const void *vertex, void *const value, size_t size) const = 0;
    virtual void Set(void *const vertex, const void *value, size_t size) const = 0;

    virtual void Copy(void *const dst, const void *src, size_t size) const = 0;
    virtual bool Equals(const void *lhs, const void *rhs, size_t size) const = 0;

    virtual void Clear(void *const vertex) const = 0;

    template <typename T>
    void Get(void *const vertex, T *const value) const {
        Get(vertex, value, sizeof(T));
    }

    template <typename T>
    void Set(void *const vertex, const T& value) const {
        Set(vertex, &value, sizeof(T));
    }

private:
    u32 _offset;
};
//----------------------------------------------------------------------------
typedef ALIGNED_STORAGE(sizeof(AbstractVertexSubPart), sizeof(size_t))
        VertexSubPartPOD;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class VertexSubPart : public AbstractVertexSubPart {
public:
    VertexSubPart(size_t offset) : AbstractVertexSubPart(offset) {}
    virtual ~VertexSubPart() {}

    FORCE_INLINE const T& TypedGet(const void *vertex) const;
    FORCE_INLINE void TypedSet(void *const vertex, const T& value) const;

    virtual size_t SizeInBytes() const override { return sizeof(T); }

    virtual void Get(const void *vertex, void *const value, size_t size) const override;
    virtual void Set(void *const vertex, const void *value, size_t size) const override;

    virtual void Copy(void *const dst, const void *src, size_t size) const override;
    virtual bool Equals(const void *lhs, const void *rhs, size_t size) const override;

    virtual void Clear(void *const vertex) const override;
};
//----------------------------------------------------------------------------
template <VertexSubPartFormat _Format>
using TypedVertexSubPart = VertexSubPart< typename VertexSubPartFormatTraits<_Format>::type >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core

#include "Core.Graphics/Device/Geometry/VertexSubPart-inl.h"
