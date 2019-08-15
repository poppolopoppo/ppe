#pragma once

#include "RTTI.h"

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
enum class ETypeFlags : u32 {
    Default                 = 0,
    Scalar                  = 1<<0,
    Tuple                   = 1<<1,
    List                    = 1<<2,
    Dico                    = 1<<3,
    Enum                    = 1<<4,
    Object                  = 1<<5,
    Native                  = 1<<6,
    POD                     = 1<<7,
    TriviallyDestructible   = 1<<8,
};
ENUM_FLAGS(ETypeFlags);
//----------------------------------------------------------------------------
class FSizeAndFlags { // minimal packed type infos
public:
    CONSTEXPR FSizeAndFlags()
        : _sizeInBytes(0)
        , _flags(0)
    {}
    CONSTEXPR FSizeAndFlags(size_t sizeInBytes, ETypeFlags flags)
        : _sizeInBytes(u32(sizeInBytes))
        , _flags(u32(flags)) {
        Assert(SizeInBytes() == _sizeInBytes);
        Assert(Flags() == flags);
    }

    FSizeAndFlags(const FSizeAndFlags& other) = default;
    FSizeAndFlags& operator =(const FSizeAndFlags& other) = default;

    CONSTEXPR size_t SizeInBytes() const { return _sizeInBytes; }
    CONSTEXPR ETypeFlags Flags() const { return ETypeFlags(_flags); }

    CONSTEXPR friend bool operator ==(const FSizeAndFlags& lhs, const FSizeAndFlags& rhs) {
        return ((lhs._sizeInBytes == rhs._sizeInBytes) & (lhs._flags == rhs._flags));
    }
    CONSTEXPR friend bool operator !=(const FSizeAndFlags& lhs, const FSizeAndFlags& rhs) {
        return (not operator ==(lhs, rhs));
    }

private:
    u32 _sizeInBytes : 23;
    u32 _flags : 9;
};
//----------------------------------------------------------------------------
struct FTypeInfos {
    FTypeId TypeId{ 0 };
    FSizeAndFlags SizeAndFlags;

    FTypeInfos() = default;

    CONSTEXPR FTypeId Id() const { return TypeId; }
    CONSTEXPR size_t SizeInBytes() const { return SizeAndFlags.SizeInBytes(); }
    CONSTEXPR ETypeFlags Flags() const { return SizeAndFlags.Flags(); }

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
            { base.SizeInBytes(), CombineFlags(base.Flags(), typeInfos.Flags()...) }
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
            + (Meta::TIsPod_v<T> ? ETypeFlags::POD : ETypeFlags::Default)
            + (Meta::has_trivial_destructor<T>::value ? ETypeFlags::TriviallyDestructible : ETypeFlags::Default)
        };
    }

    template <typename T, FTypeId _NativeType, ETypeFlags _TypeFlags>
    static CONSTEXPR const PTypeInfos Scalar = []() CONSTEXPR NOEXCEPT -> FTypeInfos {
        return FTypeInfos{ _NativeType, BasicInfos<T>(ETypeFlags::Scalar + _TypeFlags) };
    };

    template <typename T, FTypeId _NativeType>
    static CONSTEXPR const PTypeInfos Enum = Scalar<T, _NativeType, ETypeFlags::Enum + ETypeFlags::Native>;
    template <typename T, FTypeId _NativeType>
    static CONSTEXPR const PTypeInfos Native = Scalar<T, _NativeType, ETypeFlags::Native>;
    template <typename T, FTypeId _NativeType>
    static CONSTEXPR const PTypeInfos NativeObject = Scalar<T, _NativeType, ETypeFlags::Native + ETypeFlags::Object>;
    template <typename T, FTypeId _NativeType>
    static CONSTEXPR const PTypeInfos Object = Scalar<T, _NativeType, ETypeFlags::Object>;

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

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FNamedTypeInfos {
public:
    CONSTEXPR FNamedTypeInfos() = default;

    FNamedTypeInfos(const FStringView& name, const FTypeInfos& type) NOEXCEPT
    :   FNamedTypeInfos(name, type.Id(), type.Flags(), type.SizeInBytes())
    {}
    FNamedTypeInfos(const FStringView& name, FTypeId id, ETypeFlags flags, size_t sizeInBytes) NOEXCEPT
    :   _name(name) , _typeInfos{ id, FSizeAndFlags(sizeInBytes, flags) } {
        Assert(not _name.empty());
        Assert(_typeInfos.Id());
        Assert(_typeInfos.SizeInBytes());
    }

    const FStringView& Name() const { return _name; }
    FTypeId Id() const { return _typeInfos.Id(); }
    ETypeFlags Flags() const { return _typeInfos.Flags(); }
    size_t SizeInBytes() const { return _typeInfos.SizeInBytes(); }

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
