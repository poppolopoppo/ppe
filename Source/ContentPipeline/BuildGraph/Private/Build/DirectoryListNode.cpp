#include "stdafx.h"

#include "Build/DirectoryListNode.h"

#include "BuildContext.h"
#include "BuildGraph.h"
#include "Build/FileNode.h"

#include "RTTI/Macros-impl.h"

#include "IO/FileSystemProperties.h"
#include "IO/regexp.h"
#include "VirtualFileSystem_fwd.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(BuildGraph, FDirectoryListNode, Concrete)
RTTI_PROPERTY_PRIVATE_READONLY(_path)
RTTI_PROPERTY_PRIVATE_READONLY(_pattern)
RTTI_PROPERTY_PRIVATE_READONLY(_recursive)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_VERIFY_PREDICATES
void FDirectoryListNode::RTTI_VerifyPredicates() const PPE_THROW() {
    RTTI_parent_type::RTTI_VerifyPredicates();
    RTTI_VerifyPredicate(not _path.empty());
    RTTI_VerifyPredicate(_pattern.empty() or FWRegexp::ValidateSyntax(_pattern));
}
#endif
//----------------------------------------------------------------------------
FDirectoryListNode::FDirectoryListNode() NOEXCEPT
:   _recursive(false)
{}
//----------------------------------------------------------------------------
FDirectoryListNode::FDirectoryListNode(const FDirpath& path) NOEXCEPT
:   FDirectoryListNode() {
    SetPath(path);
}
//----------------------------------------------------------------------------
FDirectoryListNode::FDirectoryListNode(const FDirpath& path, FWString&& pattern) NOEXCEPT
:   FDirectoryListNode() {
    SetPath(path);
    SetPattern(std::move(pattern));
}
//----------------------------------------------------------------------------
EBuildResult FDirectoryListNode::Scan(FScanContext& ctx) {
    auto foreach_file = [this, &ctx](const FFilename& fname) {
        AddStaticDep(ctx.GetOrCreateFileNode(fname));
    };

    if (_pattern.empty()) {
        VFS_EnumerateFiles(_path, _recursive, foreach_file);
    }
    else {
        FWRegexp re;
        re.Compile(_pattern, FileSystem::CaseSensitive);
        VFS_MatchFiles(_path, re, _recursive, foreach_file);
    }

    return EBuildResult::Built;
}
//----------------------------------------------------------------------------
EBuildResult FDirectoryListNode::Import(FBuildContext&) {
    return EBuildResult::UpToDate; // nothing to do
}
//----------------------------------------------------------------------------
EBuildResult FDirectoryListNode::Process(FBuildContext&) {
    return EBuildResult::UpToDate; // nothing to do
}
//----------------------------------------------------------------------------
EBuildResult FDirectoryListNode::Clean(FCleanContext&) {
    return EBuildResult::UpToDate; // nothing to do
}
//----------------------------------------------------------------------------
void FDirectoryListNode::SetPath(const FDirpath& path) NOEXCEPT {
    Assert(not path.empty());

    _path = path;
}
//----------------------------------------------------------------------------
void FDirectoryListNode::SetPattern(FWString&& pattern) NOEXCEPT {
    _pattern = std::move(pattern);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
