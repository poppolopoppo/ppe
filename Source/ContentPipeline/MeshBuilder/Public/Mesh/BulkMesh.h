#pragma once

#include "MeshBuilder_fwd.h"

#include "IO/FileSystem_fwd.h"
#include "Memory/MemoryView.h"
#include "Memory/RefPtr.h"

namespace PPE {
class IBufferedStreamReader;
class IBufferedStreamWriter;
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FBulkMesh {
public:
    static PPE_MESHBUILDER_API FExtname Extname();

    static PPE_MESHBUILDER_API bool Load(FGenericMesh* dst, const FFilename& filename);
    static PPE_MESHBUILDER_API bool Load(FGenericMesh* dst, const FFilename& filename, const TMemoryView<const u8>& content);
    static PPE_MESHBUILDER_API bool Load(FGenericMesh* dst, const FFilename& filename, IBufferedStreamReader* reader);

    static PPE_MESHBUILDER_API bool Save(const FGenericMesh* src, const FFilename& filename);
    static PPE_MESHBUILDER_API bool Save(const FGenericMesh* src, const FFilename& filename, IBufferedStreamWriter* writer);

private:
    FBulkMesh() = delete;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
