#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Name.h"

#include "Core/IO/StringSlice.h"
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
enum class ValueType : u32 {
    Void = 0,

#define VALUETYPE_ENUM_DEF(_Name, T, _TypeId, _Unused) _Name = (_TypeId),
FOREACH_CORE_GRAPHIC_VALUETYPE(VALUETYPE_ENUM_DEF)
#undef VALUETYPE_ENUM_DEF
};
//----------------------------------------------------------------------------
StringSlice ValueTypeToCStr(ValueType value);
size_t ValueSizeInBytes(ValueType value);
//----------------------------------------------------------------------------
void ValueCopy(ValueType type, const MemoryView<u8>& dst, const MemoryView<const u8>& src);

void ValueDefault(ValueType type, const MemoryView<u8>& dst);

bool ValueEquals(ValueType type, const MemoryView<const u8>& lhs, const MemoryView<const u8>& rhs);

hash_t ValueHash(ValueType type, const MemoryView<const u8>& data);

void ValueLerp( ValueType type,
                const MemoryView<u8>& dst,
                const MemoryView<const u8>& a,
                const MemoryView<const u8>& b,
                float t );
void ValueLerpArray(ValueType type,
                    const MemoryView<u8>& dst, size_t dstStride,
                    const MemoryView<const u8>& a,
                    const MemoryView<const u8>& b,
                    const MemoryView<const float>& ts );

void ValueBarycentricLerp(  ValueType type, const MemoryView<u8>& dst,
                            const MemoryView<const u8>& a,
                            const MemoryView<const u8>& b,
                            const MemoryView<const u8>& c,
                            const float3& uvw );
void ValueBarycentricLerpArray( ValueType type,
                                const MemoryView<u8>& dst, size_t dstStride,
                                const MemoryView<const u8>& a,
                                const MemoryView<const u8>& b,
                                const MemoryView<const u8>& c,
                                const MemoryView<const float3>& uvws );

bool ValueIsPromotable(ValueType dst, ValueType src);

bool ValuePromote(ValueType output, const MemoryView<u8>& dst, ValueType input, const MemoryView<const u8>& src);

bool ValuePromoteArray( ValueType output, const MemoryView<u8>& dst, size_t dstStride,
                        ValueType input, const MemoryView<const u8>& src, size_t srcStride,
                        size_t count );

