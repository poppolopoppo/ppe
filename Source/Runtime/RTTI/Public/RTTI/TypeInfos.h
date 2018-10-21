#pragma once

#include "RTTI.h"

#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FTypeId = u32;
//----------------------------------------------------------------------------
enum class ETypeFlags : u32 {
    Scalar                  = 1<<0,
    Tuple                   = 1<<1,
    List                    = 1<<2,
    Dico                    = 1<<3,
    Enum                    = 1<<4,
    Native                  = 1<<5,
    POD                     = 1<<6,
    TriviallyDestructible   = 1<<7,
};
ENUM_FLAGS(ETypeFlags);
//----------------------------------------------------------------------------
class FSizeAndFlags { // minimal packed type infos
public:
    FSizeAndFlags(size_t sizeInBytes, ETypeFlags flags)
        : _sizeInBytes(checked_cast<u32>(sizeInBytes))
        , _flags(u32(flags)) {
        Assert(sizeInBytes == _sizeInBytes);
        Assert(u32(flags) == _flags);
    }

    FSizeAndFlags(const FSizeAndFlags& other) { operator =(other); }
    FSizeAndFlags& operator =(const FSizeAndFlags& other) {
        *(u32*)this = *(const u32*)&other;
        return (*this);
    }

    size_t SizeInBytes() const { return _sizeInBytes; }
    ETypeFlags Flags() const { return ETypeFlags(_flags); }

private:
    u32 _sizeInBytes : 24;
    u32 _flags : 8;
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
