// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Build/FileNode.h"

#include "BuildContext.h"

#include "RTTI/Macros-impl.h"

#include "VirtualFileSystem_fwd.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(BuildGraph, FFileNode, Concrete)
RTTI_PROPERTY_PRIVATE_FIELD(_filename)
RTTI_PROPERTY_PRIVATE_READONLY(_timestamp)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_VERIFY_PREDICATES
void FFileNode::RTTI_VerifyPredicates() const PPE_THROW() {
    RTTI_parent_type::RTTI_VerifyPredicates();
    RTTI_VerifyPredicate(not _filename.empty());
}
#endif
//----------------------------------------------------------------------------
FFileNode::FFileNode() NOEXCEPT
{}
//----------------------------------------------------------------------------
FFileNode::FFileNode(const FFilename& filename) NOEXCEPT {
    SetFilename(filename);
}
//----------------------------------------------------------------------------
EBuildResult FFileNode::Scan(FScanContext&) {
    return EBuildResult::UpToDate;
}
//----------------------------------------------------------------------------
EBuildResult FFileNode::Import(FBuildContext&) {
    return EBuildResult::Built; // always call Process()
}
//----------------------------------------------------------------------------
EBuildResult FFileNode::Process(FBuildContext&) {
    FTimestamp lastModified;
    if (VFS_FileLastModified(&lastModified, _filename)) {
        if (lastModified != _timestamp) {
            _timestamp = lastModified;
            return EBuildResult::Built;
        }
        else {
            return EBuildResult::UpToDate;
        }
    }
    else {
        _timestamp = 0;
        return EBuildResult::Failed;
    }
}
//----------------------------------------------------------------------------
EBuildResult FFileNode::Clean(FCleanContext&) {
    return EBuildResult::UpToDate; // nothing to do
}
//----------------------------------------------------------------------------
void FFileNode::SetFilename(const FFilename& filename) NOEXCEPT {
    Assert(not filename.empty());

    _filename = filename;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
