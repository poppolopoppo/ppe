#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/Maths/Geometry/ScalarVector_fwd.h"
#include "Core/Maths/Transform/ScalarMatrix_fwd.h"
#include "Core/Meta/BitField.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ConstantFieldType {
    Unknown = -1,

    Bool = 0,

    Int,
    Int2,
    Int3,
    Int4,

    UInt,
    UInt2,
    UInt3,
    UInt4,

    Float,
    Float2,
    Float3,
    Float4,

    Float3x3,
    Float4x3,
    Float3x4,
    Float4x4,
};
//----------------------------------------------------------------------------
size_t ConstantFieldTypeSizeInBytes(ConstantFieldType value);
//----------------------------------------------------------------------------
const char *ConstantFieldTypeToCStr(ConstantFieldType value);
//----------------------------------------------------------------------------
struct ConstantField {
    typedef Meta::Bit<u16>::First<1>::type bitinuse_type;
    typedef Meta::Bit<u16>::After<bitinuse_type>::Field<4>::type bittype_type;
    typedef Meta::Bit<u16>::After<bittype_type>::Remain::type bitoffset_type;

    FORCE_INLINE bool InUse() const { return bitinuse_type::Get(_data); }
    FORCE_INLINE size_t Offset() const { return bitoffset_type::Get(_data); }
    FORCE_INLINE ConstantFieldType Type() const { return static_cast<ConstantFieldType>(bittype_type::Get(_data)); }

    u16 HashValue() const { return bitinuse_type::Set(_data, false); }

    void Reset(bool inUse, size_t offset, ConstantFieldType type) {
        _data = 0;
        bitinuse_type::InplaceSet(_data, inUse);
        bittype_type::InplaceSet(_data, static_cast<u16>(type));
        bitoffset_type::InplaceSet(_data, checked_cast<u16>(offset));
    }

    u16 _data;
};
STATIC_ASSERT(sizeof(ConstantField) == sizeof(u16));
//----------------------------------------------------------------------------
inline bool operator ==(const ConstantField& lhs, const ConstantField& rhs) {
    return lhs.HashValue() == rhs.HashValue();
}
//----------------------------------------------------------------------------
inline bool operator !=(const ConstantField& lhs, const ConstantField& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const ConstantField& field) {
    return field.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct ConstantFieldTraits {
    enum : bool { Enabled = false };
    static const ConstantFieldType Type = ConstantFieldType::Unknown;
};
//----------------------------------------------------------------------------
#define DEF_CONSTANTFIELDTRAITS(_T, _FIELDTYPE) \
    template <> struct ConstantFieldTraits<_T> { \
        enum : bool { Enabled = false }; \
        static const ConstantFieldType Type = ConstantFieldType::_FIELDTYPE; \
    }
//----------------------------------------------------------------------------
DEF_CONSTANTFIELDTRAITS(bool, Bool);
//----------------------------------------------------------------------------
DEF_CONSTANTFIELDTRAITS(i32, Int);
DEF_CONSTANTFIELDTRAITS(i322, Int2);
DEF_CONSTANTFIELDTRAITS(i323, Int3);
DEF_CONSTANTFIELDTRAITS(i324, Int4);
//----------------------------------------------------------------------------
DEF_CONSTANTFIELDTRAITS(u32, UInt);
DEF_CONSTANTFIELDTRAITS(u322, UInt2);
DEF_CONSTANTFIELDTRAITS(u323, UInt3);
DEF_CONSTANTFIELDTRAITS(u324, UInt4);
//----------------------------------------------------------------------------
DEF_CONSTANTFIELDTRAITS(float, Float);
DEF_CONSTANTFIELDTRAITS(float2, Float2);
DEF_CONSTANTFIELDTRAITS(float3, Float3);
DEF_CONSTANTFIELDTRAITS(float4, Float4);
//----------------------------------------------------------------------------
DEF_CONSTANTFIELDTRAITS(float3x3, Float3x3);
DEF_CONSTANTFIELDTRAITS(float4x3, Float4x3);
DEF_CONSTANTFIELDTRAITS(float3x4, Float3x4);
DEF_CONSTANTFIELDTRAITS(float4x4, Float4x4);
//----------------------------------------------------------------------------
#undef DEF_CONSTANTFIELDTRAITS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(_Prefix, _Tpl, _Args, T) \
    _Prefix template class _Tpl<T _Args >
//----------------------------------------------------------------------------
#define CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_SCALAR_(_Prefix, _Tpl, _Args) \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(_Prefix, _Tpl, _Args, bool); \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(_Prefix, _Tpl, _Args, i32); \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(_Prefix, _Tpl, _Args, u32); \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(_Prefix, _Tpl, _Args, float)
//----------------------------------------------------------------------------
#define CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_VECTOR_(_Prefix, _Tpl, _Args) \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(_Prefix, _Tpl, _Args, i322); \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(_Prefix, _Tpl, _Args, i323); \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(_Prefix, _Tpl, _Args, i324); \
    \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(_Prefix, _Tpl, _Args, u322); \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(_Prefix, _Tpl, _Args, u323); \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(_Prefix, _Tpl, _Args, u324); \
    \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(_Prefix, _Tpl, _Args, float2); \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(_Prefix, _Tpl, _Args, float3); \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(_Prefix, _Tpl, _Args, float4)
//----------------------------------------------------------------------------
#define CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_MATRIX_(_Prefix, _Tpl, _Args) \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(_Prefix, _Tpl, _Args, float3x3); \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(_Prefix, _Tpl, _Args, float4x3); \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(_Prefix, _Tpl, _Args, float3x4); \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(_Prefix, _Tpl, _Args, float4x4)
//----------------------------------------------------------------------------
#define CONSTANTFIELD_EXTERNALTEMPLATE_DECL(_Tpl, _Args) \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_SCALAR_(extern, _Tpl, _Args); \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_VECTOR_(extern, _Tpl, _Args); \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_MATRIX_(extern, _Tpl, _Args)
//----------------------------------------------------------------------------
#define CONSTANTFIELD_EXTERNALTEMPLATE_DEF(_Tpl, _Args) \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_SCALAR_(    , _Tpl, _Args); \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_VECTOR_(    , _Tpl, _Args); \
    CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_MATRIX_(    , _Tpl, _Args)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