void ValueSwap(ValueType type, const MemoryView<u8>& lhs, const MemoryView<u8>& rhs);
//----------------------------------------------------------------------------
template <typename T>
struct ValueTraits { STATIC_CONST_INTEGRAL(ValueType, TypeId, ValueType::Void); };
template <ValueType _Type>
struct ValueTraitsReverse { typedef void type; };
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Value {
public:
    union udata_type {
#define VALUETYPE_UNION_DEF(_Name, T, _TypeId, _Unused) T _Name;
FOREACH_CORE_GRAPHIC_VALUETYPE(VALUETYPE_UNION_DEF)
#undef VALUETYPE_UNION_DEF
    // http://en.cppreference.com/w/cpp/language/union
        udata_type() {}
        ~udata_type() {}
    };

    Value() : _type(ValueType::Void) {}

    Value(const Value& other) { operator =(other); }
    Value& operator =(const Value& other) {
        _type = other._type;
        ValueCopy(_type, MakeView(), other.MakeView());
        return *this;
    };

    template <typename T>
    Value(const T& value) { Set(value); }
    template <typename T>
    Value& operator =(const T& value) { Set(value); return (*this); }

    ValueType Type() const { return _type; }

    bool empty() const { return (_type == ValueType::Void); }
    void clear()  { _type = ValueType::Void; }

    MemoryView<u8> MakeView() { return MemoryView<u8>((u8*)&_data, ValueSizeInBytes(_type)); }
    MemoryView<const u8> MakeView() const { return MemoryView<const u8>((const u8*)&_data, ValueSizeInBytes(_type)); }

    template <typename T>
    T& Get();
    template <typename T>
    const T& Get() const;
    template <typename T>
    void Set(const T& value);
    template <typename T>
    T Lerp(const T& other, float f) const;

    void SetRaw(ValueType type, const MemoryView<const u8>& rawData) {
        Assert(ValueType::Void != type);
        Assert(ValueSizeInBytes(type) == rawData.SizeInBytes());
        _type = type;
        memcpy(&_data, rawData.data(), rawData.SizeInBytes());
    }

    friend hash_t hash_value(const Value& value) {
        return ValueHash(value._type, value.MakeView());
    }

    friend bool operator ==(const Value& lhs, const Value& rhs) {
        return (lhs._type == rhs._type &&
                ValueEquals(lhs._type, lhs.MakeView(), rhs.MakeView()) );
    }

    friend bool operator !=(const Value& lhs, const Value& rhs) {
        return not operator ==(lhs, rhs);
    }

private:
    ValueType _type;
    udata_type _data;
};
//----------------------------------------------------------------------------
inline Value Lerp(const Value& v0, const Value& v1, float t) {
    Assert(v0.Type() == v1.Type());
    Value result(v0);
    ValueLerp(v0.Type(), result.MakeView(), v0.MakeView(), v1.MakeView(), t);
    return result;
}
//----------------------------------------------------------------------------
#define VALUETYPE_TRAITS_DEF(_Name, T, _TypeId, _Unused) \
    template <> struct ValueTraits< T > { \
        STATIC_ASSERT(size_t(ValueType::_Name) == (_TypeId)); \
        STATIC_CONST_INTEGRAL(ValueType, TypeId, ValueType::_Name); \
        static T& Get(Value::udata_type& data) { return data._Name; } \
    }; \
    template <> struct ValueTraitsReverse< ValueType::_Name > { \
        typedef T type; \
    };
FOREACH_CORE_GRAPHIC_VALUETYPE(VALUETYPE_TRAITS_DEF)
#undef VALUETYPE_TRAITS_DEF
//----------------------------------------------------------------------------
template <typename T>
T& Value::Get() {
    STATIC_ASSERT(ValueTraits<T>::TypeId != ValueType::Void);
    Assert(_type == ValueTraits<T>::TypeId);
    return ValueTraits<T>::Get(_data);
}
//----------------------------------------------------------------------------
template <typename T>
const T& Value::Get() const {
    STATIC_ASSERT(ValueTraits<T>::TypeId != ValueType::Void);
    Assert(_type == ValueTraits<T>::TypeId);
    return ValueTraits<T>::Get(const_cast<udata_type&>(_data));
}
//----------------------------------------------------------------------------
template <typename T>
void Value::Set(const T& value) {
    STATIC_ASSERT(ValueTraits<T>::TypeId != ValueType::Void);
    _type = ValueTraits<T>::TypeId;
    new ((void*)&ValueTraits<T>::Get(_data)) T(value);
}
//----------------------------------------------------------------------------
template <typename T>
T Value::Lerp(const T& other, float f) const {
    STATIC_ASSERT(ValueTraits<T>::TypeId != ValueType::Void);
    Assert(ValueTraits<T>::TypeId == _type);
    return Lerp(ValueTraits<T>::Get(_data), other, f);
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
// Value RTTI Wrapping :
//----------------------------------------------------------------------------
void GraphicsValueToAtom(RTTI::PMetaAtom& dst, const Graphics::Value& src);
void AtomToGraphicsValue(Graphics::Value& dst, const RTTI::PMetaAtom& src);
//----------------------------------------------------------------------------
template <>
struct MetaTypeTraitsImpl< Graphics::Value > {
    typedef Graphics::Value wrapped_type;
    typedef RTTI::PMetaAtom wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeScalarTraits< wrapper_type > *VirtualTraits();

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
