#pragma once

#include "Lattice.h"

#include "Container/Vector.h"
#include "IO/FileSystem_fwd.h"
#include "Memory/RefPtr.h"

namespace PPE {
class IBufferedStreamWriter;
namespace Lattice {
FWD_REFPTR(GenericMaterial);
FWD_REFPTR(GenericMesh);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_LATTICE_API FWaveFrontObj {
public:
    static const FExtname& Ext;

    static bool Load(FGenericMesh* dst, const FFilename& filename);
    static bool Load(FGenericMesh* dst, const FFilename& filename, const FStringView& content);

    static bool Save(const FGenericMesh* src, const FFilename& filename);
    static bool Save(const FGenericMesh* src, const FFilename& filename, IBufferedStreamWriter* writer);

private:
    FWaveFrontObj() = delete;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace PPE
