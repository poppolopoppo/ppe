#pragma once

#include "RTTI_fwd.h"

#include "Container/Hash.h"
#include "Container/Tuple.h"

#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"

#include "HAL/PlatformMaths.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ETypeFlags : u16 {
    // /!\ Report changes to MetaEnumHelpers.cpp

    // type traits
    __MASK_Traits           = 0x3,
    Scalar                  = 0,
    Tuple                   = 1,
    List                    = 2,
    Dico                    = 3,

    // content type
    Alias                   = 1<<2,
    Arithmetic              = 1<<3,
    Boolean                 = 1<<4,
    Enum                    = 1<<5,
    FloatingPoint           = 1<<6,
    Native                  = 1<<7,
    Object                  = 1<<8,
    String                  = 1<<9,
    SignedIntegral          = 1<<10,
    UnsignedIntegral        = 1<<11,

    // requirements
    Comparable              = 1<<12,
    POD                     = 1<<13,
    TriviallyDestructible   = 1<<14,

    Unknown = 0
};
ENUM_FLAGS(ETypeFlags);
//----------------------------------------------------------------------------
CONSTEXPR bool is_scalar_v(ETypeFlags flags) { return (Meta::EnumAnd(flags, ETypeFlags::__MASK_Traits) == ETypeFlags::Scalar); }
CONSTEXPR bool is_tuple_v(ETypeFlags flags) { return (Meta::EnumAnd(flags, ETypeFlags::__MASK_Traits) == ETypeFlags::Tuple); }
CONSTEXPR bool is_list_v(ETypeFlags flags) { return (Meta::EnumAnd(flags, ETypeFlags::__MASK_Traits) == ETypeFlags::List); }
CONSTEXPR bool is_dico_v(ETypeFlags flags) { return (Meta::EnumAnd(flags, ETypeFlags::__MASK_Traits) == ETypeFlags::Dico); }
CONSTEXPR bool is_alias_v(ETypeFlags flags) { return (flags & ETypeFlags::Alias); }
CONSTEXPR bool is_arithmetic_v(ETypeFlags flags) { return (flags & ETypeFlags::Arithmetic); }
CONSTEXPR bool is_boolean_v(ETypeFlags flags) { return (flags & ETypeFlags::Boolean); }
CONSTEXPR bool is_enum_v(ETypeFlags flags) { return (flags & ETypeFlags::Enum); }
CONSTEXPR bool is_floating_point_v(ETypeFlags flags) { return (flags & ETypeFlags::FloatingPoint); }
CONSTEXPR bool is_native_v(ETypeFlags flags) { return (flags & ETypeFlags::Native); }
CONSTEXPR bool is_object_v(ETypeFlags flags) { return (flags & ETypeFlags::Object); }
CONSTEXPR bool is_string_v(ETypeFlags flags) { return (flags & ETypeFlags::String); }
CONSTEXPR bool is_integral_v(ETypeFlags flags) { return (flags & ETypeFlags::SignedIntegral || flags & ETypeFlags::UnsignedIntegral); }
CONSTEXPR bool is_signed_integral_v(ETypeFlags flags) { return (flags & ETypeFlags::SignedIntegral); }
CONSTEXPR bool is_unsigned_integral_v(ETypeFlags flags) { return (flags & ETypeFlags::UnsignedIntegral); }
CONSTEXPR bool is_structured_v(ETypeFlags flags) { return (not is_scalar_v(flags) || is_object_v(flags)); }
CONSTEXPR bool is_comparable_v(ETypeFlags flags) { return (flags & ETypeFlags::Comparable); }
CONSTEXPR bool is_pod_v(ETypeFlags flags) { return (flags & ETypeFlags::POD); }
CONSTEXPR bool is_trivially_destructible_v(ETypeFlags flags) { return (flags & ETypeFlags::TriviallyDestructible); }
//----------------------------------------------------------------------------
class FSizeAndFlags { // minimal packed type infos
public:
    u16 PackedAlignment : 2;
    u16 SizeInBytes : 14;
    ETypeFlags Flags{ Default };

    CONSTEXPR size_t Alignment() const { return UnpackAlignment(PackedAlignment); }

    FSizeAndFlags() = default;

    CONSTEXPR FSizeAndFlags(size_t alignment, size_t sizeInBytes, ETypeFlags flags)
    :   PackedAlignment(PackAlignment(alignment))
    ,   SizeInBytes(static_cast<u16>(sizeInBytes))
    ,   Flags(flags) {
        Assert(SizeInBytes == sizeInBytes);
        STATIC_ASSERT(sizeof(FSizeAndFlags) == 4);
    }

