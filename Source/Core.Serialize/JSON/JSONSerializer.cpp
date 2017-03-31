#include "stdafx.h"

#include "JSONSerializer.h"

#include "JSON.h"

#include "Core.RTTI/MetaAtom.h"
#include "Core.RTTI/MetaClass.h"
#include "Core.RTTI/MetaObject.h"
#include "Core.RTTI/MetaProperty.h"
#include "Core.RTTI/MetaTransaction.h"

#include "Core/Container/HashSet.h"
#include "Core/Container/Vector.h"
#include "Core/IO/Format.h"
#include "Core/IO/StreamProvider.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FJSONSerializer::FJSONSerializer(bool minify/* = true */) : _minify(minify) {}
//----------------------------------------------------------------------------
FJSONSerializer::~FJSONSerializer() {}
//----------------------------------------------------------------------------
void FJSONSerializer::Deserialize(RTTI::FMetaTransaction* transaction, IStreamReader* input, const wchar_t *sourceName/* = nullptr */) {
    Assert(transaction);
    Assert(input);

    FJSON json;
    if (not FJSON::Load(&json, FFilename(MakeStringView(sourceName, Meta::noinit_tag{})), input))
        CORE_THROW_IT(FJSONSerializerException("failed to parse JSON document"));

    VECTOR(Serialize, RTTI::PMetaAtom) parsed;
    JSONtoRTTI(parsed, json);

    for (const RTTI::PMetaAtom& atom : parsed)
        transaction->Add(atom->Cast<RTTI::PMetaObject>()->Wrapper().get());
}
//----------------------------------------------------------------------------
void FJSONSerializer::Serialize(IStreamWriter* output, const RTTI::FMetaTransaction* transaction) {
    Assert(output);
    Assert(transaction);

    VECTOR(Serialize, RTTI::PMetaAtom) atoms;
    atoms.reserve(transaction->size());
    for (const RTTI::PMetaObject& object : transaction->MakeView())
        atoms.emplace_back(RTTI::MakeAtom(object));

    FJSON json;
    RTTItoJSON(json, atoms);

    FStreamWriterOStream oss(output);
    json.ToStream(oss, _minify);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
