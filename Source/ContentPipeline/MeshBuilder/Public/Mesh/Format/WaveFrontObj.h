#pragma once

#include "MeshBuilder_fwd.h"

#include "Container/Vector.h"
#include "IO/FileSystem_fwd.h"
#include "Memory/RefPtr.h"

namespace PPE {
class IBufferedStreamWriter;
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_MESHBUILDER_API FWaveFrontObj {
public:
    static const FExtname& Extname();

    static bool Load(FGenericMesh* dst, const FFilename& filename);
    static bool Load(FGenericMesh* dst, const FFilename& filename, const FStringView& content);
    static bool Load(FGenericMesh* dst, const FFilename& filename, IBufferedStreamReader* reader);

    static bool Save(const FGenericMesh* src, const FFilename& filename);
    static bool Save(const FGenericMesh* src, const FFilename& filename, IBufferedStreamWriter* writer);

private:
    FWaveFrontObj() = delete;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
