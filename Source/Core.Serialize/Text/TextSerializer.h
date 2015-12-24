#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Exceptions.h"
#include "Core.Serialize/ISerializer.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class TextSerializerException : public SerializeException {
public:
    TextSerializerException(const char* what) : SerializeException(what) {}
};
//----------------------------------------------------------------------------
class TextSerializer : public ISerializer {
public:
    TextSerializer(RTTI::MetaTransaction *transaction);
    virtual ~TextSerializer();

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
