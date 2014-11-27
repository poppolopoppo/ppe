#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core/Container/Vector.h"
#include "Core/Memory/MemoryView.h"
#include "Core/RTTI/RTTI_fwd.h"

namespace Core {
class ThreadLocalIStringStream;
class ThreadLocalOStringStream;

namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ISerializer {
public:
    virtual ~ISerializer() {}

    ISerializer(const ISerializer& ) = delete;
    ISerializer& operator =(const ISerializer& ) = delete;

    virtual RTTI::MetaTransaction *Transaction() const = 0;

    virtual void Deserialize(VECTOR(Transaction, RTTI::PMetaObject)& objects, ThreadLocalIStringStream& input) = 0;
    virtual void Serialize(ThreadLocalOStringStream& output, const MemoryView<const RTTI::PCMetaObject>& objects) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