    FSizeAndFlags(const FSizeAndFlags& other) = default;
    FSizeAndFlags& operator =(const FSizeAndFlags& other) = default;

    FSizeAndFlags(FSizeAndFlags&& rvalue) = default;
    FSizeAndFlags& operator =(FSizeAndFlags&& rvalue) = default;

    CONSTEXPR friend bool operator ==(const FSizeAndFlags& lhs, const FSizeAndFlags& rhs) {
        return ((lhs.PackedAlignment == rhs.PackedAlignment) &&
                (lhs.SizeInBytes == rhs.SizeInBytes) &&
                (lhs.Flags == rhs.Flags) );
    }
    CONSTEXPR friend bool operator !=(const FSizeAndFlags& lhs, const FSizeAndFlags& rhs) {
        return (not operator ==(lhs, rhs));
    }

    static CONSTEXPR size_t UnpackAlignment(u16 packed) {
        return (static_cast<size_t>(1) << packed);
    }
    static CONSTEXPR u16 PackAlignment(size_t value) {
        Assert(Meta::IsPow2(value));
        const u16 packed = checked_cast<u16>(FGenericPlatformMaths/* constexpr */::FloorLog2(value));
        Assert(UnpackAlignment(packed) == value);
        return packed;
    }

};
PPE_ASSUME_TYPE_AS_POD(FSizeAndFlags);
//----------------------------------------------------------------------------
using PTypeInfos = struct FTypeInfos (*)(void) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T>
struct TTypeTag {};
template <typename T>
CONSTEXPR const TTypeTag< Meta::TDecay<T> > TypeTag;
//----------------------------------------------------------------------------
struct FTypeInfos {
    FTypeId TypeId{ 0 };
    FSizeAndFlags SizeAndFlags{};

    FTypeInfos() = default;
    CONSTEXPR FTypeInfos(FTypeId typeId, FSizeAndFlags sizeAndFlags) NOEXCEPT
    :   TypeId(typeId)
    ,   SizeAndFlags(sizeAndFlags)
    {}

    CONSTEXPR FTypeId Id() const { return TypeId; }
    CONSTEXPR size_t Alignment() const { return SizeAndFlags.Alignment(); }
    CONSTEXPR size_t SizeInBytes() const { return SizeAndFlags.SizeInBytes; }
    CONSTEXPR ETypeFlags Flags() const { return SizeAndFlags.Flags; }

    CONSTEXPR bool IsScalar() const { return is_scalar_v(Flags()); }
    CONSTEXPR bool IsTuple() const { return is_tuple_v(Flags()); }
    CONSTEXPR bool IsList() const { return is_list_v(Flags()); }
    CONSTEXPR bool IsDico() const { return is_dico_v(Flags()); }

    CONSTEXPR bool IsAlias() const { return is_alias_v(Flags()); }
    CONSTEXPR bool IsArithmetic() const { return is_arithmetic_v(Flags()); }
    CONSTEXPR bool IsBoolean() const { return is_boolean_v(Flags()); }
    CONSTEXPR bool IsEnum() const { return is_enum_v(Flags()); }
    CONSTEXPR bool IsFloatingPoint() const { return is_floating_point_v(Flags()); }
    CONSTEXPR bool IsNative() const { return is_native_v(Flags()); }
    CONSTEXPR bool IsObject() const { return is_object_v(Flags()); }
    CONSTEXPR bool IsString() const { return is_string_v(Flags()); }
    CONSTEXPR bool IsIntegral() const { return is_integral_v(Flags()); }
    CONSTEXPR bool IsSignedIntegral() const { return is_integral_v(Flags()); }
    CONSTEXPR bool IsUnsignedIntegral() const { return is_unsigned_integral_v(Flags()); }
    CONSTEXPR bool IsStructured() const { return is_structured_v(Flags()); }

    CONSTEXPR bool IsComparable() const { return is_comparable_v(Flags()); }
    CONSTEXPR bool IsPOD() const { return is_pod_v(Flags()); }
    CONSTEXPR bool IsTriviallyDestructible() const { return is_trivially_destructible_v(Flags()); }


    CONSTEXPR friend bool operator ==(FTypeInfos lhs, FTypeInfos rhs) {
        return ((lhs.TypeId == rhs.TypeId) && (lhs.SizeAndFlags == rhs.SizeAndFlags));
    }
    CONSTEXPR friend bool operator !=(FTypeInfos lhs, FTypeInfos rhs) {
        return (not operator ==(lhs, rhs));
    }

