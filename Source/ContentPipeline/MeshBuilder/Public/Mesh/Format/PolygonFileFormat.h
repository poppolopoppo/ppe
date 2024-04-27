#pragma once

#include "MeshBuilder_fwd.h"

#include "Mesh/MeshFormat.h"

#include "IO/FileSystem_fwd.h"
#include "IO/Stream_fwd.h"

namespace PPE {
class IBufferedStreamWriter;
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FPolygonFileFormat final : public IMeshFormat {
public:
    NODISCARD PPE_MESHBUILDER_API static const FExtname& Extname() NOEXCEPT;

    NODISCARD PPE_MESHBUILDER_API static bool Load(FGenericMesh* dst, const FFilename& filename);
    NODISCARD PPE_MESHBUILDER_API static bool Load(FGenericMesh* dst, const FFilename& filename, const FStringView& content);
    NODISCARD PPE_MESHBUILDER_API static bool Load(FGenericMesh* dst, const FFilename& filename, IBufferedStreamReader& reader);

    NODISCARD PPE_MESHBUILDER_API static bool Save(const FGenericMesh& src, const FFilename& filename);
    NODISCARD PPE_MESHBUILDER_API static bool Save(const FGenericMesh& src, const FFilename& filename, IBufferedStreamWriter& writer);

public:
    NODISCARD virtual const FExtname& MeshExtname() const NOEXCEPT override { return Extname(); }

    NODISCARD PPE_MESHBUILDER_API virtual bool ExportGenericMesh(IStreamWriter* output, const FGenericMesh& mesh) const override;

    NODISCARD PPE_MESHBUILDER_API virtual FMeshBuilderResult ImportGenericMesh(const FRawMemoryConst& memory) const override;
    NODISCARD PPE_MESHBUILDER_API virtual FMeshBuilderResult ImportGenericMesh(IStreamReader& input) const override;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
