#pragma once

#include "RTTI_fwd.h"

#include "Container/Hash.h"
#include "Container/Tuple.h"

#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// need to be in RTTI namespace for ADL
template <typename T>
struct TType {};
template <typename T>
CONSTEXPR TType<T> Type{};
//----------------------------------------------------------------------------
enum class ETypeFlags : u16 {
    // /!\ Report changes to MetaEnumHelpers.cpp

    Default                 = 0,

    // type traits
    Scalar                  = 1<<0,
    Tuple                   = 1<<1,
    List                    = 1<<2,
    Dico                    = 1<<3,

    // content type
    Alias                   = 1<<4,
    Arithmetic              = 1<<5,
    Boolean                 = 1<<6,
    Enum                    = 1<<7,
    FloatingPoint           = 1<<8,
    Native                  = 1<<9,
    Object                  = 1<<10,
    String                  = 1<<11,
    SignedIntegral          = 1<<12,
    UnsignedIntegral        = 1<<13,

    // requirements
    POD                     = 1<<14,
    TriviallyDestructible   = 1<<15,
};
ENUM_FLAGS(ETypeFlags);
//----------------------------------------------------------------------------
CONSTEXPR bool is_scalar_v(ETypeFlags flags) { return (flags & ETypeFlags::Scalar); }
CONSTEXPR bool is_tuple_v(ETypeFlags flags) { return (flags & ETypeFlags::Tuple); }
CONSTEXPR bool is_list_v(ETypeFlags flags) { return (flags & ETypeFlags::List); }
CONSTEXPR bool is_dico_v(ETypeFlags flags) { return (flags & ETypeFlags::Dico); }
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
CONSTEXPR bool is_pod_v(ETypeFlags flags) { return (flags & ETypeFlags::POD); }
CONSTEXPR bool is_trivially_destructible_v(ETypeFlags flags) { return (flags & ETypeFlags::TriviallyDestructible); }
//----------------------------------------------------------------------------
class FSizeAndFlags { // minimal packed type infos
public:
    u16 SizeInBytes;
    ETypeFlags Flags;

    FSizeAndFlags() = default;

    CONSTEXPR FSizeAndFlags(size_t sizeInBytes, ETypeFlags flags)
    :   SizeInBytes(u16(sizeInBytes))
    ,   Flags(flags) {
        Assert(SizeInBytes == sizeInBytes);
        Assert(Flags == flags);
        STATIC_ASSERT(sizeof(FSizeAndFlags) == sizeof(u32));
    }

    FSizeAndFlags(const FSizeAndFlags& other) = default;
    FSizeAndFlags& operator =(const FSizeAndFlags& other) = default;

	FSizeAndFlags(FSizeAndFlags&& rvalue) = default;
	FSizeAndFlags& operator =(FSizeAndFlags&& rvalue) = default;

    CONSTEXPR friend bool operator ==(const FSizeAndFlags& lhs, const FSizeAndFlags& rhs) {
        return ((lhs.SizeInBytes == rhs.SizeInBytes) & (lhs.Flags == rhs.Flags));
    }
    CONSTEXPR friend bool operator !=(const FSizeAndFlags& lhs, const FSizeAndFlags& rhs) {
        return (not operator ==(lhs, rhs));
    }

};
//----------------------------------------------------------------------------
struct FTypeInfos {
    FTypeId TypeId{ 0 };
    FSizeAndFlags SizeAndFlags;

    FTypeInfos() = default;

    CONSTEXPR FTypeId Id() const { return TypeId; }
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

    CONSTEXPR bool IsPOD() const { return is_pod_v(Flags()); }
    CONSTEXPR bool IsTriviallyDestructible() const { return is_trivially_destructible_v(Flags()); }


    CONSTEXPR friend bool operator ==(FTypeInfos lhs, FTypeInfos rhs) {
        return ((lhs.TypeId == rhs.TypeId) & (lhs.SizeAndFlags == rhs.SizeAndFlags));
    }
    CONSTEXPR friend bool operator !=(FTypeInfos lhs, FTypeInfos rhs) {
        return (not operator ==(lhs, rhs));
    }