    template <typename T>
    static CONSTEXPR FSizeAndFlags BasicInfos(ETypeFlags base = Default) {
        return {
            alignof(T), sizeof(T), base
            + (std::is_arithmetic_v<T> ? ETypeFlags::Arithmetic : Default)
            + (std::is_floating_point_v<T> ? ETypeFlags::FloatingPoint : Default)
            + (std::is_integral_v<T> && std::is_signed_v<T> ? ETypeFlags::SignedIntegral : Default)
            + (std::is_integral_v<T> && std::is_unsigned_v<T> ? ETypeFlags::UnsignedIntegral : Default)
            + (Meta::has_trivial_compare_v<T> ? ETypeFlags::Comparable : Default)
            + (Meta::is_pod_v<T> ? ETypeFlags::POD : Default)
            + (Meta::has_trivial_destructor<T>::value ? ETypeFlags::TriviallyDestructible : Default)
        };
    }

    template <typename... _TypeId>
    static CONSTEXPR FTypeId CombineIds(FTypeId seed, _TypeId... typeId) {
        return static_cast<u32>(hash_tuple(seed, typeId...));
    }

    template <typename... _TypeFlags>
    static CONSTEXPR ETypeFlags CombineFlags(ETypeFlags seed, _TypeFlags... typeFlags) {
        return (seed
            // if any has object then the result has object
            + ( ... + (typeFlags ^ ETypeFlags::Object ? ETypeFlags::Object : Default) )
            // if any non comparable then the result can't be comparable
            - ( ... + (typeFlags ^ ETypeFlags::Comparable ? Default : ETypeFlags::Comparable) )
            // if any non POD then the result can't be POD
            - ( ... + (typeFlags ^ ETypeFlags::POD ? Default : ETypeFlags::POD) )
            // if any non trivially destructible then the result can't be trivially destructible
            - ( ... + (typeFlags ^ ETypeFlags::TriviallyDestructible ? Default : ETypeFlags::TriviallyDestructible) )
        );
    }

    template <typename... _TypeInfos>
    static CONSTEXPR FTypeInfos CombineTypes(FTypeId typeId, FSizeAndFlags base, _TypeInfos... typeInfos) {
        return {
            CombineIds(typeId, typeInfos.Id()...),
            { base.Alignment(), base.SizeInBytes, CombineFlags(base.Flags, typeInfos.Flags()...) }
        };
    }
};
PPE_ASSUME_TYPE_AS_POD(FTypeInfos);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FNamedTypeInfos {
public:
    FNamedTypeInfos() = default;

    CONSTEXPR FNamedTypeInfos(FStringLiteral name, const FTypeInfos& type) NOEXCEPT
    :   FNamedTypeInfos(name, type.Id(), type.Flags(), type.Alignment(), type.SizeInBytes())
    {}
    CONSTEXPR FNamedTypeInfos(FStringLiteral name, FTypeId id, ETypeFlags flags, size_t alignment, size_t sizeInBytes) NOEXCEPT
    :   _name(name)
    ,   _typeInfos(id, FSizeAndFlags(alignment, sizeInBytes, flags)) {
        Assert(not _name.empty());
        Assert(_typeInfos.Id());
        Assert(_typeInfos.SizeInBytes());
    }

    CONSTEXPR FStringLiteral Name() const { return _name; }
    CONSTEXPR const FTypeInfos& TypeInfos() const { return _typeInfos; }

    CONSTEXPR FTypeId Id() const { return _typeInfos.Id(); }
    CONSTEXPR ETypeFlags Flags() const { return _typeInfos.Flags(); }
    CONSTEXPR size_t SizeInBytes() const { return _typeInfos.SizeInBytes(); }

    CONSTEXPR friend bool operator ==(const FNamedTypeInfos& lhs, const FNamedTypeInfos& rhs) { return (lhs._typeInfos == rhs._typeInfos); }
    CONSTEXPR friend bool operator !=(const FNamedTypeInfos& lhs, const FNamedTypeInfos& rhs) { return not operator ==(lhs, rhs); }

private:
    FStringLiteral _name;
    FTypeInfos _typeInfos;
};
PPE_ASSUME_TYPE_AS_POD(FNamedTypeInfos);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, ETypeFlags flags);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, ETypeFlags flags);
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, const FNamedTypeInfos& typeInfos);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, const FNamedTypeInfos& typeInfos);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
