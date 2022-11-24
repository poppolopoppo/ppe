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
FFATSerializer::FFATSerializer() {}
//----------------------------------------------------------------------------
FFATSerializer::~FFATSerializer() = default;
//----------------------------------------------------------------------------
void FFATSerializer::Deserialize(IStreamReader& input, FTransactionLinker* linker) const {
    Assert(linker);

    Unused(input);
    Unused(linker);
    AssertNotImplemented();
    /*
    UsingBufferedStream(&input, [linker](IBufferedStreamReader* buffered) {
        FBinaryFormatReader reader;
        reader.Read(*buffered, *linker);
    });
    */
}
//----------------------------------------------------------------------------
void FFATSerializer::Serialize(const FTransactionSaver& saver, IStreamWriter* output) const {
    Assert(output);

    Unused(saver);
    Unused(output);
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
PSerializer FFATSerializer::Get() {
    return PSerializer::Make<FFATSerializer>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