    template <typename... _TypeId>
    static CONSTEXPR FTypeId CombineIds(FTypeId seed, _TypeId... typeId) {
        return static_cast<u32>(hash_tuple(seed, typeId...));
    }
    template <typename... _TypeFlags>
    static CONSTEXPR ETypeFlags CombineFlags(ETypeFlags seed, _TypeFlags... typeFlags) {
        return (seed
            // if any has object then the result has object
            + ( ... + (typeFlags ^ ETypeFlags::Object ? ETypeFlags::Object : ETypeFlags::Default) )
            // if any non POD then the result can't be POD
            - ( ... + (typeFlags ^ ETypeFlags::POD ? ETypeFlags::Default : ETypeFlags::POD) )
            // if any non trivially destructible then the result can't be trivially destructible
            - ( ... + (typeFlags ^ ETypeFlags::TriviallyDestructible ? ETypeFlags::Default : ETypeFlags::TriviallyDestructible) )
        );
    }
    template <typename... _TypeInfos>
    static CONSTEXPR FTypeInfos CombineTypes(FTypeId typeId, FSizeAndFlags base, _TypeInfos... typeInfos) {
        return {
            CombineIds(typeId, typeInfos.Id()...),
            { base.SizeInBytes, CombineFlags(base.Flags, typeInfos.Flags()...) }
        };
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using PTypeInfos = FTypeInfos(*)(void) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR FTypeInfos MakeTypeInfos() NOEXCEPT {
    return TypeInfos(Type< Meta::TDecay<T> >)();
}
//----------------------------------------------------------------------------
struct FTypeHelpers {
    template <typename T>
    static CONSTEXPR FSizeAndFlags BasicInfos(ETypeFlags base = ETypeFlags::Default) {
        return {
            sizeof(T), base
            + (std::is_arithmetic_v<T> ? ETypeFlags::Arithmetic : ETypeFlags::Default)
            + (std::is_floating_point_v<T> ? ETypeFlags::FloatingPoint : ETypeFlags::Default)
            + (std::is_integral_v<T> && std::is_signed_v<T> ? ETypeFlags::SignedIntegral : ETypeFlags::Default)
            + (std::is_integral_v<T> && std::is_unsigned_v<T> ? ETypeFlags::UnsignedIntegral : ETypeFlags::Default)
            + (Meta::is_pod_v<T> ? ETypeFlags::POD : ETypeFlags::Default)
            + (Meta::has_trivial_destructor<T>::value ? ETypeFlags::TriviallyDestructible : ETypeFlags::Default)
        };
    }

    template <typename T, FTypeId _NativeType, ETypeFlags _TypeFlags>
    static CONSTEXPR const PTypeInfos Scalar = []() CONSTEXPR NOEXCEPT -> FTypeInfos {
        return FTypeInfos{ _NativeType, BasicInfos<T>(ETypeFlags::Scalar + _TypeFlags) };
    };

    template <typename T, FTypeId _NativeType>
    static CONSTEXPR const PTypeInfos Boolean = Scalar<T, _NativeType, ETypeFlags::Boolean + ETypeFlags::Native>;
    template <typename T, FTypeId _NativeType>
    static CONSTEXPR const PTypeInfos Enum = Scalar<T, _NativeType, ETypeFlags::Enum + ETypeFlags::Native>;
    template <typename T, FTypeId _NativeType>
    static CONSTEXPR const PTypeInfos Native = Scalar<T, _NativeType, ETypeFlags::Native>;
    template <typename T, FTypeId _NativeType>
    static CONSTEXPR const PTypeInfos NativeObject = Scalar<T, _NativeType, ETypeFlags::Native + ETypeFlags::Object>;
    template <typename T, FTypeId _NativeType>
    static CONSTEXPR const PTypeInfos Object = Scalar<T, _NativeType, ETypeFlags::Object>;
    template <typename T, FTypeId _NativeType>
    static CONSTEXPR const PTypeInfos String = Scalar<T, _NativeType, ETypeFlags::String + ETypeFlags::Native>;

    template <typename T, typename... _Args>
    static CONSTEXPR const PTypeInfos Tuple = []() CONSTEXPR NOEXCEPT -> FTypeInfos {
        return FTypeInfos::CombineTypes(FTypeId(ETypeFlags::Tuple), BasicInfos<T>(ETypeFlags::Tuple), MakeTypeInfos<_Args>()... );
    };

    template <typename T, typename _Item>
    static CONSTEXPR const PTypeInfos List = []() CONSTEXPR NOEXCEPT -> FTypeInfos {
        return FTypeInfos::CombineTypes(FTypeId(ETypeFlags::List), BasicInfos<T>(ETypeFlags::List), MakeTypeInfos<_Item>() );
    };

    template <typename T, typename _Key, typename _Value>
    static CONSTEXPR const PTypeInfos Dico = []() CONSTEXPR NOEXCEPT -> FTypeInfos {
        return FTypeInfos::CombineTypes(FTypeId(ETypeFlags::Dico), BasicInfos<T>(ETypeFlags::Dico), MakeTypeInfos<_Key>(), MakeTypeInfos<_Value>() );
    };

    // defer alias evaluation, so _From/_To don't need to defined (forward declaration is enough)
    template <typename _From, typename _To, typename _Aliased>
    struct aliased_type_infos_t {
        _Aliased AliasedPTypeInfos;
        CONSTEXPR FTypeInfos operator ()() const {
            //STATIC_ASSERT(sizeof(_From) == sizeof(_To));
            const FTypeInfos infos = AliasedPTypeInfos();
            return FTypeInfos{
                infos.TypeId,
                FSizeAndFlags{
                    infos.SizeInBytes(),
                    infos.Flags() + ETypeFlags::Alias
                }};
        }
    };

    template <typename _From, typename _To>
    static CONSTEXPR const auto Alias = aliased_type_infos_t<
        _From, _To, decltype(TypeInfos(Type<_To>))> {
        TypeInfos(Type<_To>)
    };

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FNamedTypeInfos {
public:
    FNamedTypeInfos() = default;

    CONSTEXPR FNamedTypeInfos(const FStringView& name, const FTypeInfos& type) NOEXCEPT
    :   FNamedTypeInfos(name, type.Id(), type.Flags(), type.SizeInBytes())
    {}
    CONSTEXPR FNamedTypeInfos(const FStringView& name, FTypeId id, ETypeFlags flags, size_t sizeInBytes) NOEXCEPT
    :   _name(name) , _typeInfos{ id, FSizeAndFlags(sizeInBytes, flags) } {
        Assert(not _name.empty());
        Assert(_typeInfos.Id());
        Assert(_typeInfos.SizeInBytes());
    }

    CONSTEXPR const FStringView& Name() const { return _name; }
    CONSTEXPR const FTypeInfos& TypeInfos() const { return _typeInfos; }

    CONSTEXPR FTypeId Id() const { return _typeInfos.Id(); }
    CONSTEXPR ETypeFlags Flags() const { return _typeInfos.Flags(); }
    CONSTEXPR size_t SizeInBytes() const { return _typeInfos.SizeInBytes(); }

    CONSTEXPR friend bool operator ==(const FNamedTypeInfos& lhs, const FNamedTypeInfos& rhs) { return (lhs._typeInfos == rhs._typeInfos); }
    CONSTEXPR friend bool operator !=(const FNamedTypeInfos& lhs, const FNamedTypeInfos& rhs) { return not operator ==(lhs, rhs); }

private:
    FStringView _name;
    FTypeInfos _typeInfos;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, RTTI::ETypeFlags flags);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, RTTI::ETypeFlags flags);
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, const RTTI::FNamedTypeInfos& typeInfos);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FNamedTypeInfos& typeInfos);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
