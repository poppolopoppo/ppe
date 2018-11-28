#pragma once

#include "RTTI.h"

#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
    CONSTEXPR FSizeAndFlags(size_t sizeInBytes, ETypeFlags flags)
        : _sizeInBytes(u32(sizeInBytes))
        , _flags(u32(flags))
    {}

    FSizeAndFlags(const FSizeAndFlags& other) { operator =(other); }
    FSizeAndFlags& operator =(const FSizeAndFlags& other) {
        *(u32*)this = *(const u32*)&other;
        return (*this);
    }

    size_t SizeInBytes() const { return _sizeInBytes; }
    ETypeFlags Flags() const { return ETypeFlags(_flags); }

    inline friend bool operator ==(const FSizeAndFlags& lhs, const FSizeAndFlags& rhs) {
        return (lhs._sizeInBytes == rhs._sizeInBytes && lhs._flags == rhs._flags);
    }
    inline friend bool operator !=(const FSizeAndFlags& lhs, const FSizeAndFlags& rhs) {
        return not operator ==(lhs, rhs);
    }

private:
    u32 _sizeInBytes : 23;
    u32 _flags : 9;
};
//----------------------------------------------------------------------------
class FTypeInfos {
public:
    FTypeInfos()
        : _id(FTypeId(0))
        , _flags(ETypeFlags(0))
        , _sizeInBytes(0)
    {}

    FTypeInfos(const FStringView& name, FTypeId id, ETypeFlags flags, size_t sizeInBytes)
        : _name(name)
        , _id(id)
        , _flags(flags)
        , _sizeInBytes(sizeInBytes) {
        Assert(not _name.empty());
        Assert(_id);
        Assert(_sizeInBytes);
    }

    const FStringView& Name() const { return _name; }
    FTypeId Id() const { return _id; }
    ETypeFlags Flags() const { return _flags; }
    size_t SizeInBytes() const { return _sizeInBytes; }

    inline friend bool operator ==(const FTypeInfos& lhs, const FTypeInfos& rhs) { return (lhs._id == rhs._id); }
    inline friend bool operator !=(const FTypeInfos& lhs, const FTypeInfos& rhs) { return not operator ==(lhs, rhs); }

private:
    FStringView _name;
    FTypeId _id;
    ETypeFlags _flags;
    size_t _sizeInBytes;
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
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, const RTTI::FTypeInfos& typeInfos);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FTypeInfos& typeInfos);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
