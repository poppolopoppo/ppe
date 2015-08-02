#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core/Container/RawStorage.h"
#include "Core/Container/Vector.h"
#include "Core/Memory/MemoryView.h"
#include "Core.RTTI/RTTI_fwd.h"

namespace Core {
class IVirtualFileSystemIStream;
class IVirtualFileSystemOStream;

namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ISerializer {
public:
    ISerializer() {}
    virtual ~ISerializer() {}

    ISerializer(const ISerializer& ) = delete;
    ISerializer& operator =(const ISerializer& ) = delete;

    virtual RTTI::MetaTransaction *Transaction() const = 0;

    virtual void Deserialize(VECTOR(Transaction, RTTI::PMetaAtom)& atoms, const RAWSTORAGE(Serializer, u8)& input, const char *sourceName = nullptr) = 0;
    virtual void Serialize(RAWSTORAGE(Serializer, u8)& output, const MemoryView<const RTTI::PCMetaAtom>& atoms) = 0;

    void Deserialize(VECTOR(Transaction, RTTI::PMetaAtom)& atoms, IVirtualFileSystemIStream *iss);
    void Serialize(IVirtualFileSystemOStream *oss, const MemoryView<const RTTI::PCMetaAtom>& atoms);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
