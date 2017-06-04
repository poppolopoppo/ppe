#include "stdafx.h"

#include "Core.Lattice/GenericMaterial.h"
#include "Core.Lattice/GenericMesh.h"
#include "Core.Lattice/GenericMeshHelpers.h"
#include "Core.Lattice/WaveFrontObj.h"

#include "Core/Color/Color.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Maths/ScalarBoundingBox.h"
#include "Core/Maths/ScalarBoundingBox.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Thread/Task.h"

namespace Core {
namespace ContentGenerator {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void Test_LoadWaveFrontObj(const FFilename& filename) {
    Lattice::FGenericMesh mesh;
    if (!Lattice::FWaveFrontObj::Load(&mesh, filename))
        AssertNotReached();

    const size_t indexCount = mesh.IndexCount();
    const size_t vertxCount = mesh.VertexCount();

    mesh.CleanAndOptimize();

    LOG(Info, L"[Lattice] '{0}' {1} indices, {2} vertices (cleaned {3} indices & {4} vertices)",
        filename, mesh.IndexCount(), mesh.VertexCount(), indexCount - mesh.IndexCount(), vertxCount - mesh.VertexCount());

    const FFilename outfname = filename.WithAppendBasename(L".clean");

    if (!Lattice::FWaveFrontObj::Save(&mesh, outfname))
        AssertNotReached();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Lattice() {
    const FFilename inputs[] = {
        L"Data:/Models/Sphere.obj",
        L"Data:/Models/monkey.obj",
        L"Data:/Models/Cerberus.obj",
    };

    parallel_for(std::begin(inputs), std::end(inputs), [](const FFilename& fname) {
        Test_LoadWaveFrontObj(fname);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentGenerator
} //!namespace Core

