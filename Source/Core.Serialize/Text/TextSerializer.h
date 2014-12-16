#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/ISerializer.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class TextSerializer : public ISerializer {
public:
    TextSerializer(RTTI::MetaTransaction *transaction);
    virtual ~TextSerializer();

    virtual RTTI::MetaTransaction *Transaction() const override { return _transaction; }

    virtual void Deserialize(VECTOR(Transaction, RTTI::PMetaAtom)& atoms, const RAWSTORAGE(Serializer, u8)& input, const char *sourceName = nullptr) override;
    virtual void Serialize(RAWSTORAGE(Serializer, u8)& output, const MemoryView<const RTTI::PCMetaAtom>& atoms) override;

private:
    RTTI::PMetaTransaction _transaction;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
