#pragma once

#include "MeshBuilder_fwd.h"

#include "IO/FileSystem_fwd.h"
#include "IO/Stream_fwd.h"
#include "Maths/Transform.h"

#include "Meta/AutoEnum.h"
#include "Meta/Optional.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FMeshBuilderResult = Meta::TOptional<FGenericMesh>;
//----------------------------------------------------------------------------
PPE_DEFINE_AUTOENUM(ERecomputeMode, u8,
    Ignore = 0,
    Force,
    IfMissing,
    Remove,
    Unknown = Ignore)
inline CONSTEXPR bool ERecomputeMode_Apply(ERecomputeMode mode, bool missing) {
    return (mode == ERecomputeMode::Force || (
            mode == ERecomputeMode::IfMissing && missing));
}
//----------------------------------------------------------------------------
struct FMeshBuilderSettings {
    Meta::TOptional<FTransform> Transform;

    bool EncodeTangentSpaceToQuaternion{ false };
    bool MergeCloseVertices{ false };
    bool MergeDuplicateVertices{ false };
    bool OptimizeIndicesOrder{ false };
    bool OptimizeVerticesOrder{ false };
    bool RemoveUnusedVertices{ false };
    bool RemoveZeroAreaTriangles{ false };

    float Epsilon{ Epsilon_v<float> };

    ERecomputeMode RecomputeNormals{ Default };
    ERecomputeMode RecomputeTangentSpace{ Default };
};
//----------------------------------------------------------------------------
// Should be state-less8
class IMeshFormat : Meta::FNonCopyableNorMovable {
public:
    virtual ~IMeshFormat() = default;

    NODISCARD virtual const FExtname& MeshExtname() const NOEXCEPT = 0;

    NODISCARD virtual bool ExportGenericMesh(IStreamWriter* output, const FGenericMesh& mesh) const = 0;

    NODISCARD virtual FMeshBuilderResult ImportGenericMesh(const FRawMemoryConst& memory) const = 0;
    NODISCARD virtual FMeshBuilderResult ImportGenericMesh(IStreamReader& input) const = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
