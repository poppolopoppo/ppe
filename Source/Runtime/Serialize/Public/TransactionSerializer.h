#pragma once

#include "Serialize_fwd.h"

#include "MetaObject.h"
#include "RTTI_fwd.h"
#include "RTTI/Macros.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ETransactionSerializerFlags : u32 {
    None                = 0,
    AutoBuild           = 1<<0,
    AutoMount           = 1<<1,
    AutoImport          = 1<<2,

    Default             = AutoBuild|AutoMount|AutoImport
};
ENUM_FLAGS(ETransactionSerializerFlags);
RTTI_ENUM_HEADER(PPE_SERIALIZE_API, ETransactionSerializerFlags);
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FTransactionSerializer : RTTI::FMetaObject {
    RTTI_CLASS_HEADER(PPE_SERIALIZE_API, FTransactionSerializer, RTTI::FMetaObject);
public:
    FTransactionSerializer(RTTI::FConstructorTag);
    ~FTransactionSerializer();

    FTransactionSerializer(const FTransactionSerializer&) = delete;
    FTransactionSerializer& operator =(const FTransactionSerializer&) = delete;

    const RTTI::FName& Namespace() const { return _namespace; }
    ETransactionSerializerFlags Flags() const { return _flags; }

    bool HasAutoBuild() const { return (_flags ^ ETransactionSerializerFlags::AutoBuild); }
    bool HasAutoMount() const { return (_flags ^ ETransactionSerializerFlags::AutoMount); }
    bool HasAutoImport() const { return (_flags ^ ETransactionSerializerFlags::AutoImport); }

    const RTTI::PMetaTransaction& Exported() const { return _exported; }
    TMemoryView<const RTTI::PMetaTransaction> Importeds() const { return _importeds; }

private:
    RTTI::FName _namespace;
    ETransactionSerializerFlags _flags;

    RTTI::PMetaTransaction _exported;
    VECTORINSITU(Transient, RTTI::PMetaTransaction, 4) _importeds;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
