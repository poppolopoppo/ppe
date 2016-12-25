#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Name.h"

#include "Core/IO/StringView.h"
#include "Core/Maths/ScalarMatrix.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/PackedVectors.h"
#include "Core/Memory/MemoryView.h"

#include "Core.Graphics/Value.Definitions-inl.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EValueType : u32 {
    Void = 0,

#define VALUETYPE_ENUM_DEF(_Name, T, _TypeId, _Unused) _Name = (_TypeId),
FOREACH_CORE_GRAPHIC_VALUETYPE(VALUETYPE_ENUM_DEF)
#undef VALUETYPE_ENUM_DEF
};
//----------------------------------------------------------------------------
FStringView ValueTypeToCStr(EValueType value);
size_t ValueSizeInBytes(EValueType value);
//----------------------------------------------------------------------------
void ValueCopy(EValueType type, const TMemoryView<u8>& dst, const TMemoryView<const u8>& src);

void ValueDefault(EValueType type, const TMemoryView<u8>& dst);

bool ValueEquals(EValueType type, const TMemoryView<const u8>& lhs, const TMemoryView<const u8>& rhs);

hash_t ValueHash(EValueType type, const TMemoryView<const u8>& data);

void ValueLerp( EValueType type,
                const TMemoryView<u8>& dst,
                const TMemoryView<const u8>& a,
                const TMemoryView<const u8>& b,
                float t );
void ValueLerpArray(EValueType type,
                    const TMemoryView<u8>& dst, size_t dstStride,
                    const TMemoryView<const u8>& a,
                    const TMemoryView<const u8>& b,
                    const TMemoryView<const float>& ts );

void ValueBarycentricLerp(  EValueType type, const TMemoryView<u8>& dst,
                            const TMemoryView<const u8>& a,
                            const TMemoryView<const u8>& b,
                            const TMemoryView<const u8>& c,
                            const float3& uvw );
void ValueBarycentricLerpArray( EValueType type,
                                const TMemoryView<u8>& dst, size_t dstStride,
                                const TMemoryView<const u8>& a,
                                const TMemoryView<const u8>& b,
                                const TMemoryView<const u8>& c,
                                const TMemoryView<const float3>& uvws );

bool ValueIsPromotable(EValueType dst, EValueType src);

bool ValuePromote(EValueType output, const TMemoryView<u8>& dst, EValueType input, const TMemoryView<const u8>& src);

bool ValuePromoteArray( EValueType output, const TMemoryView<u8>& dst, size_t dstStride,
                        EValueType input, const TMemoryView<const u8>& src, size_t srcStride,
                        size_t count );

void ValueSwap(EValueType type, const TMemoryView<u8>& lhs, const TMemoryView<u8>& rhs);
//----------------------------------------------------------------------------
template <typename T>
struct TValueTraits { STATIC_CONST_INTEGRAL(EValueType, ETypeId, EValueType::Void); };
template <EValueType _Type>
struct TValueTraitsReverse { typedef void type; };
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FValue {
public:
    union udata_type {
#define VALUETYPE_UNION_DEF(_Name, T, _TypeId, _Unused) T _Name;
FOREACH_CORE_GRAPHIC_VALUETYPE(VALUETYPE_UNION_DEF)
#undef VALUETYPE_UNION_DEF
    // http://en.cppreference.com/w/cpp/language/union
        udata_type() {}
        ~udata_type() {}
    };

    FValue() : _type(EValueType::Void) {}

    FValue(const FValue& other) { operator =(other); }
    FValue& operator =(const FValue& other) {
        _type = other._type;
        ValueCopy(_type, MakeView(), other.MakeView());
        return *this;
    };

    template <typename T>
    FValue(const T& value) { Set(value); }
    template <typename T>
    FValue& operator =(const T& value) { Set(value); return (*this); }

    EValueType Type() const { return _type; }

    bool empty() const { return (_type == EValueType::Void); }
    void clear()  { _type = EValueType::Void; }

    TMemoryView<u8> MakeView() { return TMemoryView<u8>((u8*)&_data, ValueSizeInBytes(_type)); }
    TMemoryView<const u8> MakeView() const { return TMemoryView<const u8>((const u8*)&_data, ValueSizeInBytes(_type)); }

    template <typename T>
    T& Get();
    template <typename T>
    const T& Get() const;
    template <typename T>
    void Set(const T& value);
    template <typename T>
    T Lerp(const T& other, float f) const;

    void SetRaw(EValueType type, const TMemoryView<const u8>& rawData) {
        Assert(EValueType::Void != type);
        Assert(ValueSizeInBytes(type) == rawData.SizeInBytes());
        _type = type;
        memcpy(&_data, rawData.data(), rawData.SizeInBytes());
    }

    friend hash_t hash_value(const FValue& value) {
        return ValueHash(value._type, value.MakeView());
    }

    friend bool operator ==(const FValue& lhs, const FValue& rhs) {
        return (lhs._type == rhs._type &&
                ValueEquals(lhs._type, lhs.MakeView(), rhs.MakeView()) );
    }

    friend bool operator !=(const FValue& lhs, const FValue& rhs) {
        return not operator ==(lhs, rhs);
    }

    friend std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const FValue& value);
    friend std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, const FValue& value);

