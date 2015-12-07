#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Exceptions.h"
#include "Core.Serialize/ISerializer.h"

#include "Core.RTTI/RTTI_fwd.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class BinarySerializerException : public SerializeException {
public:
    BinarySerializerException(const char* what) : SerializeException(what) {}
};
//----------------------------------------------------------------------------
class BinarySerializer : public ISerializer {
public:
    BinarySerializer(RTTI::MetaTransaction *transaction);
    virtual ~BinarySerializer();

    virtual RTTI::MetaTransaction *Transaction() const override { return _transaction.get(); }

    using ISerializer::Deserialize;
    using ISerializer::Serialize;

    virtual void Deserialize(VECTOR(Transaction, RTTI::PMetaObject)& objects, const MemoryView<const u8>& input, const wchar_t *sourceName = nullptr) override;
    virtual void Serialize(IStreamWriter* output, const MemoryView<const RTTI::PMetaObject>& objects) override;

private:
    RTTI::SMetaTransaction _transaction;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
