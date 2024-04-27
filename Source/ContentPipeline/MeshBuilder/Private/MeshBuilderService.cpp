// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "MeshBuilderService.h"

#include "Mesh/Format/PolygonFileFormat.h"
#include "Mesh/Format/WaveFrontObj.h"

#include "Container/HashMap.h"
#include "Diagnostic/FeedbackContext.h"
#include "Diagnostic/Logger.h"
#include "IO/BufferedStream.h"
#include "IO/Extname.h"
#include "IO/ObservableStream.h"
#include "Maths/ScalarBoundingBox.h"
#include "Thread/ThreadSafe.h"
#include "VirtualFileSystem_fwd.h"

#include "Mesh/GenericMesh.h"
#include "Mesh/GenericMeshHelpers.h"

namespace PPE {
//-- --------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace ContentPipeline {
//----------------------------------------------------------------------------
class FDefaultMeshBuilderService_ final : public IMeshBuilderService {
public:
    FDefaultMeshBuilderService_()
    // register default mesh formats here:
    :   _meshFormats({
            // very basic DCC mesh formats
            { FPolygonFileFormat::Extname(), UMeshFormat::Make<FPolygonFileFormat>() },
            { FWaveFrontObj::Extname(), UMeshFormat::Make<FWaveFrontObj>() },
        })
    {}

public: // ITextureService

    /** Image formats **/

    virtual size_t AllMeshFormats(TAppendable<UMeshFormat> outMeshFormats) const override {
        u32 numMeshFormats = 0;
        for (const TPair<const FExtname, UMeshFormat>& it : *_meshFormats.LockShared()) {
            outMeshFormats.emplace_back(it.second);
            numMeshFormats++;
        }
        return numMeshFormats;
    }

    virtual void RegisterMeshFormat(const FExtname& extname, UMeshFormat&& impl) NOEXCEPT override {
        _meshFormats.LockExclusive()->emplace(extname, std::move(impl));
    }

