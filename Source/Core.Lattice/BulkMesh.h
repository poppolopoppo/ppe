#pragma once

#include "Core.Lattice/Lattice.h"

#include "Core/Container/Vector.h"
#include "Core/IO/FileSystem_fwd.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
class IBufferedStreamReader;
class IBufferedStreamWriter;
namespace Lattice {
FWD_REFPTR(GenericMaterial);
FWD_REFPTR(GenericMesh);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_LATTICE_API FBulkMesh {
public:
    static const FExtname& Ext;

    static bool Load(FGenericMesh* dst, const FFilename& filename);
    static bool Load(FGenericMesh* dst, const FFilename& filename, const TMemoryView<const u8>& content);
    static bool Load(FGenericMesh* dst, const FFilename& filename, IBufferedStreamReader* reader);

    static bool Save(const FGenericMesh* src, const FFilename& filename);
    static bool Save(const FGenericMesh* src, const FFilename& filename, IBufferedStreamWriter* writer);

private:
    FBulkMesh() = delete;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
