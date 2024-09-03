// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Binary/BinarySerializer.h"

// those headers are private :
#include "Binary/BinaryFormatReader.h"
#include "Binary/BinaryFormatWriter.h"

#include "IO/BufferedStream.h"
#include "IO/ConstNames.h"
#include "IO/Extname.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FBinarySerializer::Deserialize(const FDeserializeContext& ctx, IStreamReader& input, FTransactionLinker* linker) const {
    Assert(linker);
    Unused(ctx);

    UsingBufferedStream(&input, [linker](IBufferedStreamReader* buffered) {
        FBinaryFormatReader reader;
        reader.Read(*buffered, *linker);
    });
}
//----------------------------------------------------------------------------
void FBinarySerializer::Serialize(const FSerializeContext& ctx, const FTransactionSaver& saver, IStreamWriter* output) const {
    Assert(output);
    Unused(ctx);

    FBinaryFormatWriter writer;
    writer.Append(saver);

    UsingBufferedStream(output, [&writer](IBufferedStreamWriter* buffered) {
        writer.Finalize(*buffered);
    });
}
//----------------------------------------------------------------------------
FExtname FBinarySerializer::Extname() {
    return FFSConstNames::Bnx();
}
//----------------------------------------------------------------------------
USerializer FBinarySerializer::Get() {
    return FBinarySerializer();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
