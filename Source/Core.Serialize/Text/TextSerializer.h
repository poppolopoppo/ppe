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
    TextSerializer();
    virtual ~TextSerializer();

    using ISerializer::Deserialize;
    using ISerializer::Serialize;

    virtual void Deserialize(RTTI::MetaTransaction* transaction, const MemoryView<const u8>& input, const wchar_t *sourceName = nullptr) override;
    virtual void Serialize(IStreamWriter* output, const RTTI::MetaTransaction* transaction) override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
