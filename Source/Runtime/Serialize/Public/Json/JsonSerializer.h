#pragma once

#include "Serialize_fwd.h"

#include "ISerializer.h"

#include "Container/Vector.h"
#include "SerializeExceptions.h"

#include "RTTI_fwd.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FJsonSerializerException : public FSerializeException {
public:
    FJsonSerializerException(const char* what) : FSerializeException(what) {}

    PPE_DEFAULT_EXCEPTION_DESCRIPTION(FJsonSerializerException);
};
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FJsonSerializer final : public ISerializer {
public:
    virtual ~FJsonSerializer();

    static FExtname Extname();
    static PSerializer Get();

public: // ISerializer
    virtual void Deserialize(IStreamReader& input, FTransactionLinker* linker) const override final;
    virtual void Serialize(const FTransactionSaver& saver, IStreamWriter* output) const override final;

private:
    friend struct TInSituPtr<ISerializer>;

    explicit FJsonSerializer(bool minify = true);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_SERIALIZE_API void RTTI_to_Json(const RTTI::FAtom& atom, class FJson* dst);
PPE_SERIALIZE_API void RTTI_to_Json(const RTTI::FMetaObject& obj, class FJson* dst);
PPE_SERIALIZE_API void RTTI_to_Json(const RTTI::PMetaObject& pobj, class FJson* dst);
PPE_SERIALIZE_API void RTTI_to_Json(const FTransactionSaver& saved, class FJson* dst);
PPE_SERIALIZE_API void RTTI_to_Json(const TMemoryView<const RTTI::SMetaObject>& objs, class FJson* dst);
PPE_SERIALIZE_API void RTTI_to_Json(const TMemoryView<const RTTI::SCMetaTransaction>& mnamespace, class FJson* dst);
//----------------------------------------------------------------------------
PPE_SERIALIZE_API void RTTI_to_Json(const RTTI::FMetaClass& mclass, class FJson* dst);
PPE_SERIALIZE_API void RTTI_to_Json(const RTTI::FMetaEnum& menum, class FJson* dst);
PPE_SERIALIZE_API void RTTI_to_Json(const RTTI::FMetaFunction& func, class FJson* dst);
PPE_SERIALIZE_API void RTTI_to_Json(const RTTI::FMetaProperty& prop, class FJson* dst);
PPE_SERIALIZE_API void RTTI_to_Json(const RTTI::FMetaModule& mod, class FJson* dst);
PPE_SERIALIZE_API void RTTI_to_Json(const RTTI::PTypeTraits& traits, class FJson* dst);
//----------------------------------------------------------------------------
PPE_SERIALIZE_API bool Json_to_RTTI(const class FJson& src, FTransactionLinker* link);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