private:
    EValueType _type;
    udata_type _data;
};
//----------------------------------------------------------------------------
inline FValue Lerp(const FValue& v0, const FValue& v1, float t) {
    Assert(v0.Type() == v1.Type());
    FValue result(v0);
    ValueLerp(v0.Type(), result.MakeView(), v0.MakeView(), v1.MakeView(), t);
    return result;
}
//----------------------------------------------------------------------------
#define VALUETYPE_TRAITS_DEF(_Name, T, _TypeId, _Unused) \
    template <> struct TValueTraits< T > { \
        STATIC_ASSERT(size_t(EValueType::_Name) == (_TypeId)); \
        STATIC_CONST_INTEGRAL(EValueType, ETypeId, EValueType::_Name); \
        static T& Get(FValue::udata_type& data) { return data._Name; } \
    }; \
    template <> struct TValueTraitsReverse< EValueType::_Name > { \
        typedef T type; \
    };
FOREACH_CORE_GRAPHIC_VALUETYPE(VALUETYPE_TRAITS_DEF)
#undef VALUETYPE_TRAITS_DEF
//----------------------------------------------------------------------------
template <typename T>
T& FValue::Get() {
    STATIC_ASSERT(TValueTraits<T>::ETypeId != EValueType::Void);
    Assert(_type == TValueTraits<T>::ETypeId);
    return TValueTraits<T>::Get(_data);
}
//----------------------------------------------------------------------------
template <typename T>
const T& FValue::Get() const {
    STATIC_ASSERT(TValueTraits<T>::ETypeId != EValueType::Void);
    Assert(_type == TValueTraits<T>::ETypeId);
    return TValueTraits<T>::Get(const_cast<udata_type&>(_data));
}
//----------------------------------------------------------------------------
template <typename T>
void FValue::Set(const T& value) {
    STATIC_ASSERT(TValueTraits<T>::ETypeId != EValueType::Void);
    _type = TValueTraits<T>::ETypeId;
    new ((void*)&TValueTraits<T>::Get(_data)) T(value);
}
//----------------------------------------------------------------------------
template <typename T>
T FValue::Lerp(const T& other, float f) const {
    STATIC_ASSERT(TValueTraits<T>::ETypeId != EValueType::Void);
    Assert(TValueTraits<T>::ETypeId == _type);
    return Lerp(TValueTraits<T>::Get(_data), other, f);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core

#include "Core.RTTI/RTTI_fwd.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FValue RTTI Wrapping :
//----------------------------------------------------------------------------
void GraphicsValueToAtom(RTTI::PMetaAtom& dst, const Graphics::FValue& src);
void AtomToGraphicsValue(Graphics::FValue& dst, const RTTI::PMetaAtom& src);
//----------------------------------------------------------------------------
template <>
struct TMetaTypeTraitsImpl< Graphics::FValue, void > {
    typedef Graphics::FValue wrapped_type;
    typedef RTTI::PMetaAtom wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypeScalarTraits< wrapper_type > *VirtualTraits();

    static bool IsDefaultValue(const wrapped_type& value) { return value.empty(); }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) { return (lhs == rhs); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { GraphicsValueToAtom(dst, src); }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { GraphicsValueToAtom(dst, src); }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { AtomToGraphicsValue(dst, src); }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { AtomToGraphicsValue(dst, src); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
