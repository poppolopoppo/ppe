#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/IO/StringView.h"
#include "Core/IO/TextWriter_fwd.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FTypeId = u32;
//----------------------------------------------------------------------------
enum class ETypeFlags : u32 {
    Scalar      = 1<<0,
    Pair        = 1<<1,
    List        = 1<<2,
    Dico        = 1<<3,
    Native      = 1<<4,
    Object      = 1<<5,
};
ENUM_FLAGS(ETypeFlags);
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
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, RTTI::ETypeFlags flags);
CORE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, RTTI::ETypeFlags flags);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
