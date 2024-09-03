// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "FAT/FATSerializer.h"

#include "IO/BufferedStream.h"
#include "IO/ConstNames.h"
#include "IO/Extname.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FFATSerializer::Deserialize(const FDeserializeContext& ctx, IStreamReader& input, FTransactionLinker* linker) const {
    Assert(linker);

    Unused(ctx, input, linker);
    AssertNotImplemented();
    /*
    UsingBufferedStream(&input, [linker](IBufferedStreamReader* buffered) {
        FBinaryFormatReader reader;
        reader.Read(*buffered, *linker);
    });
    */
}
//----------------------------------------------------------------------------
void FFATSerializer::Serialize(const FSerializeContext& ctx, const FTransactionSaver& saver, IStreamWriter* output) const {
    Assert(output);

    Unused(ctx, saver, output);
    AssertNotImplemented();
    /*
    FBinaryFormatWriter writer;
    writer.Append(saver);

    UsingBufferedStream(output, [&writer](IBufferedStreamWriter* buffered) {
        writer.Finalize(*buffered);
    });
    */
}
//----------------------------------------------------------------------------
FExtname FFATSerializer::Extname() {
    return FFSConstNames::Z(); // #TODO
}
//----------------------------------------------------------------------------
USerializer FFATSerializer::Get() {
    return FFATSerializer();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
