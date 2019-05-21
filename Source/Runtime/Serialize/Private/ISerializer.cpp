#include "stdafx.h"

#include "ISerializer.h"

#include "SerializeExceptions.h"

#include "TransactionLinker.h"

#include "Binary/BinarySerializer.h"
#include "Json/JsonSerializer.h"
#include "Text/TextSerializer.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformDialog.h"
#include "IO/Extname.h"
#include "IO/StringBuilder.h"
#include "Memory/MemoryProvider.h"

namespace PPE {
namespace Serialize {
EXTERN_LOG_CATEGORY(PPE_SERIALIZE_API, Serialize);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTIONS
static NO_INLINE FPlatformDialog::EResult ShowSerializeException_(const FTransactionLinker& linker, const FSerializeException& e) {
    UNUSED(linker);
    LOG(Serialize, Error, L"caught exception while serializing '{0}':\n{1}", linker.Filename(), e);

    FWStringBuilder text;
#   ifdef PLATFORM_WINDOWS
    text << FTextFormat::Crlf;
#   endif
#   if USE_PPE_EXCEPTION_DESCRIPTION
    e.Description(text);
#   else
    text << MakeCStringView(e.What());
#   endif

    return FPlatformDialog::AbortRetryIgnore(
        text.Written(),
        L"caught exception while serializing");
}
#endif //!#if USE_PPE_EXCEPTIONS
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ISerializer::Deserialize(
    const ISerializer& serializer,
    const TMemoryView<const u8>& rawData,
    FTransactionLinker* linker ) {
    Assert(not rawData.empty());

    FMemoryViewReader reader(rawData);
    serializer.Deserialize(reader, linker);
}
//----------------------------------------------------------------------------
bool ISerializer::InteractiveDeserialize(
    const ISerializer& serializer,
    IStreamReader& input, FTransactionLinker* linker) {
    PPE_TRY {
        serializer.Deserialize(input, linker);
        return true;
    }
    PPE_CATCH(const FSerializeException& e)
    PPE_CATCH_BLOCK({
        switch (ShowSerializeException_(*linker, e)) {
        case FPlatformDialog::Abort:
            PPE_THROW_VOID();
        case FPlatformDialog::Retry:
            return false;
        case FPlatformDialog::Ignore:
            return true;
        default:
            AssertNotReached();
        }
    })
}
//----------------------------------------------------------------------------
FExtname ISerializer::Extname(ESerializeFormat fmt) {
    switch (fmt) {
    case PPE::Serialize::ESerializeFormat::Binary:
        return FBinarySerializer::Extname();
    case PPE::Serialize::ESerializeFormat::Json:
        return FJsonSerializer::Extname();
    case PPE::Serialize::ESerializeFormat::Markup:
        AssertNotImplemented(); // #TODO
    case PPE::Serialize::ESerializeFormat::Script:
        return FTextSerializer::Extname();
    default:
        AssertNotReached();
    }
}
//----------------------------------------------------------------------------
PSerializer ISerializer::FromExtname(const FExtname& ext) {
    Assert(not ext.empty());

    if (ext == FBinarySerializer::Extname())
        return FBinarySerializer::Get();
    else if (ext == FJsonSerializer::Extname())
        return FJsonSerializer::Get();
    else if (ext == FTextSerializer::Extname())
        return FTextSerializer::Get();

    return PSerializer(); // unknown extension
}
//----------------------------------------------------------------------------
PSerializer ISerializer::FromFormat(ESerializeFormat fmt) {
    switch (fmt) {
    case PPE::Serialize::ESerializeFormat::Binary:
        return FBinarySerializer::Get();
    case PPE::Serialize::ESerializeFormat::Json:
        return FJsonSerializer::Get();
    case PPE::Serialize::ESerializeFormat::Markup:
        AssertNotImplemented(); // #TODO
    case PPE::Serialize::ESerializeFormat::Script:
        return FTextSerializer::Get();
    default:
        AssertNotReached();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
