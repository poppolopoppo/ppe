#include "stdafx.h"

/* %_NOCOMMIT% TODO

#include "GenericMaterial.h"
#include "GenericMesh.h"
#include "GenericMeshHelpers.h"
#include "WaveFrontObj.h"

#include "Color/Color.h"
#include "Diagnostic/Logger.h"
#include "IO/VirtualFileSystem.h"
#include "Maths/ScalarBoundingBox.h"
#include "Maths/ScalarBoundingBox.h"
#include "Maths/ScalarVector.h"
#include "Thread/Task.h"

*/

namespace PPE {
namespace Test {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
/* %_NOCOMMIT% TODO
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
*/
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Lattice() {
    /* %_NOCOMMIT% TODO
    const FFilename inputs[] = {
        L"Data:/Models/Sphere.obj",
        L"Data:/Models/monkey.obj",
        L"Data:/Models/Cerberus.obj",
    };

    parallel_for(std::begin(inputs), std::end(inputs), [](const FFilename& fname) {
        Test_LoadWaveFrontObj(fname);
    });
*/
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