    NODISCARD virtual UMeshFormat MeshFormat(const FExtname& extname) const NOEXCEPT override {
        const auto shared = _meshFormats.LockShared();

        if (const auto it = shared->find(extname); it != shared->end())
            return it->second;

        return Default;
    }

private:
    TThreadSafe<HASHMAP(MeshBuilder, FExtname, UMeshFormat), EThreadBarrier::RWLock> _meshFormats;
};
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void IMeshBuilderService::MakeDefault(UMeshBuilderService* mesh) {
    mesh->create<ContentPipeline::FDefaultMeshBuilderService_>();
}
//----------------------------------------------------------------------------
// Export generic mesh to file:
//----------------------------------------------------------------------------
bool IMeshBuilderService::ExportGenericMesh(const FFilename& sourceFile, const ContentPipeline::FGenericMesh& mesh) const {
    MEMORYDOMAIN_THREAD_SCOPE(MeshBuilder);

    using namespace ContentPipeline;
    Assert_NoAssume(not sourceFile.empty());
    Assert_NoAssume(not mesh.empty());

    const UMeshFormat format = MeshFormat(sourceFile.Extname());
    if (Unlikely(not format)) {
        PPE_SLOG(MeshBuilder, Error, "unknown mesh format", {
            {"Format", sourceFile.Extname().MakeLiteral()},
            {"SourceFile", Opaq::Format(sourceFile)},
        });
        return false;
    }

    if (const UStreamWriter writer = VFS_OpenBinaryWritable(sourceFile))
        return format->ExportGenericMesh(writer.get(), mesh);

    return false;
}
//----------------------------------------------------------------------------
// Import generic mesh from file:
//----------------------------------------------------------------------------
ContentPipeline::FMeshBuilderResult IMeshBuilderService::ImportGenericMesh(const FFilename& sourceFile, const ContentPipeline::FMeshBuilderSettings& settings) const {
    MEMORYDOMAIN_THREAD_SCOPE(MeshBuilder);

    using namespace ContentPipeline;

    const UMeshFormat format = MeshFormat(sourceFile.Extname());
    if (Unlikely(not format)) {
        PPE_SLOG(MeshBuilder, Error, "unknown mesh format", {
            {"Format", sourceFile.Extname().MakeLiteral()},
            {"SourceFile", Opaq::Format(sourceFile)},
        });
        return std::nullopt;
    }

    const UStreamReader reader = VFS_OpenBinaryReadable(sourceFile);
    if (not reader)
        return std::nullopt;

    FMeshBuilderResult result = UsingStreamWithProgress(*reader,
        INLINE_FORMAT(50 + FileSystem::MaxPathLength, "Loading mesh: \"{}\"", sourceFile),
        [&format](TPtrRef<IStreamReader> rd) {
            return format->ImportGenericMesh(rd);
        });
    if (not result)
        return std::nullopt;

    FFeedbackProgressBar pbar{
        INLINE_FORMAT(50 + FileSystem::MaxPathLength, "Building mesh: \"{}\"", sourceFile),
        9};

    PPE_LOG_CHECKEX(MeshBuilder, std::nullopt, result->Validate());
    pbar.Inc();

    constexpr size_t defaultIndex = 0;

    if (settings.Transform.has_value())
        Transform(*result, defaultIndex, *settings.Transform);
    pbar.Inc();

    FNormals3f normals = result->Normal3f_IFP(defaultIndex);
    if (ERecomputeMode_Apply(settings.RecomputeNormals, not normals)) {
        PPE_LOG_CHECKEX(MeshBuilder, std::nullopt, ComputeNormals(*result, defaultIndex));
        Assert_NoAssume(result->Validate());
        normals = result->Normal3f(0);
    }
    else if (normals and ERecomputeMode::Remove == settings.RecomputeNormals) {
        result->RemoveSubPart(normals);
    }
    pbar.Inc();

    FTangents3f tangents3 = result->Tangent3f_IFP(defaultIndex);
    FBinormals3f binormal3 = result->Binormal3f_IFP(defaultIndex);
    FTangents4f tangents4 = result->Tangent4f_IFP(defaultIndex);
    bool hasTangents = (tangents3 || binormal3 || tangents4);
    if (normals && ERecomputeMode_Apply(settings.RecomputeTangentSpace, not hasTangents)) {
        PPE_LOG_CHECKEX(MeshBuilder, std::nullopt, ComputeTangentSpace(*result, defaultIndex));
        Assert_NoAssume(result->Validate());
        hasTangents = true;
    }
    else if (hasTangents && ERecomputeMode::Remove == settings.RecomputeTangentSpace) {
        if (tangents3)
            result->RemoveSubPart(tangents3);
        if (binormal3)
            result->RemoveSubPart(binormal3);
        if (tangents4)
            result->RemoveSubPart(tangents4);
    }
    pbar.Inc();

    if (normals && hasTangents && settings.EncodeTangentSpaceToQuaternion) {
        PPE_LOG_CHECKEX(MeshBuilder, std::nullopt, TangentSpaceToQuaternion(*result, defaultIndex, true));
        Assert_NoAssume(result->Validate());
    }
    pbar.Inc();

    if (settings.MergeCloseVertices || settings.RemoveZeroAreaTriangles) {
        const FAabb3f meshBounds = ComputeBounds(*result, defaultIndex);
        const float mergeDistance = (0.005f * meshBounds.Extents().MaxComponent());

        if (settings.MergeCloseVertices) {
            MergeCloseVertices(*result, defaultIndex, mergeDistance);
            Assert_NoAssume(result->Validate());
        }

        if (settings.RemoveZeroAreaTriangles) {
            RemoveZeroAreaTriangles(*result, defaultIndex, (mergeDistance * mergeDistance) / 2.f);
            Assert_NoAssume(result->Validate());
        }
    }
    pbar.Inc();

    if (settings.RemoveUnusedVertices) {
        RemoveUnusedVertices(*result);
        Assert_NoAssume(result->Validate());
    }
    pbar.Inc();

    if (settings.OptimizeIndicesOrder) {
        OptimizeIndicesOrder(*result);
        Assert_NoAssume(result->Validate());
    }
    pbar.Inc();

    if (settings.OptimizeVerticesOrder) {
        OptimizeVerticesOrder(*result);
        Assert_NoAssume(result->Validate());
    }
    pbar.Inc();

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
