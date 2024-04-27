#pragma once

#include "MeshBuilder_fwd.h"

#include "Mesh/MeshFormat.h"

#include "Container/Appendable.h"
#include "IO/FileSystem_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IMeshBuilderService {
public:
    virtual ~IMeshBuilderService() = default;

public: // Interface:

    /** Mesh formats **/

    virtual size_t AllMeshFormats(TAppendable<ContentPipeline::UMeshFormat> outMeshFormat) const = 0;
    virtual void RegisterMeshFormat(const FExtname& extname, ContentPipeline::UMeshFormat&& impl) NOEXCEPT = 0;

    NODISCARD virtual ContentPipeline::UMeshFormat MeshFormat(const FExtname& extname) const NOEXCEPT = 0;

public: // Public helpers:

    NODISCARD PPE_MESHBUILDER_API bool ExportGenericMesh(const FFilename& sourceFile, const ContentPipeline::FGenericMesh& mesh) const;

    NODISCARD PPE_MESHBUILDER_API ContentPipeline::FMeshBuilderResult ImportGenericMesh(const FFilename& sourceFile, const ContentPipeline::FMeshBuilderSettings& settings) const;

public:
    static PPE_MESHBUILDER_API void MakeDefault(UMeshBuilderService* meshBuilder);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
