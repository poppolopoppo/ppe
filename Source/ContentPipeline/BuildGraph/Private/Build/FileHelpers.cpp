// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Build/FileHelpers.h"

#include "Build/FileNode.h"

#include "Container/HashSet.h"
#include "Container/RingBuffer.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool FindFilesRec_(
    const TMemoryView<const PBuildNode>& deps,
    FFileNodeList& files,
    HASHSET(BuildGraph, FBuildNode*)& visited);
//----------------------------------------------------------------------------
static bool FindFilesRec_(
    FBuildNode& n,
    FFileNodeList& files,
    HASHSET(BuildGraph, FBuildNode*)& visited) {

    const bool recurse = (not visited.insert_ReturnIfExists(&n));

    bool result = false;
    if (FFileNode* const f = RTTI::Cast<FFileNode>(&n)) {
        result = true;
        if (recurse)
            Emplace_Back(files, f);
    }

    return (result || (recurse && (
        FindFilesRec_(n.StaticDeps(), files, visited) ||
        FindFilesRec_(n.RuntimeDeps(), files, visited) ||
        FindFilesRec_(n.DynamicDeps(), files, visited) )) );
}
//----------------------------------------------------------------------------
static bool FindFilesRec_(
    const TMemoryView<const PBuildNode>& deps,
    FFileNodeList& files,
    HASHSET(BuildGraph, FBuildNode*)& visited) {
    bool result = false;
    for (const PBuildNode& n : deps)
        result |= FindFilesRec_(*n, files, visited);

    return result;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FillFileNodeList(FFileNodeList* pFiles, FBuildNode& root) {
    Assert(pFiles);

    HASHSET(BuildGraph, FBuildNode*) visited;
    visited.reserve(pFiles->size());

    for (const SFileNode& f : *pFiles)
        visited.insert_AssertUnique(f.get());

    return (not visited.insert_ReturnIfExists(&root)
        ? FindFilesRec_(root, *pFiles, visited)
        : false );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
